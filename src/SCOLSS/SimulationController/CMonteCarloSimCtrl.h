//
// Created by mpolovyi on 25/01/16.
//

#ifndef PROJECT_CMONTECARLOSIMCTRL_H
#define PROJECT_CMONTECARLOSIMCTRL_H

#include <vector>
#include <list>
#include <random>
#include <cereal/cereal.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/archives/json.hpp>
#include <fstream>
#include <cstdlib>
#include <chrono>

#include <SCOLSS/ParticlePhysics/CParticleBase.h>
#include <SCOLSS/ParticlePhysics/CYukawaDipolePt.h>
#include <SCOLSS/EPSPlot/EPSPlot.h>

#include "CBaseSimParams.h"
#include "CMonteCarloSimParams.h"
#include "CBaseSimCtrl.h"

class CMonteCarloSimCtrl : public CBaseSimCtrl {
public:
    const CMonteCarloSimParams SimulationParameters;

    CMonteCarloSimCtrl(CMonteCarloSimParams d) : CBaseSimCtrl(d), SimulationParameters(d), particles(particles_old) { };

    template<class Archive>
    void DoSerialize(Archive &archieve) const {
        CBaseSimCtrl::DoSerialize(archieve);

        archieve(cereal::make_nvp("SimulationCycles", Cycles));
    }

    std::uniform_real_distribution<double> uniformDistributionAcceptance;
    std::uniform_real_distribution<double> uniformDistributionMove;

protected:
    std::vector<CYukawaDipolePt>& particles;

    double oldParticleCoordinates;
    CQuaternion oldParticleRotation;

    bool AcceptMove(double energy, double oldEnergy) {
        bool ret = energy <= oldEnergy;

        if (!ret) {
            auto ex = exp(-(energy - oldEnergy) / SimulationParameters.KbT);
            ret = uniformDistributionAcceptance(rnd_gen) < ex;
        }

        return ret;
    };

    void DoTestMove(size_t ptIndex) {
        auto beforeEnergy = GetParticlePotentialEnergy(ptIndex);
        oldParticleCoordinates = particles[ptIndex].Coordinates;

        double move = uniformDistributionMove(rnd_gen);
        if (SimulationParameters.PtCount == 2) {
            particles[ptIndex].Coordinates += move;
        }
        else {
            particles[ptIndex].Coordinates += move;
            AccountForBorderAfterMove(particles[ptIndex]);
        }

        auto afterEnergy = GetParticlePotentialEnergy(ptIndex);
        if (AcceptMove(afterEnergy, beforeEnergy)) {
            return;
        }
        else {
            particles[ptIndex].Coordinates = oldParticleCoordinates;
        }
    };

    void DoTestRotation(size_t ptIndex) {
        auto beforeEnergy = GetParticlePotentialEnergy(ptIndex);
        oldParticleRotation = particles[ptIndex].Rotation;

        particles[ptIndex].Rotation = GetRandomUnitQuaternion();

        auto afterEnergy = GetParticlePotentialEnergy(ptIndex);
        if (AcceptMove(afterEnergy, beforeEnergy)) {
            return;
        }
        else {
            particles[ptIndex].Rotation = oldParticleRotation;
        }
    };

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
