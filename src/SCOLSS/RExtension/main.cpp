#include <iostream>
#include <dlfcn.h>
#include <string.h>
#include <cmath>

#include <cereal/archives/json.hpp>
#include <fstream>
#include <sstream>

//#include "PtInput.h"

#include "ExtensionDefs.h"

class simParams{
public:
    simParams(){}

    simParams(int _ptCount, int _initC, double _dens, double _kbt) {
        ptCount = _ptCount;
        initC = _initC;
        dens = _dens;
        kbt = _kbt;
    }
    int ptCount;
    int initC;
    double dens;
    double kbt;

    template <typename archive>
    void serialize(archive& arch) {
        arch(cereal::make_nvp("Density", dens));
        arch(cereal::make_nvp("KbT", kbt));
        arch(cereal::make_nvp("PtCount", ptCount));
        arch(cereal::make_nvp("InitialConfiguration", initC));
    }
};

//class breaks{
//public:
//    breaks(int* _breaks) {
//        double summ = 0;
//
//        for (int i = 0; i < 9; ++i) {
//            summ += _breaks[i];
//        }
//        for (int j = 0; j < 9; ++j) {
//            probs[j] = _breaks[j]/summ;
//        }
//    }
//    double probs[10];
//
//    template <class archive>
//    void serialize(archive& arch) {
//        arch(cereal::make_nvp("LL", probs[0]));
//        arch(cereal::make_nvp("LR", probs[1]));
//        arch(cereal::make_nvp("LU", probs[2]));
//        arch(cereal::make_nvp("RL", probs[3]));
//        arch(cereal::make_nvp("RR", probs[4]));
//        arch(cereal::make_nvp("RU", probs[5]));
//        arch(cereal::make_nvp("UL", probs[6]));
//        arch(cereal::make_nvp("UR", probs[7]));
//        arch(cereal::make_nvp("UU", probs[8]));
//    }
//};

class calc_params{
public:
    template <class archive>
    void serialize(archive& arch) {

        std::string a = "Function_GetChainOrientationProbabilityAngle_sum";
        arch(cereal::make_nvp("Function", a));

        a = "cos(pi/3)";
        arch(cereal::make_nvp("AngleCut", a));

        a = "0.1";
        arch(cereal::make_nvp("DistCut", a));
    }
};
//
//class saver {
//public:
//    saver(simParams _params, int* _breaks, double _simTime):
//            simP(_params), Breaks(_breaks),  simTime(_simTime) {
//    }
//
//    simParams simP;
//    breaks Breaks;
//    calc_params CalcParams;
//    double simTime;
//
//    template <class archive>
//    void serialize(archive& arch) {
//        arch(cereal::make_nvp("SimulationParameters", simP));
//        arch(cereal::make_nvp("CalcParameters", CalcParams));
//        arch(cereal::make_nvp("Breaks", Breaks));
//        arch(cereal::make_nvp("SimulationTime", simTime));
//    }
//};


class saver_2 {
public:
    saver_2(simParams _params, double autocor, double _simTime):
    simP(_params), Autocorrelation(autocor),  simTime(_simTime) {
    }

    simParams simP;
    double Autocorrelation;
    calc_params CalcParams;
    double simTime;

    template <class archive>
    void serialize(archive& arch) {
        arch(cereal::make_nvp("SimulationParameters", simP));
        arch(cereal::make_nvp("CalcParameters", CalcParams));
        arch(cereal::make_nvp("Autocorrelation", Autocorrelation));
        arch(cereal::make_nvp("SimulationTime", simTime));
    }
};


std::string loadParticles(std::string fname, simParams* p) {
    double tmp_double;
    long tmp_int;
    std::string tmp_string;

    std::fstream saved_data(fname);
    cereal::JSONInputArchive iarch(saved_data);
    simParams params;

    iarch.startNode();
    iarch.loadValue(tmp_int);
    iarch.loadValue(tmp_string);
    iarch.startNode();
    iarch.loadValue(tmp_int);
    iarch.startNode();
    iarch.startNode();
    iarch(params);
    iarch.loadValue(tmp_double);
    iarch.loadValue(tmp_double);

    std::string particles;
    iarch.loadValue(particles);

    *p = params;
    return particles;
}

int main() {
//    void* handle = dlopen("../build/lib/libRExtensionLibrary.so", RTLD_NOW);
////    void *handle = dlopen("libRExtensionLibrary.so", RTLD_NOW);
//
    for (int i = 0; i < 10000; i += 20) {
        simParams p;

        double corrs[500];
        int chain_counts[10];
        for (int j = 0; j < 500; ++j) {
            corrs[j] = 0;
        }

        for (int sample = 1; sample <= 500; ++sample) {
            auto particles_0 = loadParticles("FullData_" + std::to_string(sample) + "_Data_0.json0", &p);
            auto particles = loadParticles("FullData_" + std::to_string(sample) + "_Data_0.json" + std::to_string(i), &p);
            double cut_off = 0.1;
            double angle_cut_off = std::cos(M_PI/3.0);

            char *encoded = &particles[0];
            char *encoded_0 = &particles_0[0];

            int sample_index = sample-1;
            Function_AutoCorrelation(corrs, &sample_index, &encoded_0, &encoded, &p.ptCount);

        }

        double res = 0;
        for (int k = 0; k < 500; k++){
            res += corrs[k];
        }

        std::fstream calculated_file("Autocor_data" + std::to_string(i), std::ios::out);
        {
            saver_2 to_save(p, res/500.0, i/100.0);

            cereal::JSONOutputArchive calc_data(calculated_file);

            calc_data(to_save);
        }
        calculated_file.close();
    }

//    int ptCount = 1600;
//    double cut_off = std::cos(M_PI_4);
//    int corr_counts[10];
//    double corr_lengths[10];
//
//    for (int i = 0; i < 10; i++) {
//        corr_lengths[i] = i;
//    }
//
//    double neigh_c[5000];
//    int coords[5000];
//    int chained[5000];
//    double systemSize = 6000;
//
//    int ptStrLength = 320000;
//    int timePointsCount = 3;
//
//    Function_GetChainOrientationProbabilityAngle(chained, coords, &encoded, &ptCount, &cut_off, &systemSize);
//    std::cout << corr_counts[0] << " " << corr_counts[1] << " " << corr_counts[2] << " " << corr_counts[3] << std::endl;
//    Function_GetDynamicChains(neigh_c, coords, chained, &encoded, &ptStrLength, &timePointsCount, &ptCount, &systemSize);

    return 0;
}
