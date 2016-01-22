//
// Created by mpolovyi on 06/11/15.
//

#ifndef SMALLTESTS_VECTOR_H
#define SMALLTESTS_VECTOR_H

#include <iostream>
#include <math.h>

class CQuaternion;

class CVector {
public:
    double X;
    double Y;
    double Z;
    double W;

    CVector() : X(0), Y(0), Z(0), W(0) { }

    CVector(double x, double y, double z) : X(x), Y(y), Z(z), W(0) { }

    explicit CVector(const CQuaternion &other);

    CVector OuterProduct(const CVector &other) const;

    CVector CrossProduct(const CVector &other) const;

    double DotProduct(const CVector &other) const;

    double GetLength() const;

    double GetLengthSquared() const;

    void Normalize();

    CVector &operator+=(const CVector &other);

    CVector &operator-=(const CVector &other);

    CVector & operator*=(double other);

    CVector & operator/=(double other);

    static const CVector AxisX;
    static const CVector AxisY;
    static const CVector AxisZ;
};

CVector operator+(CVector lhs, const CVector &rhs);

CVector operator-(CVector lhs, const CVector &rhs);

CVector operator*(CVector lhs, double rhs);

CVector operator*(double lhs, CVector rhs);

CVector operator/(CVector lhs, double rhs);

CVector operator/(double lhs, CVector rhs);

std::ostream &operator<<(std::ostream &out, CVector &v);

#endif //SMALLTESTS_VECTOR_H
