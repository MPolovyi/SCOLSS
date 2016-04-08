//
// Created by mpolovyi on 25/01/16.
//

#include "CMonteCarloSimCtrl.h"

bool CMonteCarloSimCtrl::AcceptMove(double energy, double oldEnergy) {
    bool ret = energy <= oldEnergy;

    if (!ret) {
        auto ex = exp(-(energy - oldEnergy) / SimulationParameters.KbT);
        ret = uniformDistributionAcceptance(rnd_gen) < ex;
    }

    return ret;
}

void CMonteCarloSimCtrl::DoTestMove(size_t ptIndex) {
    auto beforeEnergy = GetParticlePotentialEnergy(GetPrevious(ptIndex));

    oldParticleCoordinates = particles_old(__PRETTY_FUNCTION__)[ptIndex].Coordinates;

    double move = uniformDistributionMove(rnd_gen);

    particles_old(__PRETTY_FUNCTION__)[ptIndex].Coordinates += move;
    AccountForBorderAfterMove(particles_old(__PRETTY_FUNCTION__)[ptIndex]);

    auto afterEnergy = GetParticlePotentialEnergy(GetPrevious(ptIndex));

    if (AcceptMove(afterEnergy, beforeEnergy)) {
        return;
    }
    else {
        particles_old(__PRETTY_FUNCTION__)[ptIndex].Coordinates = oldParticleCoordinates;
    }
}

void CMonteCarloSimCtrl::DoTestRotation(size_t ptIndex) {
    auto beforeEnergy = GetParticlePotentialEnergy(GetPrevious(ptIndex));

    oldParticleRotation = particles_old(__PRETTY_FUNCTION__)[ptIndex].GetRotation();

    particles_old(__PRETTY_FUNCTION__)[ptIndex].SetRotation(GetRandomUnitQuaternion());

    auto afterEnergy = GetParticlePotentialEnergy(GetPrevious(ptIndex));

    if (AcceptMove(afterEnergy, beforeEnergy)) {
        return;
    }
    else {
        particles_old(__PRETTY_FUNCTION__)[ptIndex].SetRotation(oldParticleRotation);
    }
}

#include <cereal/types/polymorphic.hpp>
#include <cereal/archives/json.hpp>

CEREAL_REGISTER_TYPE(CMonteCarloSimCtrl);