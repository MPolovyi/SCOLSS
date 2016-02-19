//
// Created by mpolovyi on 22/01/16.
//

#include <iostream>
#include <cereal/archives/json.hpp>
#include <cereal/types/polymorphic.hpp>

#include <SCOLSS/SimulationController/MonteCarloSimCtrl/CMonteCarloSimParams.h>
#include <SCOLSS/SimulationController/MonteCarloSimCtrl/CMonteCarloSimCtrl.h>
#include <SCOLSS/SimulationController/LangevinSimCtrl/CLangevinSimParams.h>
#include <SCOLSS/SimulationController/LangevinSimCtrl/CLangevinSimCtrl.h>

#include <SCOLSS/ParticlePhysics/CYukawaDipolePt.h>

void InitializeSimulations(int argc, char **argv);
void RunSimulations(std::shared_ptr<CBaseSimCtrl> sim, std::string &mainSaveFileName, EPSPlot &pictureSaver);

void SaveToFile(const std::shared_ptr<CBaseSimCtrl> &contr, const std::string &mainSaveFileName, uint64_t cycle);

int main(int argc, char **argv) {
//    CYukawaDipolePt pt1(1000, 10, 1000);
//    CYukawaDipolePt pt2(1000, 10, 1000);
//
//    pt1.SetRotation(CQuaternion(0, CVector::AxisY));
//    pt2.SetRotation(CQuaternion(-0, CVector::AxisY));
//
//    for(int i = 1; i < 10; i++)
//        std::cout << pt1.GetPotentialEnergy(pt2, CVector(0, 0, i/10.0)) << std::endl;
//
//    std::cout << std::endl;
//    pt1.SetRotation(CQuaternion(M_PI_2, CVector::AxisX));
//    pt2.SetRotation(CQuaternion(-M_PI_2*1.1, CVector::AxisX));
//
//    for(int i = 1; i < 10; i++)
//        std::cout << pt1.GetPotentialEnergy(pt2, CVector(0, 0, i/10.0)) << std::endl;

    CVector a_i(0, 0, 1);
    CVector b_i(0, 0, 1);
    CVector c_i(0, 0, 1);

    CVector a_j(0, 0, 1);
    CVector b_j(0, 0, 1);
    CVector c_j(0, 0, 1);

    std::vector<CVector> test_i;
    std::vector<CVector> test_j;

    std::mt19937_64 rnd_gen;
    rnd_gen.seed(1253134646);
    std::uniform_real_distribution<double> uniformDistributionZeroTwoPi = std::uniform_real_distribution<double>(0, 2*M_PI);

    std::uniform_real_distribution<double> uniformDistributionZeroOne = std::uniform_real_distribution<double>(0, 1);

    for (int i = 0; i < 100; ++i) {

        auto u1 = uniformDistributionZeroOne(rnd_gen);
        auto u2 = uniformDistributionZeroTwoPi(rnd_gen);
        auto u3 = uniformDistributionZeroTwoPi(rnd_gen);

        auto ret = CQuaternion(
                sqrt(1 - u1) * sin(u2),
                sqrt(1 - u1) * cos(u2),
                sqrt(u1) * sin(u3),
                sqrt(u1) * cos(u3)
        );

        ret = CQuaternion(M_PI*u1, CVector::AxisY);

        test_i.push_back(ret*CVector::AxisZ*ret.GetInverse());

//        u1 = uniformDistributionZeroOne(rnd_gen);
//        u2 = uniformDistributionZeroTwoPi(rnd_gen);
//        u3 = uniformDistributionZeroTwoPi(rnd_gen);
//
//        ret = CQuaternion(
//                sqrt(1 - u1) * sin(u2),
//                sqrt(1 - u1) * cos(u2),
//                sqrt(u1) * sin(u3),
//                sqrt(u1) * cos(u3)
//        );

        test_j.push_back(ret*CVector::AxisZ*ret.GetInverse());
    }

    CVector sum_i;
    CVector sum_j;
    double sum_dot = 0;

    for (int j = 0; j < 100; ++j) {
        sum_i += test_i[j];
        sum_j += test_j[j];

        sum_dot += test_i[j].DotProduct(test_j[j]);
    }

    auto res = sum_dot/100.0 - (sum_i/100.0).DotProduct(sum_j/100.0);

    std::cout << res << std::endl;

    InitializeSimulations(argc, argv);
}

