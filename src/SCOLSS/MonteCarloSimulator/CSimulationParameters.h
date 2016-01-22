//
// Created by mpolovyi on 04/09/15.
//
#ifndef COLLOIDMC_DATASTRUCTURE_H
#define COLLOIDMC_DATASTRUCTURE_H

#include <SCOLSS/MathLibrary/FunctionOptimization.h>

class CSimulationParameters {
public:
    int NumberOfSavePoints;
    int NumberOfImageLines;
    size_t PtCount;

    uint64_t EquilibriumCycle;

    double Density;
    double KbT;
    double MaxTestDisplacement;

    double YukawaA;
    double YukawaK;

    std::vector<CParticle> savedStateToLoad;

    double ParticleDiameter;
    double SystemSize;

    template<class Archive>
    void save(Archive & archive) const {
        archive(cereal::make_nvp("Density", Density));
        archive(cereal::make_nvp("EquilibriumCycle", EquilibriumCycle));
        archive(cereal::make_nvp("NumberOfSavePoints", NumberOfSavePoints));
        archive(cereal::make_nvp("KbT", KbT));
        archive(cereal::make_nvp("PtCount", PtCount));
    }

    template<class Archive>
    void load(Archive & archive)
    {
        archive(cereal::make_nvp("Density", Density));
        archive(cereal::make_nvp("EquilibriumCycle", EquilibriumCycle));
        archive(cereal::make_nvp("NumberOfSavePoints", NumberOfSavePoints));
        archive(cereal::make_nvp("KbT", KbT));
        archive(cereal::make_nvp("PtCount", PtCount));
        archive(cereal::make_nvp("NumberOfImageLines", NumberOfImageLines));

        int loadSaved;
        archive(cereal::make_nvp("LoadSavedState", loadSaved));

        if (loadSaved == 1) {
            savedStateToLoad.resize(PtCount);

            archive.loadBinaryValue(&savedStateToLoad[0], sizeof(CParticle) * PtCount, "SavedParticles");
        }

        YukawaA = 1000;
        YukawaK = 10;

        auto Interaction = InteractionFunction(1/KbT, YukawaA, YukawaK, 90);
        auto min_max_zero = Interaction.GetMinMaxZero();

        ParticleDiameter = min_max_zero.Min;

        SystemSize = PtCount * ParticleDiameter / Density;

        MaxTestDisplacement = 0.5 * (min_max_zero.Min - min_max_zero.Zeros[1]);
    }

    float GetEpsDimensionX(){
        return (float) SystemSize;
    }

    float GetEpsDimensionY(){
        return (float) (ParticleDiameter * NumberOfImageLines);
    }
};

#endif //COLLOIDMC_DATASTRUCTURE_H
