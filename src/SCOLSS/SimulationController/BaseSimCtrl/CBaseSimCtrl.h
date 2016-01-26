//
// Created by mpolovyi on 25/01/16.
//

#ifndef PROJECT_CBASESIMCTRL_H
#define PROJECT_CBASESIMCTRL_H

#include <vector>
#include <random>
#include <cstdlib>
#include <chrono>
#include <fstream>

#include <cereal/cereal.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/polymorphic.hpp>
#include <cereal/archives/json.hpp>


#include <SCOLSS/EPSPlot/EPSPlot.h>

#include <SCOLSS/ParticlePhysics/CParticleBase.h>
#include <SCOLSS/ParticlePhysics/CYukawaDipolePt.h>
#include <SCOLSS/SimulationController/BaseSimCtrl/CBaseSimCtrl.h>
#include <SCOLSS/SimulationController/BaseSimCtrl/CBaseSimParams.h>

class CBaseSimCtrl {
public:
    unsigned long Cycles;

    CBaseSimParams SimulationParameters;

    std::chrono::time_point<std::chrono::system_clock> initialize_time;

    unsigned long RandomSeed;

    std::mt19937_64 rnd_gen;
    std::uniform_real_distribution<double> uniformDistributionZeroTwoPi;

    std::uniform_real_distribution<double> uniformDistributionZeroOne;

    std::uniform_real_distribution<double> initialDisplacementDistribution;

    template<class Archive>
    void save(Archive &archieve) const {
        archieve(cereal::make_nvp("SimulationParameters", SimulationParameters));
        archieve(cereal::make_nvp("OrderParameter", GetOrderParameter()));

        archieve(cereal::make_nvp("PotentialEnergy", GetAveragePotentialEnergy()));

        {
//            std::vector<CParticleBase> pts_save;
//            for (size_t i = 0; i < SimulationParameters.PtCount; ++i) {
//                pts_save.push_back(particles_old[i]);
//            }
//
//            const CParticleBase *tmp = &pts_save[0];
//            archieve.saveBinaryValue(tmp, sizeof(CParticleBase) * particles_old.size(), "Particles");

            auto tmp = GetParticleCoordinatesZ();
            archieve(cereal::make_nvp("Particles", tmp));
        }
    };

    CBaseSimCtrl(CBaseSimParams d);

    virtual void InitRandomGenerator() {
        uniformDistributionZeroOne = std::uniform_real_distribution<double>(0, 1);
        uniformDistributionZeroTwoPi = std::uniform_real_distribution<double>(0, 2 * M_PI);

        auto tmp = (SimulationParameters.ParticleDiameter / SimulationParameters.Density) / 4;
        initialDisplacementDistribution = std::uniform_real_distribution<double>(-tmp, tmp);

        time_t rawtime;
        time(&rawtime);

        std::chrono::high_resolution_clock high_resolution_clock;
        auto tm = std::chrono::time_point_cast<std::chrono::nanoseconds>(high_resolution_clock.now());

        rnd_gen();
        rnd_gen = std::mt19937_64((unsigned long) tm.time_since_epoch().count());
    };

    bool PrintTimeExtrapolation(std::chrono::time_point<std::chrono::system_clock> &start_time,
                                uint64_t &prev_measure, uint64_t totalCycles, uint64_t cycle) const;

    virtual void DoCycle() {};

    double GetAveragePotentialEnergy() const;

    void SaveForPovray(std::fstream &ofstr);

    size_t epsLine;

    void SaveIntoEps(EPSPlot &outFile);

    CQuaternion GetRandomUnitQuaternion();;

    std::vector<CYukawaDipolePt> particles_old;
    std::vector<CYukawaDipolePt> particles_new;

protected:
    double GetParticlePotentialEnergy(size_t ptIndex) const;

    double GetOrderParameter() const;

    std::vector<double> GetParticlesOrientationZ() const;

    std::vector<double> GetParticleCoordinatesZ() const;

    size_t GetNext(size_t ptIndex) const;

    size_t GetPrevious(size_t ptIndex) const;

    void AccountForBorderAfterMove(CYukawaDipolePt &pt_new);

    template<typename T>
    size_t GetNearestIndex(T arr, double val, size_t size) {
        size_t ret = 0;
        double dst_old = 100000;
        for (size_t i = 0; i < size; i++) {
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
