//
// Created by mpolovyi on 06/11/15.
//

#include "CQuaternion.h"
#include "CVector.h"

CQuaternion &CQuaternion::operator*=(const CQuaternion &other) {
    double w = this->W * other.W - this->V.DotProduct(other.V);
    this->V = this->W * other.V + other.W * this->V + this->V.CrossProduct(other.V);
    this->W = w;
    return *this;
}

CQuaternion operator*(CQuaternion lhs, const CQuaternion &rhs) {
    lhs *= rhs;
    return lhs;
};

CQuaternion &CQuaternion::operator+=(const CQuaternion &other) {
    this->W += other.W;
    this->V += other.V;
    return *this;
}

CQuaternion operator+(CQuaternion lhs, const CQuaternion &rhs) {
    lhs += rhs;
    return lhs;
};

CQuaternion &CQuaternion::operator-=(const CQuaternion &other) {
    this->W -= other.W;
    this->V -= other.V;

    return *this;
}

CQuaternion operator-(CQuaternion lhs, const CQuaternion &rhs) {
    lhs += rhs;
    return lhs;
};

CQuaternion::CQuaternion(double angle, const CVector &axis) {
    W = cos(angle / 2);
    V = sin(angle / 2) * axis;
}

CQuaternion &CQuaternion::operator*=(double other) {
    this->W *= other;
    this->V *= other;

    return *this;
}

CQuaternion &CQuaternion::operator/=(double other) {
    this->W /= other;
    this->V /= other;

    return *this;
}

CQuaternion operator*(CQuaternion lhs, double rhs) {
    lhs *= rhs;
    return lhs;
};

CQuaternion operator*(double lhs, CQuaternion rhs) {
    rhs *= lhs;
    return rhs;
};

CQuaternion operator/(CQuaternion lhs, double rhs) {
    lhs /= rhs;
    return lhs;
};

CQuaternion operator/(double lhs, CQuaternion rhs) {
    rhs /= lhs;
    return rhs;
};

CQuaternion CQuaternion::GetInverse() const {
    CQuaternion ret(*this);
    ret.V = -1 * ret.V;
    return ret;
}

std::ostream &operator<<(std::ostream &out, CQuaternion &q) {
    return out << '(' << q.W << ", " << q.V << ')';
}

CVector operator*(const CVector &lhs, const CQuaternion &rhs) {
    CQuaternion tmp;
    tmp.W = lhs.W;
    tmp.V = lhs;

    CVector ret = CVector(tmp * rhs);

    return ret;
}

CVector operator*(const CQuaternion &lhs, const CVector &rhs) {
    CQuaternion tmp;
    tmp.W = rhs.W;
    tmp.V = rhs;

    CVector ret = CVector(lhs * tmp);

    return ret;
}

double CQuaternion::GetLength() const {
    return sqrt(W * W + V.GetLength() * V.GetLength());
}

CVector CQuaternion::GetAngularVelocityRepresentation() const {
    double angle = 2*acos(W);
    CVector ret = V;
    ret.Normalize();

    return ret*angle;
}

CQuaternion CQuaternion::GetRotationFromVectorToVector(const CVector &a, const CVector &b)  {
    double angle = acos(a.DotProduct(b) / a.GetLength() / b.GetLength());
    CVector axis = a.CrossProduct(b);
    axis.Normalize();

    return CQuaternion(angle, -1 * axis);
}
