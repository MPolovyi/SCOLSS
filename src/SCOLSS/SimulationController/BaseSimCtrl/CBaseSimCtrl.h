//
// Created by mpolovyi on 25/01/16.
//

#ifndef PROJECT_CBASESIMCTRL_H
#define PROJECT_CBASESIMCTRL_H

#include <vector>
#include <random>
#include <cstdlib>
#include <chrono>
#include <fstream>

#include <mpi.h>

#include <cereal/cereal.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/polymorphic.hpp>
#include <cereal/archives/json.hpp>
#include <cereal/archives/binary.hpp>
#include <cereal/types/tuple.hpp>

#include <SCOLSS/EPSPlot/EPSPlot.h>

#include <SCOLSS/ParticlePhysics/CParticleBase.h>
#include <SCOLSS/ParticlePhysics/CYukawaDipolePt.h>
#include <SCOLSS/SimulationController/BaseSimCtrl/CBaseSimCtrl.h>
#include <SCOLSS/SimulationController/BaseSimCtrl/CBaseSimParams.h>

#include <SCOLSS/SimulationController/CDataChunk.h>
#include <SCOLSS/RExtension/ExtensionDefs.h>


class CBaseSimCtrl {
private:
    class breaks{
    public:
        breaks(int* _breaks) {
            double summ = 0;

            for (int i = 0; i < 9; ++i) {
                summ += _breaks[i];
            }
            for (int j = 0; j < 9; ++j) {
                probs[j] = _breaks[j]/summ;
            }
        }
        double probs[10];

        template <class archive>
        void serialize(archive& arch) {
            arch(cereal::make_nvp("LL", probs[0]));
            arch(cereal::make_nvp("LR", probs[1]));
            arch(cereal::make_nvp("LU", probs[2]));
            arch(cereal::make_nvp("RL", probs[3]));
            arch(cereal::make_nvp("RR", probs[4]));
            arch(cereal::make_nvp("RU", probs[5]));
            arch(cereal::make_nvp("UL", probs[6]));
            arch(cereal::make_nvp("UR", probs[7]));
            arch(cereal::make_nvp("UU", probs[8]));
        }
    };

    double AutoCorrelation_CPP(std::vector<CParticleBase> zero_config,
                               std::vector<CParticleBase> current_config) const {
        double corr = 0;

        for (int i = 0; i < zero_config.size(); ++i) {
            corr += zero_config[i].GetOrientation().DotProduct(current_config[i].GetOrientation());
        }

        return corr;
    }

    bool IsLeft(double cosAngle, double cutOff) const {
        return cosAngle < -cutOff;
    }
    bool IsRight(double cosAngle, double cutOff) const {
        return cosAngle > cutOff;
    }
    bool IsUndefined(double cosAngle, double cutOff) const {
        return cosAngle < cutOff && cosAngle > -cutOff;
    };

    int MapOrientation(double cosThis, double cosNext, double cutOff) const {
        if(IsLeft(cosThis, cutOff) && IsLeft(cosNext, cutOff)){ return 0; }
        if(IsLeft(cosThis, cutOff) && IsRight(cosNext, cutOff)){ return 1; }
        if(IsLeft(cosThis, cutOff) && IsUndefined(cosNext, cutOff)){ return 2; }

        if(IsRight(cosThis, cutOff) && IsLeft(cosNext, cutOff)){ return 3; }
        if(IsRight(cosThis, cutOff) && IsRight(cosNext, cutOff)){ return 4; }
        if(IsRight(cosThis, cutOff) && IsUndefined(cosNext, cutOff)){ return 5; }

        if(IsUndefined(cosThis, cutOff) && IsLeft(cosNext, cutOff)){ return 6; }
        if(IsUndefined(cosThis, cutOff) && IsRight(cosNext, cutOff)){ return 7; }
        if(IsUndefined(cosThis, cutOff) && IsUndefined(cosNext, cutOff)){ return 8; }

        return 999;
    }

    breaks GetChainOrientationProbabilityAngle_CPP(std::vector<CParticleBase> particles,
                                                                            double angleCutOff,
                                                                            double distanceCutOff) const {

        int ptCount = (int) particles.size();

        int breaks_counts[10];

        int chainLength = 0;

        for (int i = 0; i < particles.size(); i++) {
            auto cosThis = particles[i].GetOrientation().Z;
            auto cosNext = particles[GetNext(i)].GetOrientation().Z;

            if ((MapOrientation(cosThis, cosNext, angleCutOff) == 0 || MapOrientation(cosThis, cosNext, angleCutOff) == 4)
                && particles[i].GetDistanceRight(particles[GetNext(i)], 10000).GetLength() < distanceCutOff) {
                chainLength++;
            } else {
                breaks_counts[MapOrientation(cosThis, cosNext, angleCutOff)]++;
            }
        }
        breaks brks(breaks_counts);

        return brks;
    }

