//
// Created by mpolovyi on 25/01/16.
//

#include "CLangevinSimCtrl.h"

void CLangevinSimCtrl::MoveParticleVerlet(size_t ptIndex) {
    CYukawaDipolePt &pt_new = particles_new[ptIndex];
    CYukawaDipolePt &pt_old = particles_old[ptIndex];

    pt_new.ForceRandom = translationNormalDistribution(rnd_gen);

    pt_new.ForceOld = GetForceOld(ptIndex);

    auto av = SimulationParameters.VerletCoefficientTranslation * pt_old.Velocity
              + SimulationParameters.VerletCoefficientTranslation * pt_new.ForceRandom / (2 * SimulationParameters.ParticleMass)
              + SimulationParameters.VerletCoefficientTranslation * pt_new.ForceOld * SimulationParameters.TimeStep /
                (2 * SimulationParameters.ParticleMass);

    pt_new.Coordinates =
            pt_old.Coordinates
            + av * SimulationParameters.TimeStep;
}

void CLangevinSimCtrl::AccelerateMoveParticleVerlet(size_t ptIndex) {
    CYukawaDipolePt &pt_new = particles_new[ptIndex];
    CYukawaDipolePt &pt_old = particles_old[ptIndex];

    auto dv = SimulationParameters.TimeStep * (pt_new.ForceOld + GetForceNew(ptIndex)) / (2 * SimulationParameters.ParticleMass)
              - SimulationParameters.AlphaTranslational * (pt_new.Coordinates - pt_old.Coordinates) / SimulationParameters.ParticleMass
              + pt_new.ForceRandom / SimulationParameters.ParticleMass;

    pt_new.Velocity = pt_old.Velocity + dv;

    AccountForBorderAfterMove(pt_new);
}

void CLangevinSimCtrl::RotateParticleVerlet(size_t ptIndex) {
    CYukawaDipolePt &pt_new = particles_new[ptIndex];
    CYukawaDipolePt &pt_old = particles_old[ptIndex];

    pt_new.TorqueRandom = GetNormalRandomVector(rotationNormalDistribution);
    pt_new.TorqueOld = GetTorqueOld(ptIndex);

    std::vector<CVector> av = {
            SimulationParameters.VerletCoefficientRotation * pt_old.AngularVelocity,
            SimulationParameters.VerletCoefficientRotation * pt_new.TorqueRandom / (2 * SimulationParameters.Inertia),
            SimulationParameters.VerletCoefficientRotation * pt_new.TorqueOld * SimulationParameters.TimeStep /
            (2 * SimulationParameters.Inertia)
    };

    pt_new.Rotation = pt_old.GetRotationFromVelocity(av, SimulationParameters.TimeStep);

    pt_new.AngularDisplacement = pt_old.AngularDisplacement + CYukawaDipolePt::GetRotationDistance(pt_new, pt_old);
}

void CLangevinSimCtrl::AccelerateRotateParticleVerlet(size_t ptIndex) {
    CYukawaDipolePt &pt_new = particles_new[ptIndex];
    CYukawaDipolePt &pt_old = particles_old[ptIndex];

    pt_new.AngularVelocity = pt_old.AngularVelocity
                             + SimulationParameters.TimeStep * (pt_new.TorqueOld + GetTorqueNew(ptIndex)) / (2 * SimulationParameters.Inertia)
                             - SimulationParameters.AlphaRotational * (CYukawaDipolePt::GetRotationDistance(pt_new, pt_old)) / SimulationParameters.Inertia
                             + pt_new.TorqueRandom / SimulationParameters.Inertia;
}

double CLangevinSimCtrl::GetForceOld(size_t ptIndex) {
    return particles_old[ptIndex].GetForceFromOther(particles_old[GetPrevious(ptIndex)],
                                                    particles_old[GetNext(ptIndex)]);

}

double CLangevinSimCtrl::GetForceNew(size_t ptIndex) {

    return particles_new[ptIndex].GetForceFromOther(particles_old[GetPrevious(ptIndex)],
                                                    particles_old[GetNext(ptIndex)]);

}

