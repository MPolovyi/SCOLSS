//
// Created by mpolovyi on 04/12/15.
//

//#define NON_INTERACTING

#ifndef COLLOIDMD_SIMULATIONDATA_H_H
#define COLLOIDMD_SIMULATIONDATA_H_H


#include <cereal/cereal.hpp>
enum EInitialConfiguration{
    Aligned = 1,
    Random = 0,
    AlignedTwoSides = 2
};

class CSimulationParameters {
public:

    EInitialConfiguration InitialConfiguration;

    unsigned int PtCount;

    double KbT;
    double Density;

    unsigned int CyclesBetweenSaves;
    unsigned int NumberOfSavePoints;
    unsigned int NumberOfImageLines;

    double TimeStep;

    double Viscosity;
    double ParticleDiameter;
    double Radius;

    double ParticleMass;

    double Inertia;
    double TranslationalDampingTime;

    double AlphaTranslational;
    double AlphaRotational;

    double DistributionDeviationTranslation;
    double VerletCoefficientTranslation;

    double DistributionDeviationRotation;
    double VerletCoefficientRotation;

    double SystemSize;

    double YukawaA;
    double YukawaK;

    std::vector<CYukawaDipolePt> savedStateToLoad;

    template<class Archive>
    void save(Archive &archive) const {
        archive(cereal::make_nvp("Density", Density));
        archive(cereal::make_nvp("KbT", KbT));
        archive(cereal::make_nvp("PtCount", PtCount));
        archive(cereal::make_nvp("TimeStep", TimeStep));
        archive(cereal::make_nvp("InitialConfiguration", InitialConfiguration));
    }

    template<class Archive>
    void load(Archive &archive) {
        TimeStep = 0.01;

        YukawaA = 1000;
        YukawaK = 10;

        Viscosity = 1;
        ParticleMass = 1;

        archive(cereal::make_nvp("Density", Density));

        double timeBetweenSaves = 0;
        archive(cereal::make_nvp("TimeBetweenSaves", timeBetweenSaves));
        CyclesBetweenSaves = (unsigned int) std::ceil(timeBetweenSaves / TimeStep);
        archive(cereal::make_nvp("NumberOfSavePoints", NumberOfSavePoints));
        archive(cereal::make_nvp("KbT", KbT));
        archive(cereal::make_nvp("PtCount", PtCount));
        archive(cereal::make_nvp("NumberOfImageLines", NumberOfImageLines));

        int initialConfiguration;
        archive(cereal::make_nvp("InitialConfiguration", initialConfiguration));

        if (initialConfiguration <= 2){
            InitialConfiguration = (EInitialConfiguration) initialConfiguration;
        }
        else {
            InitialConfiguration = (EInitialConfiguration) 0;
        }

#ifdef NON_INTERACTING
        ParticleDiameter = 2;
        Radius = ParticleDiameter/2.0;

        TranslationalDampingTime = 0.1;
#else
            ParticleDiameter = 0.607592984;
            Radius = ParticleDiameter/2.0;

            TranslationalDampingTime = GetTranslationalDampingTime();
#endif

        SystemSize = PtCount * ParticleDiameter / Density;
        Inertia = GetInertia();
        AlphaTranslational = GetAlphaTranslational();
        AlphaRotational = GetAlphaRotational();
        DistributionDeviationTranslation = GetDistributionDeviationTranslation();
        VerletCoefficientTranslation = GetVerletCoefficientTranslation();
        DistributionDeviationRotation = GetDistributionDeviationRotation();
        VerletCoefficientRotation = GetVerletCoefficientRotation();

        int loadSaved;
        archive(cereal::make_nvp("LoadSavedState", loadSaved));

        if (loadSaved == 1) {
            savedStateToLoad.resize(PtCount);

            archive.loadBinaryValue(&savedStateToLoad[0], sizeof(CYukawaDipolePt) * PtCount, "SavedParticles");
        }
    }

    float GetEpsDimensionX(){
        return (float) SystemSize;
    }

    float GetEpsDimensionY(){
        return (float) (ParticleDiameter * NumberOfImageLines);
    }
private:
    double GetInertia() const {
        return 2 * ParticleMass * Radius * Radius / 5.0;
    }

    double GetTranslationalDampingTime() const {
        return ParticleMass / (6 * M_PI * Viscosity * Radius);
    }

    double GetAlphaTranslational() const {
        return ParticleMass / TranslationalDampingTime;
    }

    double GetAlphaRotational() const {
        return 10 * GetInertia() / (3 * TranslationalDampingTime);
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

#endif //COLLOIDMD_SIMULATIONDATA_H_H
