//
// Created by mpolovyi on 06/11/15.
//

#include "CVector.h"
#include "CQuaternion.h"


CVector operator+(CVector lhs, const CVector &rhs) {
    lhs += rhs;
    return lhs;
}

CVector operator-(CVector lhs, const CVector &rhs) {
    lhs -= rhs;
    return lhs;
}


CVector operator*(CVector lhs, double rhs) {
    lhs *= rhs;
    return lhs;
};

CVector operator*(double lhs, CVector rhs) {
    rhs *= lhs;
    return rhs;
};


CVector operator/(CVector lhs, double rhs) {
    lhs /= rhs;
    return lhs;
};

CVector operator/(double lhs, CVector rhs) {
    rhs /= lhs;
    return rhs;
};

std::ostream &operator<<(std::ostream &out, CVector &v) {
    return out << '(' << v.X << ", " << v.Y << ", " << v.Z << ')';
};

CVector::CVector(const CQuaternion &other) : CVector(other.V) {
    W = other.W;
}

const CVector CVector::AxisX = CVector(1,0,0);
const CVector CVector::AxisY = CVector(0,1,0);
const CVector CVector::AxisZ = CVector(0,0,1);

CVector CVector::OuterProduct(const CVector &other) const {
    CVector ret(
            other.X * X,
            other.Y * Y,
            other.Z * Z);
    return ret;
}

CVector CVector::CrossProduct(const CVector &other) const {
    CVector ret(
            +Y * other.Z - Z * other.Y,
            -X * other.Z + Z * other.X,
            +X * other.Y - Y * other.X
    );
    return ret;
};

double CVector::DotProduct(const CVector &other) const  {
    return X * other.X + Y * other.Y + Z * other.Z;
}

double CVector::GetLength() const {
    return sqrt(GetLengthSquared());
}

double CVector::GetLengthSquared() const {
    return X*X + Y*Y + Z*Z;
}

void CVector::Normalize() {
    double len = GetLength();
    if (len == 0) {
        X = 0;
        Y = 0;
        Z = 0;
        W = 0;
        return;
    }
    *this /= len;
}

CVector &CVector::operator+=(const CVector &other) {
    X += other.X;
    Y += other.Y;
    Z += other.Z;

    return *this;
}

CVector &CVector::operator-=(const CVector &other) {
    X -= other.X;
    Y -= other.Y;
    Z -= other.Z;

    return *this;
}

CVector &CVector::operator*=(double other) {
    X *= other;
    Y *= other;
    Z *= other;

    return *this;
}

CVector &CVector::operator/=(double other) {
    X /= other;
    Y /= other;
    Z /= other;

    return *this;
}