//
// Created by mpolovyi on 25/01/16.
//

#ifndef PROJECT_CMONTECARLOSIMPARAMS_H
#define PROJECT_CMONTECARLOSIMPARAMS_H

#include <SCOLSS/MathLibrary/FunctionOptimization.h>
#include "CBaseSimParams.h"

class CMonteCarloSimParams : public CBaseSimParams {
public:
    uint64_t EquilibriumCycle;

    double MaxTestDisplacement;

    template<class Archive>
    virtual void DoSerialize(Archive &archive) const {
        CBaseSimParams::DoSerialize(archive);
        archive(cereal::make_nvp("EquilibriumCycle", EquilibriumCycle));
    }

    template<class Archive>
    virtual void DoDeSerialize(Archive &archive) {
        CBaseSimParams::DoDeSerialize(archive);

        archive(cereal::make_nvp("EquilibriumCycle", EquilibriumCycle));

        auto Interaction = InteractionFunction(1/KbT, YukawaA, YukawaK, 90);
        auto min_max_zero = Interaction.GetMinMaxZero();

        ParticleDiameter = min_max_zero.Min;

        SystemSize = PtCount * ParticleDiameter / Density;

        MaxTestDisplacement = 0.5 * (min_max_zero.Min - min_max_zero.Zeros[1]);
    }

};


#endif //PROJECT_CMONTECARLOSIMPARAMS_H
