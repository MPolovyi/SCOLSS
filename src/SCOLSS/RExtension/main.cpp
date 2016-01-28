#include <iostream>
#include <dlfcn.h>

#include "PtInput.h"

#include "ExtensionDefs.h"

int main() {
    void* handle = dlopen("../build/lib/libRExtensionLibrary.so", RTLD_LAZY);

    int size = 3200;

    double maxCorrLength = 20;
    int corrPointsCount = 20;

    double correlations[20];
    double corrLengths[20];
    int correlationCounts[20];

    for (int i = 0; i < 20; ++i) {
        correlations[i] = 0;
        correlationCounts[i] = 0;
        corrLengths[i] = i;
    }

    double systemSize = 3200;

//    Function_GetCorrelations(&encoded,
//                             &size,
//                             &corrPointsCount,
//                             &maxCorrLength,
//                             correlations,
//                             correlationCounts,
//                             corrLengths,
//                             &systemSize);

    for (int i = 0; i < 20; ++i) {
        correlations[i] = 0;
        correlationCounts[i] = 0;
        corrLengths[i] = i;
    }

    double rho = 0.25;
    double cutOff = 1;
    double minDst = systemSize;
    double maxDst = 0;
    double aver = 0;

    Function_GetChainOrientationProbability(&encoded, &size, &cutOff, correlationCounts, &minDst);
//    Function_GetChainOrientationProbabilityTest(&encoded, &size, &cutOff, correlationCounts, &minDst);

    return 0;
}
