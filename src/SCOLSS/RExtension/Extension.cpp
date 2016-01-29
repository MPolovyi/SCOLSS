//
// Created by mpolovyi on 28/01/16.
//

#include "ExtensionDefs.h"//
// Created by mpolovyi on 28/01/16.
//

#include <iostream>
#include <cmath>
#include "math.h"

#include <SCOLSS/ParticlePhysics/CParticleBase.h>

#include "cereal/archives/json.hpp"
#include "cereal/archives/binary.hpp"
#include "cereal/types/vector.hpp"

int get_next(int pt, int ptCount, int step) {
    pt += step;
    while (pt >= ptCount)
        pt -= ptCount;

    return pt;
}

int get_nearest_index(double* arr, double val, int size){
    int ret = 0;
    double dst_old = 100000;
    for(int i = 0; i < size; i++){
        double dst = std::abs(val - arr[i]);
        if (dst < dst_old) {
            dst_old = dst;
            ret = i;
        }
    }
    return ret;
}

extern "C" void Function_GetCorrelations(char ** input_string,
                                         int*_ptCount,
                                         int* _corrCount,
                                         double* _maxCorrLength,
                                         double* correlations_out,
                                         int* corr_counts_out,
                                         double* corr_lengths_out,
                                         double* _systemSize) {
    int ptCount = _ptCount[0];
    double& maxCorrLength = _maxCorrLength[0];
    int& corrCount = _corrCount[0];
    double& systemSize = _systemSize[0];

    std::string input(*input_string);
    std::stringstream in_stream;

    in_stream << "{\"value0\": \"";
    in_stream << input;
    in_stream << "\"}";
    cereal::JSONInputArchive arch(in_stream);
    std::vector<CParticleBase> particles;

    for (int i = 0; i < ptCount; ++i) {
        particles.push_back(CParticleBase());
    }
    arch.loadBinaryValue(&particles[0], sizeof(CParticleBase) * ptCount);

    double dist = 0;
    for (int i = 0; i < ptCount; i++) {
        auto& pt = particles[i];

        int j = get_next(i, ptCount);

        auto cosTheta1 = pt.GetOrientation().Z;

        correlations_out[0] += cosTheta1*cosTheta1;
        corr_counts_out[0]++;
        while (j != i) {
            const auto& pt_next = particles[j];

            dist = pt.GetDistanceRight(pt_next, systemSize).GetLength();
            if(dist < maxCorrLength) {
                auto nIndex = get_nearest_index(corr_lengths_out, dist, corrCount);

                auto cosTheta2 = pt_next.GetOrientation().Z;

                correlations_out[nIndex] += cosTheta1 * cosTheta2;
                corr_counts_out[nIndex]++;

                j = get_next(j, ptCount);
            }
            else{
                break;
            }
        }
    }
}

class CWrongQuaternion {
public:
    CVector V;
    double W;
};
class CWrongPtBase {
public:
    double Coordinates;
    CWrongQuaternion Rotation;
};

extern "C" int Function_ChangeBinaryToBaseParticles(void * input_string, void * output_string, int ptCount) {
    std::string input((char*)input_string);
    std::stringstream in_stream;

    in_stream << "{\"value0\": \"";
    in_stream << input;
    in_stream << "\"}";

//    std::cout << in_stream.str() << std::endl;

    cereal::JSONInputArchive arch(in_stream);
    std::vector<CWrongPtBase> particles;
    for (int i = 0; i < ptCount; ++i) {
        particles.push_back(CWrongPtBase());
    }
    arch.loadBinaryValue(&particles[0], sizeof(CWrongPtBase) * ptCount);

    std::vector<CParticleBase> saveParticles;
    for (int i = 0; i < ptCount; ++i){
        auto pt = CParticleBase();
        pt.Coordinates = particles[i].Coordinates;
        CQuaternion newOrient;
        newOrient.W = particles[i].Rotation.W;
        newOrient.V = particles[i].Rotation.V;
        pt.SetRotation(CQuaternion(newOrient));
        saveParticles.push_back(pt);
    }

    std::stringstream out_stream;

    cereal::JSONOutputArchive oarch(out_stream);
    oarch.saveBinaryValue(&saveParticles[0], sizeof(CParticleBase)*ptCount);

    std::string out_string = out_stream.str();
    for(int i = 17; i < out_string.length()-1; i++) {
        ((char*)output_string)[i-17] = out_string[i];
    }

    return strlen((char*)output_string);
}

extern "C" void Function_GetParticleOrientationProbability(char ** input_string,
                                                           int*_ptCount,
                                                           int* corr_counts_out) {
    int ptCount = _ptCount[0];

    std::string input(*input_string);
    std::stringstream in_stream;

    in_stream << "{\"value0\": \"";
    in_stream << input;
    in_stream << "\"}";
    cereal::JSONInputArchive arch(in_stream);
    std::vector<CParticleBase> particles;

    for (int i = 0; i < ptCount; ++i) {
        particles.push_back(CParticleBase());
    }
    arch.loadBinaryValue(&particles[0], sizeof(CParticleBase) * ptCount);

    double dist = 0;
    for (int i = 0; i < ptCount; i++) {
        auto& pt = particles[i];

        auto cosTheta1 = pt.GetOrientation().Z;

        if(cosTheta1 > 0) {
            corr_counts_out[1]++;
        }
        else {
            corr_counts_out[0]++;
        }
    }
}

