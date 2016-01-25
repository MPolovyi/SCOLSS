//
// Created by mpolovyi on 25/01/16.
//

#ifndef PROJECT_CBASESIMCTRL_H
#define PROJECT_CBASESIMCTRL_H

#define __cplusplus 201103L

#include <vector>
#include <list>
#include <random>
#include <cereal/cereal.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/archives/json.hpp>
#include <fstream>
#include <chrono>

#include <SCOLSS/ParticlePhysics/CParticleBase.h>
#include <SCOLSS/ParticlePhysics/CYukawaDipolePt.h>
#include <SCOLSS/EPSPlot/EPSPlot.h>

#include "CBaseSimParams.h"

class CBaseSimCtrl {
public:
    unsigned long Cycles;

    std::chrono::time_point<std::chrono::system_clock> initialize_time;

    unsigned long RandomSeed;

    std::mt19937_64 rnd_gen;

    std::uniform_real_distribution<double> uniformDistributionZeroTwoPi;
    std::uniform_real_distribution<double> uniformDistributionZeroOne;
//
    std::uniform_real_distribution<double> initialDisplacementDistribution;

    const CBaseSimParams SimulationParameters;

    template<class Archive>
    void serialize(Archive &archieve) {
        DoSerialize(archieve);
    }

    template<class Archive>
    virtual void DoSerialize(Archive &archieve) {
        archieve(cereal::make_nvp("SimulationParameters", SimulationParameters));
        archieve(cereal::make_nvp("OrderParameter", GetOrderParameter()));

        archieve(cereal::make_nvp("PotentialEnergy", GetAveragePotentialEnergy()));

        {
            std::vector<CParticleBase> pts_save;
            for (int i = 0; i < SimulationParameters.PtCount; ++i) {
                pts_save.push_back(particles_old[i]);
            }

            const CParticleBase *tmp = &pts_save[0];
            archieve.saveBinaryValue(tmp, sizeof(CParticleBase) * particles_old.size(), "Particles");
        }
    }

    CBaseSimCtrl(CBaseSimParams d) : CBaseSimCtrl(d) {
            Cycles = 0;
            epsLine = 1;

            InitRandomGenerator();
            for (int i = 0; i < SimulationParameters.PtCount; i++) {
                CYukawaDipolePt pt(SimulationParameters.YukawaA, SimulationParameters.YukawaK, SimulationParameters.SystemSize);


                switch (SimulationParameters.InitialConfiguration) {
                    case EInitialConfiguration::Random: {
                        pt.Rotation = GetRandomUnitQuaternion();
                        pt.Coordinates = i > 0
                                         ? i * (SimulationParameters.ParticleDiameter / SimulationParameters.Density) +
                                           initialDisplacementDistribution(rnd_gen)
                                         : 0;
                        break;
                    }

                    case EInitialConfiguration::RandomUnmoving: {
                        pt.Rotation = GetRandomUnitQuaternion();
                        pt.Coordinates = i * (SimulationParameters.ParticleDiameter / SimulationParameters.Density);                                         : 0;
                        break;
                    }

                    case EInitialConfiguration::Aligned: {
                        pt.Rotation = CQuaternion(0, CVector::AxisZ);
                        pt.Coordinates = i > 0
                                         ? i * (SimulationParameters.ParticleDiameter / SimulationParameters.Density) +
                                           initialDisplacementDistribution(rnd_gen)
                                         : 0;
                        break;
                    }

                    case EInitialConfiguration::AlignedTwoSides: {
                        pt.Rotation = CQuaternion(M_PI * (i % 2), CVector::AxisY);
                        pt.Coordinates = i > 0
                                         ? i * (SimulationParameters.ParticleDiameter / SimulationParameters.Density) +
                                           initialDisplacementDistribution(rnd_gen)
                                         : 0;
                        break;
                    }

                    case EInitialConfiguration::AlingnedUnmoving: {
                        pt.Rotation = CQuaternion(M_PI * (i % 2), CVector::AxisY);
                        pt.Coordinates = i * (SimulationParameters.ParticleDiameter / SimulationParameters.Density);                                         : 0;
                        break;
                    }
                }
                particles_new.push_back(pt);
                particles_old.push_back(pt);
            }

            initialize_time = std::chrono::system_clock::now();
    }

    void InitRandomGenerator() {
        uniformDistributionZeroOne = std::uniform_real_distribution<double>(0, 1);
        uniformDistributionZeroTwoPi = std::uniform_real_distribution<double>(0, 2*M_PI);

        initialDisplacementDistribution = std::uniform_real_distribution<double>(
                -(SimulationParameters.ParticleDiameter/SimulationParameters.Density)/4,
                (SimulationParameters.ParticleDiameter/SimulationParameters.Density)/4);

        time_t rawtime;
        time(&rawtime);

        RandomSeed = rand() + rand() + rand();
        RandomSeed += rawtime;

        rnd_gen = std::mt19937_64(RandomSeed);
    };

    void RunSimulation(cereal::JSONOutputArchive &outputArchive, EPSPlot& pictureSaver) {
        outputArchive.makeArray();

        outputArchive(*this);
        SaveIntoEps(pictureSaver);
        std::chrono::time_point<std::chrono::system_clock> start_time, step_time;
        start_time = std::chrono::system_clock::now();
        uint64_t prev_measure = 0;

        uint64_t totalCycles = SimulationParameters.CyclesBetweenSaves * SimulationParameters.NumberOfSavePoints;

        int k = 1;
        for (uint64_t cycle = 1; cycle <= totalCycles; ++cycle) {
            DoCycle();

            if (0 == cycle % (SimulationParameters.CyclesBetweenSaves)) {
                outputArchive(*this);
            }

            if (0 == cycle % (totalCycles / SimulationParameters.NumberOfImageLines)) {
                SaveIntoEps(pictureSaver);
            }

            auto doFinish = PrintTimeExtrapolation(start_time, prev_measure, totalCycles, cycle);
            if (doFinish) {
                outputArchive(*this);
                SaveIntoEps(pictureSaver);
                break;
            }
        }
    }

