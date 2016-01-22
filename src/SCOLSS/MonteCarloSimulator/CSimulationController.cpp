//
// Created by mpolovyi on 24/08/15.
//
#include <iostream>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <cmath>
#include <chrono>

#include "CSimulationController.h"
#include <SCOLSS/EPSPlot/EPSPlot.h>
#include <SCOLSS/ParticlePhysics/CParticle.h>
#include "CSimulationParameters.h"


CSimulationController::CSimulationController(CSimulationParameters simulationParameters) {
    SimulationParameters = simulationParameters;
    InitRandom();

    particles.resize(0);
    cyclesTaken = 0;
    epsLine = 0;

    initialize_time = std::chrono::system_clock::now();
    for (int i = 0; i < SimulationParameters.PtCount; ++i) {
        CParticle pt(1000, 10, SimulationParameters.SystemSize);

        pt.Coordinates = i > 0
                         ? i * (SimulationParameters.ParticleDiameter/SimulationParameters.Density) + initialDisplacementDistribution(rnd_gen)
                         : 0;

        pt.Rotation = GetRandomUnitQuaternion();

        particles.push_back(pt);
    }

    if (SimulationParameters.PtCount == 2){
        particles[0].Coordinates = 0;
        particles[1].Coordinates = 1;
        particles[0].Rotation = CQuaternion(0, CVector(1, 1, 1));
        particles[1].Rotation = CQuaternion(0, CVector(1, 1, 1));
    }

    if (SimulationParameters.savedStateToLoad.size() == SimulationParameters.PtCount) {
        particles = SimulationParameters.savedStateToLoad;
    }
}

void CSimulationController::InitRandom() {
    uniformDistributionZeroOne = std::uniform_real_distribution<double>(0, 1);
    uniformDistributionZeroTwoPi = std::uniform_real_distribution<double>(0, 2*M_PI);

    uniformDistributionAcceptance = std::uniform_real_distribution<double>(0, 1);

    uniformDistributionParticleSelect = std::uniform_int_distribution<int>(0, static_cast<int>(SimulationParameters.PtCount - 1));

    uniformDistributionMove = std::uniform_real_distribution<double>(-SimulationParameters.MaxTestDisplacement / 2,
                                                         SimulationParameters.MaxTestDisplacement / 2);

    auto tmp = (SimulationParameters.ParticleDiameter/SimulationParameters.Density)/4;
    initialDisplacementDistribution = std::uniform_real_distribution<double>(-tmp, tmp);

    std::chrono::high_resolution_clock high_resolution_clock;
    auto tm = std::chrono::time_point_cast<std::chrono::nanoseconds>(high_resolution_clock.now());

    rnd_gen();
    rnd_gen = std::mt19937_64((unsigned long) tm.time_since_epoch().count());
}

void CSimulationController::DoCycle() {
    cyclesTaken++;

    if (SimulationParameters.PtCount == 2) {
       int n = 1;
    }
    else{
        for (size_t pt = 0; pt < SimulationParameters.PtCount; ++pt) {
            DoTestMove(pt);
            DoTestRotation(pt);
        }
    }
}

void CSimulationController::DoTestMove(size_t ptIndex) {
    auto beforeEnergy = GetPotentialEnergy(ptIndex);
    oldParticleCoordinates = particles[ptIndex].Coordinates;

    double move = uniformDistributionMove(rnd_gen);
    if (SimulationParameters.PtCount == 2){
        particles[ptIndex].Coordinates += move;
    }
    else{
        particles[ptIndex].Coordinates += move;
        AccountForBorderAfterMove(particles[ptIndex]);
    }

    auto afterEnergy = GetPotentialEnergy(ptIndex);
    if (AcceptMove(afterEnergy, beforeEnergy)){
        return;
    }
    else{
        particles[ptIndex].Coordinates = oldParticleCoordinates;
    }
}

void CSimulationController::DoTestRotation(size_t ptIndex) {
    auto beforeEnergy = GetPotentialEnergy(ptIndex);
    oldParticleRotation = particles[ptIndex].Rotation;

    particles[ptIndex].Rotation = GetRandomUnitQuaternion();

    auto afterEnergy = GetPotentialEnergy(ptIndex);
    if (AcceptMove(afterEnergy, beforeEnergy)){
        return;
    }
    else{
        particles[ptIndex].Rotation = oldParticleRotation;
    }
}

bool CSimulationController::AcceptMove(double energy, double oldEnergy) {
    bool ret = energy <= oldEnergy;

    if(!ret) {
        auto ex = exp(-(energy - oldEnergy)/SimulationParameters.KbT);
        ret = uniformDistributionAcceptance(rnd_gen) < ex;
    }

    return ret;
}

double CSimulationController::GetPotentialEnergy(size_t ptIndex)const {
//    if(SimulationParameters.PtCount == 2) {
//        return particles[0].GetPotentialEnergy(particles[1], particles[1].GetDistanceLeft(particles[0]));
//    }
//    else {
//
//
    return particles[ptIndex].GetPotentialEnergy(particles[GetNextParticle(ptIndex)],
                                                 particles[GetNextParticle(ptIndex)].GetDistanceLeft(particles[ptIndex], SimulationParameters.SystemSize))
               + particles[ptIndex].GetPotentialEnergy(particles[GetPreviousParticle(ptIndex)],
                                                       particles[GetPreviousParticle(ptIndex)].GetDistanceRight(particles[ptIndex], SimulationParameters.SystemSize));
//    }
}

