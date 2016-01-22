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
    CQuaternion Rotation;

    CVector GetOrientation() const {
        return ToGlobalSpace(CVector(0, 0, 1));
    };

    CVector ToGlobalSpace(CVector vector, const CQuaternion *otherRotatation = nullptr) const {
        if (otherRotatation)
            return (*otherRotatation) * vector * otherRotatation->GetInverse();

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
};


#endif //COLLOIDMD_CPARTICLEBASE_H
