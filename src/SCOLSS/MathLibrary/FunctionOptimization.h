//
// Created by mpolovyi on 01/10/15.
//
#ifndef COLLOIDMC_FUNCTIONOPTIMIZATION_H
#define COLLOIDMC_FUNCTIONOPTIMIZATION_H

#include <iostream>
#include <functional>
#include <cmath>
#include <vector>

#define M_2PI 6.283185307179586232

typedef std::function<double(double)> func;

inline double Derivative(func _f, double x_0);

inline double SecondDerivative(func _f, double x_0);

double FindZero(func f, double x_0);

double FindMinimum(func _f, double x_0);

struct MinMaxZeros{
    double Zeros[2];
    double Min;
    double Max;
};

class InteractionFunction{
public:
    double _interactionEnergy;
    double _divider;
    double _YukawaA;
    double _YukawaK;
    double _minLim;
    double _maxLim;

    double _lhsAngle;
    double _rhsAngle;

    InteractionFunction(double interactionEnergy,
                        double YukawaA,
                        double YukawaK,
                        double interAngle) {
        _interactionEnergy = 1;
        _YukawaA = YukawaA;
        _YukawaK = YukawaK;
        _divider = interAngle / 90.0;
        _minLim = M_PI * _divider;
        _maxLim = M_2PI - _minLim;
        _lhsAngle = 0;
        _rhsAngle = 0;
    }

    double InteractDipole(double dist);

    double InteractYukawa(double dist);

    double operator()(double dist);

    MinMaxZeros GetMinMaxZero();
};

#endif //COLLOIDMC_FUNCTIONOPTIMIZATION_H
