//
// Created by mpolovyi on 01/10/15.
//

#include "FunctionOptimization.h"

//
// Created by mpolovyi on 01/10/15.
//
inline double Derivative(func _f, double x_0){
    auto y_0 = _f(x_0);
    double d_x = std::min(0.0001, x_0*0.001);
    auto d_y = (_f(x_0 + d_x) - y_0);

    return d_y/d_x;
}

inline double SecondDerivative(func _f, double x_0){
    auto y_0 = Derivative(_f, x_0);
    double d_x = std::min(0.0001, x_0*0.001);
    auto d_y = (Derivative(_f, x_0 + d_x) - y_0);

    return d_y/d_x;
}

double FindZero(func f, double x_0){
    double der = Derivative(f, x_0);
    double func = f(x_0);
    while (std::abs(func) > 0.00001){
        x_0 = x_0 - func/der;
        func = f(x_0);
        der = Derivative(f, x_0);
    }
//    std::cout << "X = " << x_0 << " Func = " << func << " Deriv = " << der << std::endl;
    return x_0;
}

double FindMinimum(func _f, double x_0){
//    double func = _f(x_0);
    double der = Derivative(_f, x_0);
    double sDer = SecondDerivative(_f, x_0);
    while (std::abs(der) > 0.00001){
        x_0 = x_0 - der/sDer;
//        func = _f(x_0);
        der = Derivative(_f, x_0);
        sDer = SecondDerivative(_f, x_0);
    }
//    std::cout << "X = " << x_0 << " Func = " << _f(x_0) << " Deriv = " << der << std::endl;
    return x_0;
}

double InteractionFunction::InteractDipole(double dist) {
    if ((_lhsAngle + _rhsAngle) >= _maxLim) {
        _lhsAngle = M_PI - _lhsAngle;
        _rhsAngle = M_PI - _rhsAngle;
    }
    else if ((_lhsAngle + _rhsAngle) >= _minLim) {
        if ((_rhsAngle > (_lhsAngle + _minLim)) || (_rhsAngle < (_lhsAngle - _minLim))) {
            return 2 * _interactionEnergy;
        }
        else {
            auto cTheta = _minLim;
            auto cLhsAngle = 0.5 * (_rhsAngle - _lhsAngle + cTheta);
            auto cRhsAngle = 0.5 * (_lhsAngle - _rhsAngle + cTheta);

            _lhsAngle = cLhsAngle;
            _rhsAngle = cRhsAngle;
        }
    }

    return _interactionEnergy *(cos(std::abs(_lhsAngle - _rhsAngle) / _divider) - 3 * cos(_lhsAngle / _divider) * cos(_rhsAngle / _divider)) /
           (dist * dist * dist);
}

double InteractionFunction::InteractYukawa(double dist) {
    return _interactionEnergy*_YukawaA*exp(-dist*_YukawaK)/dist;
}

double InteractionFunction::operator()(double dist){
    return InteractDipole(dist)+InteractYukawa(dist);
}

MinMaxZeros InteractionFunction::GetMinMaxZero() {
    double x = 0.001;
    double fPrev = (*this)(x);
    auto dPrev = Derivative(*this, x);
//    auto sdPrev = SecondDerivative(*this, x);
//    auto div = dPrev * sdPrev;
    double delta = 0.001;
    std::vector<double> min_max_points;
    std::vector<double> zero_points;
    while (x < 3) {
        x += delta;
        if (Derivative(*this, x) * dPrev < 0) {//or SecondDerivative(functor, x)*sdPrev < 0) {
            min_max_points.push_back(x);
        }

        if (fPrev * (*this)(x) < 0) {
//          std::cout << "X = " << x - delta << " Func = " << functor(x-delta) << " Deriv = " << dPrev << " SecondDeriv = " <<
//          sdPrev << std::endl;
//          std::cout << "X = " << x << " Func = " << functor(x) << " Deriv = " << Derivative(functor, x) << " SecondDeriv = " << SecondDerivative(functor, x) << std::endl << std::endl;
            zero_points.push_back(x);
        }
        fPrev = (*this)(x);
        dPrev = Derivative(*this, x);
//        sdPrev = SecondDerivative(*this, x);
//        div = dPrev * sdPrev;
    }

    MinMaxZeros ret;
    for (size_t i = 0; i < min_max_points.size(); i++) {
        min_max_points[i] = FindMinimum(*this, min_max_points[i]);
        if (i == 0)
            ret.Max = min_max_points[i];
        if (i == 1)
            ret.Min = min_max_points[i];
    }

    for (size_t i = 0; i < zero_points.size(); i++) {
        zero_points[i] = FindZero(*this, zero_points[i]);
        ret.Zeros[i] = zero_points[i];
    }

    return ret;
};


