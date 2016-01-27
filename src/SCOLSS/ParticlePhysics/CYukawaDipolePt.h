//
// Created by mpolovyi on 11/11/15.
//

#ifndef SMALLTESTS_PARTICLE_H
#define SMALLTESTS_PARTICLE_H

#include <vector>
#include <SCOLSS/ParticlePhysics/CParticleBase.h>

class CYukawaDipolePt : public CParticleBase {
public:
    double A;
    double k;
    double SystemSize;
    double HalfSystemSize;

    CYukawaDipolePt() : CYukawaDipolePt(1000, 10, 10) { }

    CYukawaDipolePt(double _A, double _k, double _systemSize,
                    CQuaternion _Rotation = CQuaternion(0, CVector(0, 0, 1))) :
            A(_A),
            k(_k),
            SystemSize(_systemSize),
            HalfSystemSize(_systemSize / 2.0)
    {
        SetRotation(_Rotation);
        Velocity = 0;
        Coordinates = 0;
        ForceRandom = 0;
        ForceOld = 0;
    }

    void SetRotation(CQuaternion nRotation) {
        Rotation = nRotation;
        Orientation = CParticleBase::GetOrientation();
    }

    CVector Orientation;

    CVector AngularVelocity;
    double Velocity;

    double ForceOld;
    double ForceRandom;

    CVector TorqueOld;
    CVector TorqueRandom;

    CQuaternion GetRotationFromVelocity(std::vector<CVector> angVelocity, double dt);

    double GetForceFromOtherTheoretically(const CYukawaDipolePt &other, CVector dr_from_other) const;

    double GetForceFromOther(const CYukawaDipolePt &other_left, const CYukawaDipolePt &other_right) const;

    CVector GetTorqueFromOther(const CYukawaDipolePt &other, CVector dr_from_other) const;

    CVector GetTorqueFromOther(const CYukawaDipolePt &other_left, const CYukawaDipolePt &other_right) const;

    CVector GetDipoleFieldFromOther(const CYukawaDipolePt &other, CVector dr_from_other) const;

    double GetYukawaPotentialFromOther(const CYukawaDipolePt &other, CVector dr_from_other) const;

    double GetPotentialEnergy(const CYukawaDipolePt &other, CVector dr) const;

    CVector ToLocalSpace(CVector vector) const;

    static CVector GetRotationDistance(const CYukawaDipolePt &now, const CYukawaDipolePt &before);

    CVector AngularDisplacement;
};


#endif //SMALLTESTS_PARTICLE_H
