//
// Created by mpolovyi on 11/11/15.
//

#ifndef SMALLTESTS_PARTICLE_H
#define SMALLTESTS_PARTICLE_H

#include <vector>
#include "CParticleBase.h"

class CParticle : public CParticleBase {
public:
    double A;
    double k;
    double SystemSize;
    double HalfSystemSize;

    CParticle() : CParticle(1000, 10, 10) { }

    CParticle(double _A, double _k, double _systemSize,
              CQuaternion _Rotation = CQuaternion(0, CVector(0, 0, 1))) :
            A(_A),
            k(_k),
            SystemSize(_systemSize),
            HalfSystemSize(_systemSize / 2.0)
    {
        Rotation = _Rotation;
        Velocity = 0;
        Coordinates = 0;
        ForceRandom = 0;
        ForceOld = 0;
    }

    CVector AngularVelocity;
    double Velocity;

    double ForceOld;
    double ForceRandom;

    CVector TorqueOld;
    CVector TorqueRandom;

    CQuaternion GetRotationFromVelocity(std::vector<CVector> angVelocity, double dt);

    double GetForceFromOtherTheoretically(const CParticle &other, CVector dr_from_other) const;

    double GetForceFromOther(const CParticle &other_left, const CParticle &other_right) const;

    CVector GetTorqueFromOther(const CParticle &other, CVector dr_from_other) const;

    CVector GetTorqueFromOther(const CParticle &other_left, const CParticle &other_right) const;

    CVector GetDipoleFieldFromOther(const CParticle &other, CVector dr_from_other) const;

    double GetYukawaPotentialFromOther(const CParticle &other, CVector dr_from_other) const;

    double GetPotentialEnergy(const CParticle &other, CVector dr) const;

    double GetEnergy(const CVector &orientation, const CVector &other_orientation, CVector dr) const;

    CVector ToLocalSpace(CVector vector) const;

    static CVector GetRotationDistance(const CParticle &now, const CParticle &before);

    CVector AngularDisplacement;
};


#endif //SMALLTESTS_PARTICLE_H
