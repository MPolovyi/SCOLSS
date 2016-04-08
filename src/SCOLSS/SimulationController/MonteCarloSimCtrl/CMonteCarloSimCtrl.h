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
        int currentId = MPI::COMM_WORLD.Get_rank();
        InitRandomGenerator();
    };

    template<class Archive>
    void save(Archive& archive) const {
        archive(cereal::base_class<CBaseSimCtrl>(this));

        archive(cereal::make_nvp("SimulationParameters", SimulationParameters));
        archive(cereal::make_nvp("SimulationCycles", Cycles));
    }

    std::uniform_real_distribution<double> uniformDistributionAcceptance;
    std::uniform_real_distribution<double> uniformDistributionMove;

protected:
    double oldParticleCoordinates;
    CQuaternion oldParticleRotation;

    bool AcceptMove(double energy, double oldEnergy);

    void DoTestMove(size_t ptIndex);

    void DoTestRotation(size_t ptIndex);

    virtual void DoCycle() {
        int currentId = MPI::COMM_WORLD.Get_rank();
        Cycles += 1;

        int id = MPI::COMM_WORLD.Get_rank();
        for (size_t i = ProcessMapFull[id].beginIndex(); i < ProcessMapFull[id].endIndex(); i++) {
            DoTestMove(i);
            DoTestRotation(i);
        }

        SyncInCycle();
    };

    virtual void InitRandomGenerator() {
        int currentId = MPI::COMM_WORLD.Get_rank();

        CBaseSimCtrl::InitRandomGenerator();

        uniformDistributionAcceptance = std::uniform_real_distribution<double>(0, 1);

        uniformDistributionMove = std::uniform_real_distribution<double>(-SimulationParameters.MaxTestDisplacement / 2,
                                                                         SimulationParameters.MaxTestDisplacement / 2);

    };
};

#endif //PROJECT_CMONTECARLOSIMCTRL_H