    bool PrintTimeExtrapolation(std::chrono::time_point<std::chrono::system_clock> &start_time,
                                uint64_t &prev_measure, uint64_t totalCycles, uint64_t cycle) const {
        auto step_time = std::chrono::system_clock::now();

        std::chrono::duration<double> elapsed_secs = step_time - start_time;

        if ((elapsed_secs.count() > 15 && prev_measure == 0) || elapsed_secs.count() > 300) {
            auto sps = elapsed_secs / (cycle - prev_measure);

            auto req_s = sps * (totalCycles - cycle);

            auto req_h = std::chrono::duration_cast<std::chrono::hours>(req_s).count();
            auto req_min = std::chrono::duration_cast<std::chrono::minutes>(req_s).count() - req_h * 60;
            auto req_sec = std::chrono::duration_cast<std::chrono::seconds>(req_s).count() - req_h * 3600 - req_min * 60;

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

    virtual void DoCycle() { }

    double GetAveragePotentialEnergy() {
        if (SimulationParameters.PtCount == 2) {
            return particles_old[0].GetPotentialEnergy(particles_old[1], particles_old[1].GetDistanceLeft(particles_old[0], SimulationParameters.SystemSize));
        }

        double ret = 0;

        for (int i = 0; i < SimulationParameters.PtCount; i++) {
            auto &pt = particles_old[i];

            ret += GetParticlePotentialEnergyOld(i);
        }

        return ret / 2.0 / SimulationParameters.PtCount;
    }

    void SaveForPovray(std::fstream& ofstr) {
        for (int i = 0; i < SimulationParameters.PtCount; ++i) {
            auto rot = particles_old[i].GetOrientation();
            ofstr
            << "Colloid(<0, 0, "
            << particles_old[i].Coordinates/SimulationParameters.ParticleDiameter
            << ">, <" << rot.X << ", " << rot.Y << ", " << rot.Z << ">)" <<
            std::endl;
        }
    }

    unsigned int epsLine;

    void SaveIntoEps(EPSPlot &outFile) {
        epsLine++;
        for (int i = 0; i < SimulationParameters.PtCount; ++i) {
            auto& pt = particles_old[i];
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

    CQuaternion GetRandomUnitQuaternion() {
        auto u1 = uniformDistributionZeroOne(rnd_gen);
        auto u2 = uniformDistributionZeroTwoPi(rnd_gen);
        auto u3 = uniformDistributionZeroTwoPi(rnd_gen);

        auto ret = CQuaternion(
                sqrt(1 - u1) * sin(u2),
                sqrt(1 - u1) * cos(u2),
                sqrt(u1) * sin(u3),
                sqrt(u1) * cos(u3)
        );

        return ret;
    };

    std::vector<CYukawaDipolePt> particles_old;
    std::vector<CYukawaDipolePt> particles_new;
protected:

    double GetParticlePotentialEnergyOld(int ptIndex) {
        return particles_old[ptIndex].GetPotentialEnergy(particles_old[GetNext(ptIndex)],
                                                         particles_old[GetNext(ptIndex)].GetDistanceLeft(particles_old[ptIndex], SimulationParameters.SystemSize))
               + particles_old[ptIndex].GetPotentialEnergy(particles_old[GetPrevious(ptIndex)],
                                                           particles_old[GetPrevious(ptIndex)].GetDistanceRight(particles_old[ptIndex], SimulationParameters.SystemSize));

    }

    double GetOrderParameter() {
        auto ptOrientations = GetParticlesOrientationZ();

        double op = 0;
        for (auto r : ptOrientations) {
            op += r * r;
        }
        op /= SimulationParameters.PtCount;

        op = (3 * op - 1) / 2;

        return op;
    }

    std::vector<double> GetParticlesOrientationZ() {
        std::vector<double> ret;

        for (auto &pt : particles_old) {
            ret.push_back(pt.GetOrientation().Z);
        }

        return ret;
    }

    std::vector<double> GetParticleCoordinatesZ() {
        std::vector<double> ret;

        for (auto &pt : particles_old) {
            ret.push_back(pt.Coordinates);
        }

        return ret;
    }

    inline int GetNext(int ptIndex) {
        int ret = ptIndex + 1;

        if (ret == SimulationParameters.PtCount) {
            return 0;
        }

        return ret;
    }

    inline int GetPrevious(int ptIndex) {
        int ret = ptIndex - 1;

        if (ret == -1) {
            return SimulationParameters.PtCount - 1;
        }

        return ret;
    }

    inline void AccountForBorderAfterMove(CYukawaDipolePt &pt_new) {
        if (SimulationParameters.PtCount == 2) {
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

    template<typename T>
    int GetNearestIndex(T arr, double val, int size) {
        int ret = 0;
        double dst_old = 100000;
        for (int i = 0; i < size; i++) {
            double dst = std::abs(val - arr[i]);
            if (dst < dst_old) {
                dst_old = dst;
                ret = i;
            }
        }
        return ret;
    }
};


#endif //PROJECT_CBASESIMCTRL_H
