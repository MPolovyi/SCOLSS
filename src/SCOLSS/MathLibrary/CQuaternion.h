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
    CVector V;

    double W;

    CQuaternion() : V(CVector()), W(1) { }

    CQuaternion(double angle, const CVector &axis);

    CQuaternion(const CQuaternion &other) : V(other.V), W(other.W) { };

    CQuaternion(double w, double x, double y, double z) : V(x, y, z), W(w) { };

    explicit CQuaternion(const CVector &other) : V(other), W(other.W) { }

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