void CSimulationController::RunSimulation(cereal::JSONOutputArchive &outputArchive,
                                           EPSPlot& pictureSaver) {
    outputArchive.makeArray();

    outputArchive(*this);
    SaveIntoEps(pictureSaver);
    std::chrono::time_point<std::chrono::system_clock> start_time, step_time;
    start_time = std::chrono::system_clock::now();
    uint64_t prev_measure = 0;

    uint64_t totalCycles = SimulationParameters.EquilibriumCycle * SimulationParameters.NumberOfSavePoints;

    int k = 1;

    for (uint64_t cycle = 1; cycle <= totalCycles; ++cycle) {
        DoCycle();

        if (0 == cycle % (SimulationParameters.EquilibriumCycle)) {
            outputArchive(*this);
        }

        if (0 == cycle % (totalCycles/SimulationParameters.NumberOfImageLines)) {
            SaveIntoEps(pictureSaver);
        }

        auto doFinish = PrintTimeExtrapolation(start_time, prev_measure, totalCycles, cycle);
        if(doFinish){
            outputArchive(*this);
            SaveIntoEps(pictureSaver);
            break;
        }
    }
}

bool CSimulationController::PrintTimeExtrapolation(std::chrono::time_point<std::chrono::system_clock> &start_time,
                                                   uint64_t &prev_measure, uint64_t totalCycles, uint64_t cycle) const {
    auto step_time = std::chrono::system_clock::now();

    std::chrono::duration<double> elapsed_secs = step_time - start_time;

    if ((elapsed_secs.count() > 15 && prev_measure == 0) || elapsed_secs.count() > 300) {
        auto sps = elapsed_secs / (cycle - prev_measure);

        auto req_s = sps * (totalCycles - cycle);

        auto req_h = (int) std::chrono::duration_cast<std::chrono::hours>(req_s).count();
        auto req_min = (int) std::chrono::duration_cast<std::chrono::minutes>(req_s).count() - req_h * 60;
        auto req_sec = (int) std::chrono::duration_cast<std::chrono::seconds>(req_s).count() - req_h * 3600 - req_min * 60;

        std::cout << " will run for "
        << req_h << " hours "
        << req_min << " minutes "
        << req_sec << " seconds" << std::endl;
        start_time = std::chrono::system_clock::now();
        prev_measure = cycle;
    }
       
    elapsed_secs = step_time - initialize_time;
    auto hours_since_init = std::chrono::duration_cast<std::chrono::hours>(elapsed_secs).count();

    return hours_since_init == 48;
}

std::vector<double> CSimulationController::GetParticlesCoordinates() const {
    std::vector<double> ret;

    for(auto& pt : particles){
        ret.push_back(pt.Coordinates);
    }

    return ret;
}

std::vector<double> CSimulationController::GetParticlesOrientationsZ() const {
    std::vector<double> ret;

    for(auto& pt : particles){
        ret.push_back(pt.GetOrientation().Z);
    }

    return ret;
}

CVector CSimulationController::GetUniformRandomVector() {
    return CVector(uniformDistributionZeroOne(rnd_gen), uniformDistributionZeroOne(rnd_gen), uniformDistributionZeroOne(rnd_gen));
}

double CSimulationController::GetAveragePotentialEnergy() const {
    if (SimulationParameters.PtCount == 2) {
        return particles[0].GetPotentialEnergy(particles[1], particles[1].GetDistanceLeft(particles[0], SimulationParameters.SystemSize));
    }
    else {
        double ret = 0;

        for (size_t i = 0; i < SimulationParameters.PtCount; i++) {
            ret += GetPotentialEnergy(i);
        }
        return ret / 2 / SimulationParameters.PtCount;
    }
}

uint64_t CSimulationController::GetSimulationCycles() const {
    return cyclesTaken;
}

double CSimulationController::GetOrderParameter() const {
    auto ptOrientations = GetParticlesOrientationsZ();

    double op = 0;
    for (auto r : ptOrientations) {
        op += r * r;
    }
    op /= SimulationParameters.PtCount;

    op = (3 * op - 1) / 2;

    return op;
}

size_t CSimulationController::GetNextParticle(size_t ptIndex) const {
    size_t ret = ptIndex + 1;

    if (ret == SimulationParameters.PtCount) {
        return 0;
    }

    return ret;
}

size_t CSimulationController::GetPreviousParticle(size_t ptIndex) const {
    size_t ret = ptIndex - 1;

    if (ret == -1) {
        return SimulationParameters.PtCount - 1;
    }

    return ret;
}

void CSimulationController::AccountForBorderAfterMove(CParticle &pt_new) {
    if (SimulationParameters.PtCount == 2){
        return;
    }
    else {
        if (pt_new.Coordinates >= SimulationParameters.SystemSize) {
            pt_new.Coordinates -= SimulationParameters.SystemSize;
        } else if (pt_new.Coordinates <= 0) {
            pt_new.Coordinates += SimulationParameters.SystemSize;
        }
    }
}

void CSimulationController::SaveIntoEps(EPSPlot &outFile) {
    epsLine++;
    for (size_t i = 0; i < SimulationParameters.PtCount; ++i) {
        auto& pt = particles[i];
        auto orient = pt.GetOrientation();

        float colR;
        float colG;
        float colB;
        if(orient.Z > 0){
            colB = 1 - (float) orient.Z;
            colG = colB;
            colR = 1;
        }
        else{
            colR = 1 - (float) std::abs(orient.Z);
            colG = colR;
            colB = 1;
        }

        outFile.drawPoint((float) pt.Coordinates,
                          (float) (epsLine * SimulationParameters.ParticleDiameter),
                          (float) SimulationParameters.ParticleDiameter,
                          colR, colG, colB);
    }
}
