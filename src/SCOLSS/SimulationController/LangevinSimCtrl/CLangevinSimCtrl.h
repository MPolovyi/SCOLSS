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

    CLangevinSimCtrl(CLangevinSimParams d, int procCount) : CBaseSimCtrl(d, procCount), SimulationParameters(d) {
        InitRandomGenerator();
    }

    virtual void InitRandomGenerator() {
        CBaseSimCtrl::InitRandomGenerator();

        translationNormalDistribution = std::normal_distribution<double>(0, SimulationParameters.DistributionDeviationTranslation);
        rotationNormalDistribution = std::normal_distribution<double>(0, SimulationParameters.DistributionDeviationRotation);
    };

    void DoCycle() {
        Cycles += 1;

        int id = MPI::COMM_WORLD.Get_rank();

        for (size_t i = ProcessMapFull[id].firstIndex(); i < ProcessMapFull[id].endIndex(); i++) {
            MoveParticleVerlet(i);
            RotateParticleVerlet(i);
        }

        SyncToMain(ProcessMap_new);
        SyncFromMain(ProcessMap_new);

        for (size_t i = ProcessMapFull[id].firstIndex(); i < ProcessMapFull[id].endIndex(); i++) {
            AccelerateMoveParticleVerlet(i);
            AccelerateRotateParticleVerlet(i);
        }

        SyncToMain(ProcessMap_new);
        SyncFromMain(ProcessMap_new);

        std::swap(particles_old, particles_new);
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
};

#endif //PROJECT_CLANGEVINSIMCTRL_H
