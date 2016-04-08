//
// Created by mpolovyi on 25/01/16.
//

#ifndef PROJECT_CLANGEVINSIMCTRL_H
#define PROJECT_CLANGEVINSIMCTRL_H

#include <vector>
#include <random>
#include <cereal/cereal.hpp>
#include <cereal/types/polymorphic.hpp>
#include <cereal/archives/json.hpp>
#include <cstdlib>

#include <SCOLSS/ParticlePhysics/CParticleBase.h>
#include <SCOLSS/ParticlePhysics/CYukawaDipolePt.h>

#include <SCOLSS/SimulationController/BaseSimCtrl/CBaseSimCtrl.h>
#include <SCOLSS/SimulationController/LangevinSimCtrl/CLangevinSimParams.h>

class CLangevinSimCtrl : public CBaseSimCtrl {
public:
    CLangevinSimParams SimulationParameters;

    std::normal_distribution<double> translationNormalDistribution;
    std::normal_distribution<double> rotationNormalDistribution;

    template<class Archive>
    void save(Archive& archive) const {
        archive(cereal::base_class<CBaseSimCtrl>(this));
        archive(cereal::make_nvp("SimulationParameters", SimulationParameters));
        archive(cereal::make_nvp("SimulationTime", GetSimulationTime()));
        archive(cereal::make_nvp("KineticEnergy", GetAverageKineticEnergy()));
    }

    CLangevinSimCtrl(CLangevinSimParams d) : CBaseSimCtrl(d), SimulationParameters(d) {
        int currentId = MPI::COMM_WORLD.Get_rank();
        {
            printf("entr %s in proc %i\n", __PRETTY_FUNCTION__, currentId);
        }
        InitRandomGenerator();
    }

    void DoCycle() {
        int currentId = MPI::COMM_WORLD.Get_rank();
        Cycles += 1;

//        int currentId = MPI::COMM_WORLD.Get_rank();
//        std::cout << " " << &ProcessMapFull[currentId] << " " << &ProcessMap_old[currentId] << " " << &ProcessMap_new[currentId]
//        << " " << &particles_old()[currentId * PerProcCount] << " " << &particles_new()[currentId * PerProcCount] << "\n";

        int id = MPI::COMM_WORLD.Get_rank();
        for (size_t i = ProcessMapFull[id].beginIndex(); i < ProcessMapFull[id].endIndex(); i++) {
            MoveParticleVerlet(i);
            RotateParticleVerlet(i);
        }

        SyncInCycle();

        for (size_t i = ProcessMapFull[id].beginIndex(); i < ProcessMapFull[id].endIndex(); i++) {
            AccelerateMoveParticleVerlet(i);
            AccelerateRotateParticleVerlet(i);
        }

        SyncInCycle();
        std::swap(particles_old, particles_new);
        std::swap(ProcessMap_old, ProcessMap_new);

//        std::cout << "  " << &ProcessMapFull[currentId] << " " << &ProcessMap_old[currentId] << " " << &ProcessMap_new[currentId]
//        << " " << &particles_old()[currentId * PerProcCount] << " " << &particles_new()[currentId * PerProcCount] << "\n";
    };

    void MoveParticleVerlet(size_t ptIndex);

    void AccelerateMoveParticleVerlet(size_t ptIndex);

    void RotateParticleVerlet(size_t ptIndex);

    void AccelerateRotateParticleVerlet(size_t ptIndex);

    double GetForceOld(size_t ptIndex);

    double GetForceNew(size_t ptIndex);

    CVector GetTorqueOld(size_t ptIndex);

    CVector GetTorqueNew(size_t ptIndex);

    double GetAverageKineticEnergy() const;

    double GetRotationalEnergy() const;

    double GetTranslationalEnergy() const;

    double GetSimulationTime() const;

protected:
    double GetAverageDispl() const;

    double GetAverAngularDispl() const;

    double GetAverAngularDisplX() const;

    double GetAverAngularDisplY() const;

    double GetAverAngularDisplZ() const;

    CVector GetNormalRandomVector(std::normal_distribution<double> &normalDistribution);

    virtual void InitRandomGenerator() {
        int currentId = MPI::COMM_WORLD.Get_rank();
        CBaseSimCtrl::InitRandomGenerator();

        translationNormalDistribution = std::normal_distribution<double>(0, SimulationParameters.DistributionDeviationTranslation);
        rotationNormalDistribution = std::normal_distribution<double>(0, SimulationParameters.DistributionDeviationRotation);
    };
};

#endif //PROJECT_CLANGEVINSIMCTRL_H
