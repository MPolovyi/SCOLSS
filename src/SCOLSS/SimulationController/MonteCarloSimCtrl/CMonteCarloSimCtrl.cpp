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
    particles_old[ptIndex].Moved = true;

    auto afterEnergy = GetParticlePotentialEnergy(ptIndex);

    if (AcceptMove(afterEnergy, beforeEnergy)) {
        particles_old[ptIndex].PotentialEnergy = particles_old[ptIndex].NewPotentialEnergy;
        return;
    }
    else {
        particles_old[ptIndex].Coordinates = oldParticleCoordinates;
        particles_old[ptIndex].Moved = false;
    }
}

void CMonteCarloSimCtrl::DoTestRotation(size_t ptIndex) {
    auto beforeEnergy = GetParticlePotentialEnergy(ptIndex);

    oldParticleRotation = particles_old[ptIndex].GetRotation();

    std::vector<CVector> rv = {GetNormalRandomVector(normalDistributionRotation)};

    particles_old[ptIndex].SetRotation(particles_old[ptIndex].GetRotationFromVelocity(rv, 1));
    particles_old[ptIndex].Moved = true;

    auto afterEnergy = GetParticlePotentialEnergy(ptIndex);

    if (AcceptMove(afterEnergy, beforeEnergy)) {
        particles_old[ptIndex].PotentialEnergy = particles_old[ptIndex].NewPotentialEnergy;
        return;
    }
    else {
        particles_old[ptIndex].SetRotation(oldParticleRotation);
        particles_old[ptIndex].Moved = false;
    }
}

#include <cereal/types/polymorphic.hpp>
#include <cereal/archives/json.hpp>
#include <cereal/archives/binary.hpp>

CEREAL_REGISTER_TYPE(CMonteCarloSimCtrl);