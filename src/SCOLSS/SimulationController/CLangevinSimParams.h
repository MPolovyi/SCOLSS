//
// Created by mpolovyi on 25/01/16.
//

#ifndef PROJECT_CLANGEVINSIMPARAMS_H
#define PROJECT_CLANGEVINSIMPARAMS_H

#include <SCOLSS/ParticlePhysics/CYukawaDipolePt.h>
#include "CBaseSimParams.h"
#include <cstdlib>

class CLangevinSimParams : public CBaseSimParams {
public:
    unsigned int CyclesBetweenSaves;

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
    void DoSerialize(Archive &archive) const {
        CBaseSimParams::DoSerialize(archive);
        archive(cereal::make_nvp("TimeStep", TimeStep));
    }

    template<class Archive>
    void DoDeSerialize(Archive &archive) {
        CBaseSimParams::DoDeSerialize(archive);

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
