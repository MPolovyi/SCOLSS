//
// Created by mpolovyi on 11/11/15.
//

#include "CYukawaDipolePt.h"
#include <cmath>

CVector CYukawaDipolePt::GetRotationDistance(const CYukawaDipolePt &now, const CYukawaDipolePt &before) {
    CQuaternion spanned_distance = now.Rotation * before.Rotation.GetInverse();

    return spanned_distance.GetAngularVelocityRepresentation();
}

CVector CYukawaDipolePt::ToLocalSpace(CVector vector) const {
    return Rotation.GetInverse() * vector * Rotation;
}

double CYukawaDipolePt::GetPotentialEnergy(const CYukawaDipolePt &other, CVector dr) const {
    //Yukawa field is directed along the dr vector. But since it's one dimensional case, let's leave it as it is.
    //for 3D case replace .Z with Length()??

    return GetYukawaPotentialFromOther(other, dr) - Orientation.DotProduct(GetDipoleFieldFromOther(other, dr));
}

double CYukawaDipolePt::GetYukawaPotentialFromOther(const CYukawaDipolePt &other, CVector dr_from_other) const {
    double dist = dr_from_other.GetLength();
    auto Yukawa = A * exp(-k * dist) / dist;

    return Yukawa;
}

CVector CYukawaDipolePt::GetDipoleFieldFromOther(const CYukawaDipolePt &other, CVector dr_from_other) const {
    double dist = dr_from_other.GetLength();
    dr_from_other.Normalize();
    CVector other_orientation = other.Orientation;
    auto tmp = 3 * other_orientation.DotProduct(dr_from_other) * dr_from_other;
    auto Dipole = (3 * other_orientation.DotProduct(dr_from_other) * dr_from_other - other_orientation) / (dist * dist * dist);

    return Dipole;
}

CVector CYukawaDipolePt::GetDipoleFieldFromOther1D(const CYukawaDipolePt &other, CVector dr_from_other) const {
    double dist = std::abs(dr_from_other.Z);

    dr_from_other.Z = 3 * other.Orientation.Z;

    return (dr_from_other - other.Orientation) / (dist * dist * dist);
}

double CYukawaDipolePt::GetPotentialEnergy1D(const CYukawaDipolePt &other, CVector dr) const {
    //Yukawa field is directed along the dr vector. But since it's one dimensional case, let's leave it as it is.
    //for 3D case replace .Z with Length()??

    return GetYukawaPotentialFromOther1D(other, dr) - Orientation.DotProduct(GetDipoleFieldFromOther1D(other, dr));
}

double CYukawaDipolePt::GetYukawaPotentialFromOther1D(const CYukawaDipolePt &other, const CVector &dr_from_other) const {
    double dist = std::abs(dr_from_other.Z);
    return A * exp(-k * dist) / dist;
}

CVector CYukawaDipolePt::GetTorqueFromOther(const CYukawaDipolePt &other_left, const CYukawaDipolePt &other_right) const {
    return GetTorqueFromOther(other_left, other_left.GetDistanceRight(*this, SystemSize))
           + GetTorqueFromOther(other_right, other_right.GetDistanceLeft(*this, SystemSize));
}

CVector CYukawaDipolePt::GetTorqueFromOther(const CYukawaDipolePt &other, CVector dr_from_other) const {
    return Orientation.CrossProduct(GetDipoleFieldFromOther1D(other, dr_from_other));
}

double CYukawaDipolePt::GetForceFromOther(const CYukawaDipolePt &other_left, const CYukawaDipolePt &other_right) const {
    return GetForceFromOtherTheoretically(other_left, other_left.GetDistanceRight(*this, SystemSize))
           + GetForceFromOtherTheoretically(other_right, other_right.GetDistanceLeft(*this, SystemSize));
}

double CYukawaDipolePt::GetForceFromOtherTheoretically(const CYukawaDipolePt &other, CVector dr_from_other) const {
    CVector orientation = Orientation;
    CVector other_orientation = other.Orientation;

    double dist = dr_from_other.GetLength();
    dr_from_other.Normalize();

    auto D = (3 * orientation.DotProduct(dr_from_other) * other_orientation.DotProduct(dr_from_other)
              - orientation.DotProduct(other_orientation));

    auto ret = (3 * D - A * dist * dist * exp(-k * dist) * (k * dist + 1)) / pow(dist, 4);

    return -ret*dr_from_other.Z;
}

CQuaternion CYukawaDipolePt::GetRotationFromVelocity(std::vector<CVector> angVelocity, double dt) {
    CQuaternion rot;
    for (CVector &vec : angVelocity) {
        double len = vec.GetLength() * dt;
        vec.Normalize();
        rot *= CQuaternion(len, vec);
    }
    return rot * Rotation;
}

//double CYukawaDipolePt::GetPotentialEnergy1D(const CYukawaDipolePt &other) const {
//    const CVector &orientation = Orientation;
//    const CVector &other_orientation = other.Orientation;
//    return GetPotentialEnergy1D(Coordinates, orientation, other.Coordinates, other_orientation);
//}

