//
// Created by mpolovyi on 25/01/16.
//

#ifndef PROJECT_CBASESIMPARAMS_H
#define PROJECT_CBASESIMPARAMS_H

#include <cereal/cereal.hpp>
#include <SCOLSS/ParticlePhysics/CParticleBase.h>
#include <cmath>

enum EInitialConfiguration {
    Random = 0,
    Aligned = 1,
    AlignedTwoSides = 2,
    AlingnedUnmoving = 3,
    RandomUnmoving = 4
};

class CBaseSimParams {
public:
    EInitialConfiguration InitialConfiguration;

    unsigned int PtCount;

    double KbT;
    double Density;

    unsigned int CyclesBetweenSaves;
    unsigned int NumberOfSavePoints;
    unsigned int NumberOfImageLines;

    double ParticleDiameter;
    double Radius;

    double ParticleMass;
    double Inertia;

    double SystemSize;

    double YukawaA;
    double YukawaK;

    std::vector<CParticleBase> savedStateToLoad;

    template<class Archive>
    void save(Archive &archive) const {
        DoSerialize(archive);
    }

    template<class Archive>
    virtual void DoSerialize(Archive &archive) const {
        archive(cereal::make_nvp("Density", Density));
        archive(cereal::make_nvp("KbT", KbT));
        archive(cereal::make_nvp("PtCount", PtCount));
        archive(cereal::make_nvp("InitialConfiguration", InitialConfiguration));
    }

    template<class Archive>
    void load(Archive &archive) {
        DoDeSerialize(archive);
    }

    template<class Archive>
    virtual void DoDeSerialize(Archive &archive) {
        YukawaA = 1000;
        YukawaK = 10;

        ParticleMass = 1;

        archive(cereal::make_nvp("Density", Density));

        archive(cereal::make_nvp("NumberOfSavePoints", NumberOfSavePoints));
        archive(cereal::make_nvp("KbT", KbT));
        archive(cereal::make_nvp("PtCount", PtCount));
        archive(cereal::make_nvp("NumberOfImageLines", NumberOfImageLines));

        int initialConfiguration;
        archive(cereal::make_nvp("InitialConfiguration", initialConfiguration));

        if (initialConfiguration <= 4){
            InitialConfiguration = (EInitialConfiguration) initialConfiguration;
        }
        else {
            InitialConfiguration = (EInitialConfiguration) 0;
        }

        ParticleDiameter = 0.607592984;
        Radius = ParticleDiameter/2.0;

        SystemSize = PtCount * ParticleDiameter / Density;
        Inertia = GetInertiaSpherical();

        int loadSaved;
        archive(cereal::make_nvp("LoadSavedState", loadSaved));

        if (loadSaved == 1) {
            savedStateToLoad.resize(PtCount);

            archive.loadBinaryValue(&savedStateToLoad[0], sizeof(CParticleBase) * PtCount, "SavedParticles");
        }
    }

    double GetEpsDimensionX() {
        return SystemSize;
    }

    double GetEpsDimensionY() {
        return ParticleDiameter * NumberOfImageLines;
    }

protected:
    double GetInertiaSpherical() const {
        return 2 * ParticleMass * Radius * Radius / 5.0;
    }
};


#endif //PROJECT_CBASESIMPARAMS_H
