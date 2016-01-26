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
    void save(Archive &archieve) const {
        archieve(cereal::base_class<CBaseSimCtrl>(this));
        archieve(cereal::make_nvp("SimulationParameters", SimulationParameters));
        archieve(cereal::make_nvp("SimulationTime", GetSimulationTime()));
        archieve(cereal::make_nvp("KineticEnergy", GetAverageKineticEnergy()));
    }

    CLangevinSimCtrl(CLangevinSimParams d) : CBaseSimCtrl(d), SimulationParameters(d) {
        InitRandomGenerator();
        int i = 1;
    }

    virtual void InitRandomGenerator() {
        CBaseSimCtrl::InitRandomGenerator();

        translationNormalDistribution = std::normal_distribution<double>(0, SimulationParameters.DistributionDeviationTranslation);
        rotationNormalDistribution = std::normal_distribution<double>(0, SimulationParameters.DistributionDeviationRotation);
    };

    void DoCycle() {
        Cycles += 1;
        for (size_t i = 0; i < SimulationParameters.PtCount; i++) {
            MoveParticleVerlet(i);
            RotateParticleVerlet(i);
        }
        for (size_t i = 0; i < SimulationParameters.PtCount; i++) {
            AccelerateMoveParticleVerlet(i);
            AccelerateRotateParticleVerlet(i);
        }

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