    int get_nearest_index(std::vector<double> arr, double val, size_t size) const {
        int ret = 0;
        double dst_old = SimulationParameters.SystemSize;
        for(int i = 0; i < size; i++){
            double dst = std::abs(val - arr[i]);
            if (dst < dst_old) {
                dst_old = dst;
                ret = i;
            }
        }
        return ret;
    }

    std::pair<std::vector<double>, std::vector<double>> GetCorrelations_CPP(std::vector<double> corrs_calc_points,
                                            std::vector<CParticleBase> particles) const {
        size_t ptCount = SimulationParameters.PtCount;
        size_t corrCount = corrs_calc_points.size();
        const double& maxCorrLength = corrs_calc_points[corrCount-1];

        std::vector<double> correlations_out;
        correlations_out.resize(corrCount, 0);
        std::vector<double> corr_counts;
        corr_counts.resize(corrCount, 0);

        double dist = 0;
        for (size_t i = 0; i < ptCount; i++) {
            auto& pt = particles[i];

            size_t j = GetNext(i);

            auto cosTheta1 = pt.GetOrientation().Z;

            correlations_out[0] += cosTheta1*cosTheta1;
            corr_counts[0]++;
            while (j != i) {
                const auto& pt_next = particles[j];

                dist = pt.GetDistanceRight(pt_next, SimulationParameters.SystemSize).GetLength();
                if(dist < maxCorrLength) {
                    auto nIndex = get_nearest_index(corrs_calc_points, dist, corrCount);

                    auto cosTheta2 = pt_next.GetOrientation().Z;

                    correlations_out[nIndex] += cosTheta1 * cosTheta2;
                    corr_counts[nIndex]++;

                    j = GetNext(j);
                }
                else{
                    break;
                }
            }
        }

        for (int k = 0; k < corrCount; ++k) {
            correlations_out[k] /= ( corr_counts[k] != 0 ? corr_counts[k] : 1 );
        }

        return std::make_pair(corrs_calc_points, correlations_out);
    }

public:
    unsigned long Cycles;

    CBaseSimParams SimulationParameters;

    std::chrono::time_point<std::chrono::system_clock> initialize_time;

    unsigned long RndSeed;
    std::mt19937_64 rnd_gen;
    std::uniform_real_distribution<double> uniformDistributionZeroTwoPi;

    std::uniform_real_distribution<double> uniformDistributionZeroOne;

    std::uniform_real_distribution<double> initialDisplacementDistribution;

    template<class Archive>
    void save(Archive &archive) const {
        archive(cereal::make_nvp("SimulationParameters", SimulationParameters));
        archive(cereal::make_nvp("OrderParameter", GetOrderParameter()));

        archive(cereal::make_nvp("PotentialEnergy", GetAveragePotentialEnergy()));

        std::vector<CParticleBase> pts_save;
        for (size_t i = 0; i < SimulationParameters.PtCount; ++i) {
            pts_save.push_back(particles_old[i]);
        }

        {
            const breaks probs = GetChainOrientationProbabilityAngle_CPP(pts_save, cos(M_PI / 3), 0.1);
            archive.saveBinaryValue(&probs, sizeof(breaks), "ProbsData_01");
        }
        {
            const breaks probs = GetChainOrientationProbabilityAngle_CPP(pts_save, cos(M_PI / 3), 0.7);
            archive.saveBinaryValue(&probs, sizeof(breaks), "ProbsData_07");
        }
        {
            const breaks probs = GetChainOrientationProbabilityAngle_CPP(pts_save, cos(M_PI / 3), 1.4);
            archive.saveBinaryValue(&probs, sizeof(breaks), "ProbsData_14");
        }

        std::vector<double> corrs_calc_points;
        for (int j = 0; j < 50; ++j) {
            corrs_calc_points.push_back(j + SimulationParameters.ParticleDiameter);
        }

        auto length_correlations = GetCorrelations_CPP(corrs_calc_points, pts_save);

        const double * length_correlations_x = &length_correlations.first[0];
        archive.saveBinaryValue(length_correlations_x, sizeof(double)*50, "LengthCorrs_x");
        const double * length_correlations_y = &length_correlations.second[0];
        archive.saveBinaryValue(length_correlations_x, sizeof(double)*50, "LengthCorrs_y");

        archive(cereal::make_nvp("Autocorrelation", AutoCorrelation_CPP(particles_init, pts_save)));

        if (SimulationParameters.SaveParticlesInfo) {
            const CParticleBase *tmp = &pts_save[0];
            archive.saveBinaryValue(tmp, sizeof(CParticleBase) * particles_old.size(), "Particles");
        }
    };

