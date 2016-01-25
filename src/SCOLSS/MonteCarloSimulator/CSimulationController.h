//
// Created by mpolovyi on 24/08/15.
//
#include <list>
#include <vector>
#include <random>
#include <memory>
#include <chrono>
#include <cereal/archives/json.hpp>
#include <SCOLSS/ParticlePhysics/CYukawaDipolePt.h>
#include <SCOLSS/EPSPlot/EPSPlot.h>

#include "CSimulationParameters.h"

#ifndef COLLOIDMC_CSIMULATIONCONTROLLER_H
#define COLLOIDMC_CSIMULATIONCONTROLLER_H

class CSimulationController {
public:
    std::chrono::time_point<std::chrono::system_clock> initialize_time;

    std::vector<CYukawaDipolePt> particles;

    CSimulationParameters SimulationParameters;

    CSimulationController(CSimulationParameters simulationParameters);

    void RunSimulation(cereal::JSONOutputArchive &outputArchive,
                        EPSPlot &pictureSaver);

    uint64_t cyclesTaken;

    double GetAveragePotentialEnergy() const;

    double GetOrderParameter() const;

    uint64_t GetSimulationCycles() const;

    std::vector<double> GetParticlesCoordinates() const;

    std::vector<double> GetParticlesOrientationsZ() const;

    template<class Archive>
    void save(Archive &archieve) const {
        archieve(cereal::make_nvp("SimulationParameters", SimulationParameters));

        archieve(cereal::make_nvp("SimulationCycles", GetSimulationCycles()));
        archieve(cereal::make_nvp("OrderParameter", GetOrderParameter()));
        archieve(cereal::make_nvp("Energy", GetAveragePotentialEnergy()));

        {
            const CYukawaDipolePt *tmp = &particles[0];
            archieve.saveBinaryValue(tmp, sizeof(CYukawaDipolePt) * SimulationParameters.PtCount, "Particles");
        }

//        {
//            auto tmp_vec = GetParticlesOrientationsZ();
//            const auto *tmp = &tmp_vec[0];
//            archieve.saveBinaryValue(tmp, sizeof(&tmp) * tmp_vec.size(), "ParticlesOrientationsZ");
//        }
//
//        {
//            auto tmp_vec = GetParticlesCoordinates();
//            const auto *tmp = &tmp_vec[0];
//            archieve.saveBinaryValue(tmp, sizeof(&tmp) * tmp_vec.size(), "ParticlesCoordinates");
//        }
    }

    int epsLine;

    void SaveIntoEps(EPSPlot &outFile);

    CVector GetUniformRandomVector();

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
    }
protected:
    std::mt19937_64 rnd_gen;
    std::uniform_real_distribution<double> initialDisplacementDistribution;
    std::uniform_int_distribution<int> uniformDistributionParticleSelect;
    std::uniform_real_distribution<double> uniformDistributionAcceptance;
    std::uniform_real_distribution<double> uniformDistributionMove;

    std::uniform_real_distribution<double> uniformDistributionZeroTwoPi;
    std::uniform_real_distribution<double> uniformDistributionZeroOne;
    double oldParticleCoordinates;

    CQuaternion oldParticleRotation;

    bool AcceptMove(double energy, double oldEnergy);

    double GetPotentialEnergy(size_t ptIndex) const;

    void DoTestMove(size_t index);

    void DoTestRotation(size_t index);

    void DoCycle();

    bool PrintTimeExtrapolation(std::chrono::time_point<std::chrono::system_clock> &start_time, uint64_t &prev_measure,
                                uint64_t totalCycles,
                                uint64_t cycle) const;

    void InitRandom();

    size_t GetNextParticle(size_t ptIndex) const;

    size_t GetPreviousParticle(size_t ptIndex) const;

    void AccountForBorderAfterMove(CYukawaDipolePt &pt_new);
};

#endif //COLLOIDMC_CSIMULATIONCONTROLLER_H
