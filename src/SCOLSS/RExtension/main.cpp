#include <iostream>
#include <dlfcn.h>
#include <string.h>
#include <cmath>

#include "PtInput.h"

#include "ExtensionDefs.h"

int main() {
    void* handle = dlopen("../build/lib/libRExtensionLibrary.so", RTLD_NOW);

    int ptCount = 1600;
    double cut_off = std::cos(M_PI_4);
    int corr_counts[10];
    double corr_lengths[10];

    for(int i = 0; i < 10; i++){
        corr_lengths[i] = i;
    }

    double neigh_c[5000];
    int coords[5000];
    int chained[5000];
    double systemSize = 6000;

    int ptStrLength = 320000;
    int timePointsCount = 3;

    Function_GetChainOrientationProbabilityAngle(chained, coords, &encoded, &ptCount, &cut_off, &systemSize);
    std::cout << corr_counts[0] << " " << corr_counts[1] << " " << corr_counts[2] << " " << corr_counts[3] << std::endl;
//    Function_GetDynamicChains(neigh_c, coords, chained, &encoded, &ptStrLength, &timePointsCount, &ptCount, &systemSize);

    return 0;
}