extern "C" void Function_GetChainOrientationProbability(char ** input_string,
                                                        int*_ptCount,
                                                        double*  _separationCutOff,
                                                        int* corr_counts_out,
                                                        double* _systemSize) {
    int ptCount = _ptCount[0];
    double separationCutOff = _separationCutOff[0];
    double systemSize = _systemSize[0];

    std::string input(*input_string);
    std::stringstream in_stream;

    in_stream << "{\"value0\": \"";
    in_stream << input;
    in_stream << "\"}";
    cereal::JSONInputArchive arch(in_stream);
    std::vector<CParticleBase> particles;

    for (int i = 0; i < ptCount; ++i) {
        particles.push_back(CParticleBase());
    }
    arch.loadBinaryValue(&particles[0], sizeof(CParticleBase) * ptCount);

    bool chain_1;
    bool chain_2;

    int chainLength = 0;

    for (int i = 0; i < particles.size(); i++) {
        auto& pt = particles[i];
        chain_1 = pt.GetOrientation().Z > 0;
        auto& pt_next = particles[get_next(i, ptCount)];

        auto cosTheta = pt.GetOrientation().Z;
        auto cosTheta_next = pt_next.GetOrientation().Z;

        chain_2 = cosTheta_next > 0;

        if(pt.GetDistanceRight(pt_next, systemSize).GetLength() <= separationCutOff && cosTheta * cosTheta_next >= 0){
            chainLength++;
        }
        else {
            // rr, lr, rl, ll
            if (chain_1 && chain_2) corr_counts_out[0]++;
            if (!chain_1 && chain_2) corr_counts_out[1]++;
            if (chain_1 && !chain_2) corr_counts_out[2]++;
            if (!chain_1 && !chain_2) corr_counts_out[3]++;

            if (chain_1) {
                corr_counts_out[4]++;
            }
            else {
                corr_counts_out[5]++;
            }

            corr_counts_out[6] += chainLength;
            chainLength = 0;
        }
    }
}

extern "C" void Function_GetChainOrientationProbabilityTest(char ** input_string,
                                                            int*_ptCount,
                                                            double*  _separationCutOff,
                                                            int* corr_counts_out,
                                                            double* _systemSize) {
    int ptCount = _ptCount[0];
    double separationCutOff = _separationCutOff[0];
    double systemSize = _systemSize[0];

    std::string input(*input_string);
    std::stringstream in_stream;

    in_stream << "{\"value0\": \"";
    in_stream << input;
    in_stream << "\"}";
    cereal::JSONInputArchive arch(in_stream);
    std::vector<CParticleBase> particles;

    for (int i = 0; i < ptCount; ++i) {
        particles.push_back(CParticleBase());
    }
    arch.loadBinaryValue(&particles[0], sizeof(CParticleBase) * ptCount);

    particles = std::vector<CParticleBase>();

    double displ = 0;
    for(int i = 0; i < 10; i++){
        auto pt = CParticleBase();
        pt.Coordinates = i*0.9 + displ;

        particles.push_back(pt);
    }

//    displ = 12;
//    for(int i = 0; i < 10; i++){
//        auto pt = CParticleBase();
//        pt.Coordinates = i*0.9 + displ;
//
//        particles.push_back(pt);
//    }
//
//    displ = 24;
//    for(int i = 0; i < 10; i++){
//        auto pt = CParticleBase();
//        pt.Coordinates = i*0.9 + displ;
//        pt.Rotation = CQuaternion(M_PI, CVector(0, 1, 0));
//        particles.push_back(pt);
//    }
//
//
    displ = 36;
    for(int i = 0; i < 10; i++){
        auto pt = CParticleBase();
        pt.Coordinates = i*0.9 + displ;
        pt.SetRotation(CQuaternion(M_PI, CVector(0, 1, 0)));
        particles.push_back(pt);
    }

    displ = 48;
    for(int i = 0; i < 10; i++){
        auto pt = CParticleBase();
        pt.Coordinates = i*0.9 + displ;
        pt.SetRotation(CQuaternion(M_PI, CVector(0, 1, 0)));
        particles.push_back(pt);
    }


    bool chain_1 = particles[0].GetOrientation().Z > 0;
    bool chain_2 = true;

    int chainLength = 0;

    for (int i = 0; i < particles.size(); i++) {
        auto& pt = particles[i];
        auto& pt_next = particles[get_next(i, ptCount)];

        auto cosTheta = pt.GetOrientation().Z;
        auto cosTheta_next = pt_next.GetOrientation().Z;

        chain_2 = cosTheta_next > 0;

        if(pt.GetDistanceRight(pt_next, systemSize).GetLength() <= separationCutOff && cosTheta * cosTheta_next >= 0){
            chainLength++;
        }
        else {
            // ll, rl, lr, rr
            if (chain_1 && chain_2) corr_counts_out[3]++;
            if (!chain_1 && chain_2) corr_counts_out[2]++;
            if (chain_1 && !chain_2) corr_counts_out[1]++;
            if (!chain_1 && !chain_2) corr_counts_out[0]++;

            if (chain_1) {
                corr_counts_out[4]++;
            }
            else {
                corr_counts_out[5]++;
            }

            corr_counts_out[6] += chainLength;
            chainLength = 0;

            chain_1 = chain_2;
        }
    }
}

extern "C" void Function_UnwrapBinary(char ** input_string, int* in_size, double * ret) {
    int size = in_size[0];

    std::string input(*input_string);
    std::stringstream in_stream;

    in_stream << "{\"value0\": \"";
    in_stream << input;
    in_stream << "\"}";

    cereal::JSONInputArchive arch(in_stream);

    arch.loadBinaryValue(ret, sizeof(double) * size);
}