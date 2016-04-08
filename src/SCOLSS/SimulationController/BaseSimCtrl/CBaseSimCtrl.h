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

#include <mpi.h>

#include <cereal/cereal.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/polymorphic.hpp>
#include <cereal/archives/json.hpp>


#include <SCOLSS/EPSPlot/EPSPlot.h>

#include <SCOLSS/ParticlePhysics/CParticleBase.h>
#include <SCOLSS/ParticlePhysics/CYukawaDipolePt.h>
#include <SCOLSS/SimulationController/BaseSimCtrl/CBaseSimCtrl.h>
#include <SCOLSS/SimulationController/BaseSimCtrl/CBaseSimParams.h>

#include <SCOLSS/SimulationController/CDataChunk.h>

class CBaseSimCtrl {
public:
    unsigned long Cycles;

    CBaseSimParams SimulationParameters;

    std::chrono::time_point<std::chrono::system_clock> initialize_time;

    unsigned long RndSeed;
    std::mt19937_64 rnd_gen;
    std::uniform_real_distribution<double> uniformDistributionZeroTwoPi;

    std::uniform_real_distribution<double> uniformDistributionZeroOne;

    std::uniform_real_distribution<double> initialDisplacementDistribution;

    template<class Archive>
    void save(Archive &archive) const {
        archive(cereal::make_nvp("SimulationParameters", SimulationParameters));
        archive(cereal::make_nvp("OrderParameter", GetOrderParameter()));

        archive(cereal::make_nvp("PotentialEnergy", GetAveragePotentialEnergy()));

        if (SimulationParameters.SaveParticlesInfo) {
            std::vector<CParticleBase> pts_save;
            for (size_t i = 0; i < SimulationParameters.PtCount; ++i) {
                pts_save.push_back(particles_old_const(__PRETTY_FUNCTION__)[i]);
            }

            const CParticleBase *tmp = &pts_save[0];
            archive.saveBinaryValue(tmp, sizeof(CParticleBase) * particles_old_const(__PRETTY_FUNCTION__).size(), "Particles");
        }
    };

    CBaseSimCtrl(CBaseSimParams d);

    virtual void InitRandomGenerator() {
        int currentId = MPI::COMM_WORLD.Get_rank();
        {
            printf("entr %s in proc %i\n", __PRETTY_FUNCTION__, currentId);
        }

        if (currentId == ManagerProcId) {
            initialize_time = std::chrono::system_clock::now();

            std::chrono::high_resolution_clock high_resolution_clock;
            auto tm = std::chrono::time_point_cast<std::chrono::nanoseconds>(high_resolution_clock.now());
            RndSeed = (unsigned long) (tm.time_since_epoch().count());

            for (int destId = 0; destId < ChildProcCount; ++destId) {
                MPI::COMM_WORLD.Send(&initialize_time, sizeof(initialize_time), MPI::BYTE, destId, 0);
                MPI::COMM_WORLD.Send(&RndSeed, sizeof(RndSeed), MPI::BYTE, destId, 0);
            }
        } else {
            MPI::Status status;
            MPI::COMM_WORLD.Recv(&initialize_time, sizeof(initialize_time), MPI::BYTE, ManagerProcId, 0, status);
            MPI::COMM_WORLD.Recv(&RndSeed, sizeof(RndSeed), MPI::BYTE, ManagerProcId, 0, status);
        }

        uniformDistributionZeroOne = std::uniform_real_distribution<double>(0, 1);
        uniformDistributionZeroTwoPi = std::uniform_real_distribution<double>(0, 2 * M_PI);

        auto tmp = (SimulationParameters.ParticleDiameter / SimulationParameters.Density) / 4;
        initialDisplacementDistribution = std::uniform_real_distribution<double>(-tmp, tmp);

        rnd_gen();
        rnd_gen = std::mt19937_64(RndSeed);
    };

    bool PrintTimeExtrapolation(std::chrono::time_point<std::chrono::system_clock> &start_time,
                                uint64_t &prev_measure, uint64_t totalCycles, uint64_t cycle) const;

    virtual void DoCycle() { };

    double GetAveragePotentialEnergy() const;

    void SaveForPovray(std::fstream &ofstr);

    void SyncBeforeSave();

    size_t epsLine;

    void SaveIntoEps(EPSPlot &outFile);

    CQuaternion GetRandomUnitQuaternion();;
    int ManagerProcId;

protected:
    std::vector<CYukawaDipolePt> m_particles_old;

    std::vector<CYukawaDipolePt> &particles_old(const char *called_from) {
//        int currentId = MPI::COMM_WORLD.Get_rank();
//        {
//            printf("entr %s from %s in proc %i\n", __PRETTY_FUNCTION__, called_from, currentId);
//        }
        return m_particles_old;
    };

    const std::vector<CYukawaDipolePt> &particles_old_const(const char *called_from) const {
//        int currentId = MPI::COMM_WORLD.Get_rank();
//        {
//            printf("entr %s from %s in proc %i\n", __PRETTY_FUNCTION__, called_from, currentId);
//        }
        return m_particles_old;
    };

    std::vector<CYukawaDipolePt> m_particles_new;

    std::vector<CYukawaDipolePt> &particles_new(const char *called_from) {
//        int currentId = MPI::COMM_WORLD.Get_rank();
//        {
//            printf("entr %s in proc %i\n", __PRETTY_FUNCTION__, currentId);
//        }
        return m_particles_new;
    };

    const std::vector<CYukawaDipolePt> &particles_new_const() const {
//        int currentId = MPI::COMM_WORLD.Get_rank();
//        {
//            printf("entr %s in proc %i\n", __PRETTY_FUNCTION__, currentId);
//        }
        return m_particles_new;
    };

    std::vector<CDataChunk<CYukawaDipolePt> > ProcessMap_old;
    std::vector<CDataChunk<CYukawaDipolePt> > ProcessMap_new;

    std::vector<CDataChunk<CYukawaDipolePt> > ProcessMapFull;
    int ChildProcCount;

    size_t PerProcCount;

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

    CYukawaDipolePt getPt(size_t i);

    void CreateDataMapping(int procCount);

    void SyncToMain();

    void SyncInCycle();
};


#endif //PROJECT_CBASESIMCTRL_H