void InitializeSimulations(int argc, char **argv) {
    std::string simType = argv[2];
    ESimulationType simT;
    if(simType == "MC"){
        simT = ESimulationType::MonteCarlo;
    } else if (simType == "LD"){
        simT = ESimulationType::LangevinDynamics;
    } else {
        std::cout << "Unknown simulation type. Exiting." << std::endl;
        return;
    }

    std::string samples = argv[3];
    for(int i = 1; i <= std::stoi(samples); i++) {
        std::string fname = argv[1];
        std::ifstream simDataFileStream(fname);

        cereal::JSONInputArchive simDataArchieve(simDataFileStream);

        std::shared_ptr<CBaseSimCtrl> sim;

        switch (simT){
            case MonteCarlo: {
                CMonteCarloSimParams data;
                simDataArchieve(data);
                sim = std::make_shared<CMonteCarloSimCtrl>(CMonteCarloSimCtrl(data));
                break;
            };
            case LangevinDynamics: {
                CLangevinSimParams data;
                simDataArchieve(data);
                sim = std::make_shared<CLangevinSimCtrl>(CLangevinSimCtrl(data));
                break;
            };
        }

        std::stringstream ssMainSaveFileName;

        ssMainSaveFileName << "Results_" << i << fname;
        std::string mainSaveFileName = ssMainSaveFileName.str();

        std::string pictureSaveFileName = "Picture_" + fname + ".eps";

        EPSPlot savePictureFile(pictureSaveFileName.c_str(),
                                0,
                                0,
                                sim->SimulationParameters.GetEpsDimensionX(),
                                sim->SimulationParameters.GetEpsDimensionY());

        savePictureFile.initParticleSavings(sim->SimulationParameters.ParticleDiameter);

        RunSimulations(sim, mainSaveFileName, savePictureFile);
        std::cout << i << std::endl;
    }
}

void RunSimulations(std::shared_ptr<CBaseSimCtrl> sim, std::string &mainSaveFileName, EPSPlot &pictureSaver) {
    SaveToFile(sim, mainSaveFileName, 0);

    sim->SaveIntoEps(pictureSaver);
    std::chrono::time_point<std::chrono::system_clock> start_time, step_time;
    start_time = std::chrono::system_clock::now();
    uint64_t prev_measure = 0;

    uint64_t totalCycles = sim->SimulationParameters.CyclesBetweenSaves * sim->SimulationParameters.NumberOfSavePoints;
    sim->SimulationParameters.NumberOfImageLines = std::min(totalCycles, sim->SimulationParameters.NumberOfImageLines);
    for (uint64_t cycle = 1; cycle <= totalCycles; ++cycle) {
        sim->DoCycle();

        if (0 == cycle % (sim->SimulationParameters.CyclesBetweenSaves)) {
            SaveToFile(sim, mainSaveFileName, cycle);
        }

        if (0 == cycle % (totalCycles / sim->SimulationParameters.NumberOfImageLines)) {
            sim->SaveIntoEps(pictureSaver);
        }

        auto doFinish = sim->PrintTimeExtrapolation(start_time, prev_measure, totalCycles, cycle);
        if (doFinish) {
            SaveToFile(sim, mainSaveFileName, cycle);

            sim->SaveIntoEps(pictureSaver);
            break;
        }
    }
}

void SaveToFile(const std::shared_ptr<CBaseSimCtrl> &contr, const std::string &mainSaveFileName, uint64_t cycle) {
    std::fstream saveFileStream((mainSaveFileName + std::to_string(cycle)).c_str(), std::ios_base::out);
    cereal::JSONOutputArchive saveFileArchieve(saveFileStream);

    saveFileArchieve(contr);
}
