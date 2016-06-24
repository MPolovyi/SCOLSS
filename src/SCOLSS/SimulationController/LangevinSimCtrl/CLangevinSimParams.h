//
// Created by mpolovyi on 25/01/16.
//

#ifndef PROJECT_CLANGEVINSIMPARAMS_H
#define PROJECT_CLANGEVINSIMPARAMS_H

#include <cstdlib>
#include <SCOLSS/ParticlePhysics/CYukawaDipolePt.h>
#include <SCOLSS/SimulationController/BaseSimCtrl/CBaseSimParams.h>

class CLangevinSimParams : public CBaseSimParams {
public:
    double TimeStep;

    double TranslationalDampingTime;
    double Viscosity;

    double AlphaTranslational;
    double AlphaRotational;

    double DistributionDeviationTranslation;
    double VerletCoefficientTranslation;

    double DistributionDeviationRotation;
    double VerletCoefficientRotation;

    template<class Archive>
    void save(Archive &archive) const {
        archive(cereal::make_nvp("Base", cereal::base_class<CBaseSimParams>(this)));
//        archive(cereal::make_nvp("TimeStep", TimeStep));
    }

    template<class Archive>
    void load(Archive &archive) {
        archive(cereal::make_nvp("Base", cereal::base_class<CBaseSimParams>(this)));

        TimeStep = 0.01;
        Viscosity = 1;

        double timeBetweenSaves = 0;
        archive(cereal::make_nvp("TimeBetweenSaves", timeBetweenSaves));
        CyclesBetweenSaves = (unsigned int) std::ceil(timeBetweenSaves / TimeStep);

        TranslationalDampingTime = GetTranslationalDampingTime();

        AlphaTranslational = GetAlphaTranslational();
        AlphaRotational = GetAlphaRotational();
        DistributionDeviationTranslation = GetDistributionDeviationTranslation();
        VerletCoefficientTranslation = GetVerletCoefficientTranslation();
        DistributionDeviationRotation = GetDistributionDeviationRotation();
        VerletCoefficientRotation = GetVerletCoefficientRotation();
    }

protected:
    double GetTranslationalDampingTime() const {
        return ParticleMass / (6 * M_PI * Viscosity * Radius);
    }

    double GetAlphaTranslational() const {
        return ParticleMass / TranslationalDampingTime;
    }

    double GetAlphaRotational() const {
        return 10 * GetInertiaSpherical() / (3 * TranslationalDampingTime);
    }

    double GetDistributionDeviationTranslation() const {
        return sqrt(2 * GetAlphaTranslational() * KbT * TimeStep);
    }

// b
    double GetVerletCoefficientTranslation() const {
        return 1 / (1 + (GetAlphaTranslational() * TimeStep / 2 / ParticleMass));
    }

    double GetDistributionDeviationRotation() const {
        return sqrt(2 * GetAlphaRotational() * KbT * TimeStep);
    }

    double GetVerletCoefficientRotation() const {
        return 1 / (1 + (GetAlphaRotational() * TimeStep / 2 / Inertia));
    }
};

#endif //PROJECT_CLANGEVINSIMPARAMS_H
