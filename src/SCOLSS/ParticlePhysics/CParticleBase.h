//
// Created by mpolovyi on 18/01/16.
//

#ifndef COLLOIDMD_CPARTICLEBASE_H
#define COLLOIDMD_CPARTICLEBASE_H

#include <SCOLSS/MathLibrary/CVector.h>
#include <SCOLSS/MathLibrary/CQuaternion.h>

class CParticleBase {
public:
    double Coordinates;

    CQuaternion GetRotation() const {
        return Rotation;
    }

    void SetRotation(CQuaternion nRotation) {
        Rotation = nRotation;
    }

    CVector ToGlobalSpace(CVector vector) const {
        return Rotation * vector * Rotation.GetInverse();
    };

    CVector GetDistanceLeft(const CParticleBase &other_left, double SystemSize) const {
        CVector dr(0, 0, other_left.Coordinates - Coordinates);

        if (dr.Z > 0)
            dr.Z = dr.Z - SystemSize;

        return dr;
    }

    CVector GetDistanceRight(const CParticleBase &other_right, double SystemSize) const {
        CVector dr(0, 0, other_right.Coordinates - Coordinates);

        if (dr.Z < 0)
            dr.Z = SystemSize + dr.Z;

        return dr;
    }

    CVector GetOrientation() const {
        return ToGlobalSpace(CVector(0, 0, 1));
    };
protected:
    CQuaternion Rotation;
};


#endif //COLLOIDMD_CPARTICLEBASE_H
