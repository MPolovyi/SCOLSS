//
// Created by mpolovyi on 06/11/15.
//

#ifndef SMALLTESTS_QUATERNION_H
#define SMALLTESTS_QUATERNION_H

#include <math.h>
#include <iostream>
#include "CVector.h"

class CQuaternion {
public:
    double W;

    CVector V;

    CQuaternion() : W(1), V(CVector()) { }

    CQuaternion(double angle, const CVector &axis);

    CQuaternion(const CQuaternion &other) : W(other.W), V(other.V) { };

    CQuaternion(double w, double x, double y, double z) : W(w), V(x, y, z) { };

    explicit CQuaternion(const CVector &other) : W(V.W), V(other) { }

    static CQuaternion GetRotationFromVectorToVector(const CVector &a, const CVector &b);

    double GetLength() const;

    CQuaternion &operator*=(const CQuaternion &other);

    CQuaternion &operator*=(double other);

    CQuaternion &operator/=(double other);

    CQuaternion &operator+=(const CQuaternion &other);

    CQuaternion &operator-=(const CQuaternion &other);

    CQuaternion GetInverse() const;

    CVector GetAngularVelocityRepresentation() const;
};

CQuaternion operator*(CQuaternion lhs, const CQuaternion &rhs);

CQuaternion operator+(CQuaternion lhs, const CQuaternion &rhs);

CQuaternion operator-(CQuaternion lhs, const CQuaternion &rhs);

CQuaternion operator*(CQuaternion lhs, double rhs);

CQuaternion operator*(double lhs, CQuaternion rhs);

CQuaternion operator/(CQuaternion lhs, double rhs);

CQuaternion operator/(double lhs, CQuaternion rhs);

std::ostream &operator<<(std::ostream &out, CQuaternion &q);

CVector operator*(const CVector &lhs, const CQuaternion &rhs);

CVector operator*(const CQuaternion &lhs, const CVector &rhs);

#endif //SMALLTESTS_QUATERNION_H
