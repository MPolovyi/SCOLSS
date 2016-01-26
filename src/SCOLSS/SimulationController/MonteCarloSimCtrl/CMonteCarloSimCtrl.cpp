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
    auto beforeEnergy = GetParticlePotentialEnergy(ptIndex);
    oldParticleCoordinates = particles_old[ptIndex].Coordinates;

    double move = uniformDistributionMove(rnd_gen);

    particles_old[ptIndex].Coordinates += move;
    AccountForBorderAfterMove(particles_old[ptIndex]);

    auto afterEnergy = GetParticlePotentialEnergy(ptIndex);
    if (AcceptMove(afterEnergy, beforeEnergy)) {
        return;
    }
    else {
        particles_old[ptIndex].Coordinates = oldParticleCoordinates;
    }
}

void CMonteCarloSimCtrl::DoTestRotation(size_t ptIndex) {
    auto beforeEnergy = GetParticlePotentialEnergy(ptIndex);
    oldParticleRotation = particles_old[ptIndex].Rotation;

    particles_old[ptIndex].Rotation = GetRandomUnitQuaternion();

    auto afterEnergy = GetParticlePotentialEnergy(ptIndex);
    if (AcceptMove(afterEnergy, beforeEnergy)) {
        return;
    }
    else {
        particles_old[ptIndex].Rotation = oldParticleRotation;
    }
}

#include <cereal/types/polymorphic.hpp>
#include <cereal/archives/json.hpp>

CEREAL_REGISTER_TYPE(CMonteCarloSimCtrl);