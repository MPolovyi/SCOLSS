//
// Created by mpolovyi on 28/01/16.
//

#ifndef PROJECT_EXTENSIONDEFS_H
#define PROJECT_EXTENSIONDEFS_H

int get_next(int pt, int ptCount, int step = 1);

constexpr double GetRadius() {
    return 0.607592984 / 2.0;
}

constexpr double GetInertia() {
    return 2 * 1 * GetRadius() * GetRadius() / 5.0;
}

int get_nearest_index(double* arr, double val, int size);

extern "C" void Function_GetCorrelations(char ** input_string,
                                         int*_ptCount,
                                         int* _corrCount,
                                         double* _maxCorrLength,
                                         double* correlations_out,
                                         int* corr_counts_out,
                                         double* corr_lengths_out,
                                         double* _systemSize);

extern "C" void Function_GetParticleOrientationProbability(char ** input_string,
                                                           int*_ptCount,
                                                           int* corr_counts_out);

extern "C" void Function_GetChainOrientationProbability(char ** input_string,
                                                        int*_ptCount,
                                                        double*  _separationCutOff,
                                                        int* corr_counts_out,
                                                        double* _systemSize);

extern "C" void Function_GetChainOrientationProbabilityTest(char ** input_string,
                                                            int*_ptCount,
                                                            double*  _separationCutOff,
                                                            int* corr_counts_out,
                                                            double* _systemSize);

extern "C" void Function_UnwrapBinary(char ** input_string, int* in_size, double * ret);

#endif //PROJECT_EXTENSIONDEFS_H
