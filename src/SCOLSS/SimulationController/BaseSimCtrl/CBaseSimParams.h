//
// Created by mpolovyi on 25/01/16.
//

#ifndef PROJECT_CBASESIMPARAMS_H
#define PROJECT_CBASESIMPARAMS_H

#include <cereal/cereal.hpp>
#include <cmath>
#include <cstdlib>
#include <SCOLSS/ParticlePhysics/CParticleBase.h>
#include <SCOLSS/MathLibrary/FunctionOptimization.h>

enum ESimulationType{
    MonteCarlo = 0,
    LangevinDynamics = 1
};

enum EInitialConfiguration {
    Random = 0,
    Aligned = 1,
    AlignedTwoSides = 2,
    AlingnedUnmoving = 3,
    RandomUnmoving = 4,
    OneCluster = 5,
    First = Random,
    Last = OneCluster
};

class CBaseSimParams {
public:
    EInitialConfiguration InitialConfiguration;
    size_t PtCount;

    double KbT;
    double Density;

    size_t CyclesBetweenSaves;
    size_t NumberOfSavePoints;
    size_t NumberOfImageLines;

    double ParticleDiameter;
    double Radius;

    double ParticleMass;
    double Inertia;

    double SystemSize;

    double YukawaA;
    double YukawaK;

    bool SaveParticlesInfo;
    bool SaveEpsPicture;

    std::vector<CParticleBase> savedStateToLoad;

    template<class Archive>
    void save(Archive &archive) const {
        archive(cereal::make_nvp("Density", Density));
        archive(cereal::make_nvp("KbT", KbT));
        archive(cereal::make_nvp("PtCount", PtCount));
        archive(cereal::make_nvp("InitialConfiguration", InitialConfiguration));
    }

    template<class Archive>
    void load(Archive &archive) {
        YukawaA = 1000;
        YukawaK = 10;

        ParticleMass = 1;

        archive(cereal::make_nvp("Density", Density));

        archive(cereal::make_nvp("NumberOfSavePoints", NumberOfSavePoints));
        archive(cereal::make_nvp("KbT", KbT));
        archive(cereal::make_nvp("PtCount", PtCount));
        archive(cereal::make_nvp("NumberOfImageLines", NumberOfImageLines));

        archive(cereal::make_nvp("SaveParticlesInfo", SaveParticlesInfo));
        archive(cereal::make_nvp("SaveEpsPicture", SaveEpsPicture));

        int initialConfiguration;
        archive(cereal::make_nvp("InitialConfiguration", initialConfiguration));

        if (initialConfiguration <= EInitialConfiguration::Last){
            InitialConfiguration = (EInitialConfiguration) initialConfiguration;
        }
        else {
            InitialConfiguration = EInitialConfiguration::Random;
        }

        auto Interaction = InteractionFunction(1/KbT, YukawaA, YukawaK, 90);
        auto min_max_zero = Interaction.GetMinMaxZero();

        ParticleDiameter = min_max_zero.Min;

        SystemSize = PtCount * ParticleDiameter / Density;

        Radius = ParticleDiameter/2.0;

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
        uint64_t totalCycles = CyclesBetweenSaves * NumberOfSavePoints;
        return std::min(ParticleDiameter * totalCycles, ParticleDiameter * NumberOfImageLines);
    }

protected:
    double GetInertiaSpherical() const {
        return 2 * ParticleMass * Radius * Radius / 5.0;
    }
};


#endif //PROJECT_CBASESIMPARAMS_H
