//
// Created by mpolovyi on 25/01/16.
//

#ifndef PROJECT_CMONTECARLOSIMPARAMS_H
#define PROJECT_CMONTECARLOSIMPARAMS_H

#include <cstdlib>
#include <SCOLSS/MathLibrary/FunctionOptimization.h>
#include <SCOLSS/SimulationController/BaseSimCtrl/CBaseSimParams.h>

class CMonteCarloSimParams : public CBaseSimParams {
public:

    double MaxTestDisplacement;

    template<class Archive>
    void save(Archive &archive) const {
        archive(cereal::make_nvp("Base", cereal::base_class<CBaseSimParams>(this)));
        archive(cereal::make_nvp("CyclesBetweenSaves", CyclesBetweenSaves));
    }

    template<class Archive>
    void load(Archive &archive) {
        archive(cereal::make_nvp("Base", cereal::base_class<CBaseSimParams>(this)));
        archive(cereal::make_nvp("CyclesBetweenSaves", CyclesBetweenSaves));

        auto Interaction = InteractionFunction(1/KbT, YukawaA, YukawaK, 90);
        auto min_max_zero = Interaction.GetMinMaxZero();

        MaxTestDisplacement = 0.5 * (min_max_zero.Min - min_max_zero.Zeros[1]);
    }

};


#endif //PROJECT_CMONTECARLOSIMPARAMS_H
