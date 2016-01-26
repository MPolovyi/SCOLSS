//
// Created by mpolovyi on 25/01/16.
//

#ifndef PROJECT_CMONTECARLOSIMCTRL_H
#define PROJECT_CMONTECARLOSIMCTRL_H

#include <vector>
#include <random>
#include <cereal/cereal.hpp>
#include <cereal/types/polymorphic.hpp>
#include <cereal/archives/json.hpp>
#include <cstdlib>

#include <SCOLSS/ParticlePhysics/CParticleBase.h>
#include <SCOLSS/ParticlePhysics/CYukawaDipolePt.h>

#include <SCOLSS/SimulationController/BaseSimCtrl/CBaseSimCtrl.h>
#include <SCOLSS/SimulationController/MonteCarloSimCtrl/CMonteCarloSimParams.h>

class CMonteCarloSimCtrl : public CBaseSimCtrl {
public:
    CMonteCarloSimParams SimulationParameters;

    CMonteCarloSimCtrl(CMonteCarloSimParams d) : CBaseSimCtrl(d), SimulationParameters(d) {
        InitRandomGenerator();
        int i = 1;
    };

    template<class Archive>
    void save(Archive &archieve) const {
        archieve(cereal::base_class<CBaseSimCtrl>(this));
        archieve(cereal::make_nvp("SimulationParameters", SimulationParameters));
        archieve(cereal::make_nvp("SimulationCycles", Cycles));
    }

    std::uniform_real_distribution<double> uniformDistributionAcceptance;
    std::uniform_real_distribution<double> uniformDistributionMove;

protected:
    double oldParticleCoordinates;
    CQuaternion oldParticleRotation;

    bool AcceptMove(double energy, double oldEnergy);;

    void DoTestMove(size_t ptIndex);;

    void DoTestRotation(size_t ptIndex);;

    virtual void DoCycle() {
        Cycles++;

        for (size_t pt = 0; pt < SimulationParameters.PtCount; ++pt) {
            DoTestMove(pt);
            DoTestRotation(pt);
        }

    };

    virtual void InitRandomGenerator() {
        CBaseSimCtrl::InitRandomGenerator();

        uniformDistributionAcceptance = std::uniform_real_distribution<double>(0, 1);

        uniformDistributionMove = std::uniform_real_distribution<double>(-SimulationParameters.MaxTestDisplacement / 2,
                                                                         SimulationParameters.MaxTestDisplacement / 2);

    };
};

#endif //PROJECT_CMONTECARLOSIMCTRL_H