    CBaseSimCtrl(CBaseSimParams d);

    virtual void InitRandomGenerator() {
        int currentId = MPI::COMM_WORLD.Get_rank();

        if (currentId == ManagerProcId) {
            initialize_time = std::chrono::system_clock::now();

            std::chrono::high_resolution_clock high_resolution_clock;
            auto tm = std::chrono::time_point_cast<std::chrono::nanoseconds>(high_resolution_clock.now());
            RndSeed = (unsigned long) (tm.time_since_epoch().count());

            for (int destId = 0; destId < ChildProcCount; ++destId) {
                MPI::COMM_WORLD.Send(&initialize_time, sizeof(initialize_time), MPI::BYTE, destId, 0);
                MPI::COMM_WORLD.Send(&RndSeed, sizeof(RndSeed), MPI::BYTE, destId, 0);
            }
        } else {
            MPI::Status status;
            MPI::COMM_WORLD.Recv(&initialize_time, sizeof(initialize_time), MPI::BYTE, ManagerProcId, 0, status);
            MPI::COMM_WORLD.Recv(&RndSeed, sizeof(RndSeed), MPI::BYTE, ManagerProcId, 0, status);
        }

        uniformDistributionZeroOne = std::uniform_real_distribution<double>(0, 1);
        uniformDistributionZeroTwoPi = std::uniform_real_distribution<double>(0, 2 * M_PI);

        auto tmp = (SimulationParameters.ParticleDiameter / SimulationParameters.Density) / 4;
        initialDisplacementDistribution = std::uniform_real_distribution<double>(-tmp, tmp);

        rnd_gen();
        rnd_gen = std::mt19937_64(RndSeed);
    };

    bool PrintTimeExtrapolation(std::chrono::time_point<std::chrono::system_clock> &start_time,
                                uint64_t &prev_measure, uint64_t totalCycles, uint64_t cycle) const;

    virtual void DoCycle() { };

    double GetAveragePotentialEnergy() const;

    void SaveForPovray(std::fstream &ofstr);

    void SyncBeforeSave();

    size_t epsLine;

    void SaveIntoEps(EPSPlot &outFile);

    CQuaternion GetRandomUnitQuaternion();;
    int ManagerProcId;
    int ProcCount;
protected:
    std::vector<CParticleBase> particles_init;

    std::vector<CYukawaDipolePt> particles_old;

    std::vector<CYukawaDipolePt> particles_new;

    std::vector<CDataChunk<CYukawaDipolePt> > ProcessMap_old;
    std::vector<CDataChunk<CYukawaDipolePt> > ProcessMap_new;

    std::vector<CDataChunk<CYukawaDipolePt> > ProcessMapFull;
    int ChildProcCount;

    size_t PerProcCount;

    double GetParticlePotentialEnergy_const(size_t ptIndex) const;
    double GetParticlePotentialEnergy(size_t ptIndex);

    double GetOrderParameter() const;

    std::vector<double> GetParticlesOrientationZ() const;

    std::vector<double> GetParticleCoordinatesZ() const;

    size_t GetNext(size_t ptIndex) const;

    size_t GetPrevious(size_t ptIndex) const;

    void AccountForBorderAfterMove(CYukawaDipolePt &pt_new);

    template<typename T>
    size_t GetNearestIndex(T arr, double val, size_t size) {
        size_t ret = 0;
        double dst_old = 100000;
        for (size_t i = 0; i < size; i++) {
            double dst = std::abs(val - arr[i]);
            if (dst < dst_old) {
                dst_old = dst;
                ret = i;
            }
        }
        return ret;
    }

    CYukawaDipolePt getPt(size_t i);

    void CreateDataMapping();

    void SyncToMain();

    void SyncInCycle();

    CVector GetNormalRandomVector(std::normal_distribution<double> &normalDistribution);
};


#endif //PROJECT_CBASESIMCTRL_H