CVector CLangevinSimCtrl::GetTorqueOld(size_t ptIndex) {
    return particles_old[ptIndex].GetTorqueFromOther(particles_old[GetPrevious(ptIndex)],
                                                     particles_old[GetNext(ptIndex)]);

}

CVector CLangevinSimCtrl::GetTorqueNew(size_t ptIndex) {
    return particles_new[ptIndex].GetTorqueFromOther(particles_new[GetPrevious(ptIndex)],
                                                     particles_new[GetNext(ptIndex)]);

}

double CLangevinSimCtrl::GetAverageKineticEnergy() const {
    double ret = 0;

    for (size_t i = 0; i < SimulationParameters.PtCount; i++) {
        auto &pt = particles_old[i];

        ret += SimulationParameters.ParticleMass * pt.Velocity * pt.Velocity / 2;
        ret += SimulationParameters.Inertia * pt.AngularVelocity.GetLengthSquared() / 2;
    }

    return ret/SimulationParameters.PtCount;
}

double CLangevinSimCtrl::GetRotationalEnergy() const {
    double ret = 0;

    for (size_t i = 0; i < SimulationParameters.PtCount; i++) {
        auto &pt = particles_old[i];

        ret += SimulationParameters.Inertia * pt.AngularVelocity.GetLengthSquared() / 2;
    }

    return ret;
}

double CLangevinSimCtrl::GetTranslationalEnergy() const {
    double ret = 0;

    for (size_t i = 0; i < SimulationParameters.PtCount; i++) {
        auto &pt = particles_old[i];

        ret += SimulationParameters.ParticleMass * pt.Velocity * pt.Velocity / 2;
    }

    return ret;
}

double CLangevinSimCtrl::GetSimulationTime() const {
    return Cycles * SimulationParameters.TimeStep;
}

double CLangevinSimCtrl::GetAverageDispl() const {
    double ret = 0;

    for (size_t i = 0; i < particles_old.size(); ++i) {
        const CYukawaDipolePt &pt_old = particles_old[i];
        auto dr = pow(pt_old.Coordinates, 2);

        ret += dr;
    }

    return ret / SimulationParameters.PtCount;
}

double CLangevinSimCtrl::GetAverAngularDisplX() const {
    double ret = 0;

    for (size_t i = 0; i < particles_old.size(); ++i) {
        const CYukawaDipolePt &pt_old = particles_old[i];

        ret += pow(pt_old.AngularDisplacement.X, 2);
    }

    return ret / SimulationParameters.PtCount;
}

double CLangevinSimCtrl::GetAverAngularDispl() const {
    double ret = 0;

    for (size_t i = 0; i < particles_old.size(); ++i) {
        const CYukawaDipolePt &pt_old = particles_old[i];

        ret += pt_old.AngularDisplacement.GetLengthSquared();
    }

    return ret / SimulationParameters.PtCount;
}

double CLangevinSimCtrl::GetAverAngularDisplY() const {
    double ret = 0;

    for (size_t i = 0; i < particles_old.size(); ++i) {
        const CYukawaDipolePt &pt_old = particles_old[i];

        ret += pow(pt_old.AngularDisplacement.Y, 2);
    }

    return ret / SimulationParameters.PtCount;
}

double CLangevinSimCtrl::GetAverAngularDisplZ() const {
    double ret = 0;

    for (size_t i = 0; i < particles_old.size(); ++i) {
        const CYukawaDipolePt &pt_old = particles_old[i];

        ret += pow(pt_old.AngularDisplacement.Z, 2);
    }

    return ret / SimulationParameters.PtCount;
}

CVector CLangevinSimCtrl::GetNormalRandomVector(std::normal_distribution<double> &normalDistribution) {
    return CVector(normalDistribution(rnd_gen), normalDistribution(rnd_gen), normalDistribution(rnd_gen));
}

#include <cereal/types/polymorphic.hpp>
#include <cereal/archives/json.hpp>

CEREAL_REGISTER_TYPE(CLangevinSimCtrl);