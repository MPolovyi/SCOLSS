//
// Created by mpolovyi on 11/11/15.
//

#ifndef SMALLTESTS_SIMULATIONCONTROLLER_H
#define SMALLTESTS_SIMULATIONCONTROLLER_H

#include <vector>
#include <list>
#include <random>
#include <cereal/cereal.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/archives/json.hpp>
#include <fstream>
#include <chrono>

#include <SCOLSS/ParticlePhysics/CParticleBase.h>
#include <SCOLSS/ParticlePhysics/CParticle.h>
#include <SCOLSS/EPSPlot/EPSPlot.h>

#include "CSimulationParameters.h"

class CSimulationController {
public:
    unsigned long Cycles;

    std::chrono::time_point<std::chrono::system_clock> initialize_time;

    unsigned long RandomSeed;

    std::mt19937_64 rnd_gen;
    std::uniform_real_distribution<double> uniformDistribution;
    std::normal_distribution<double> translationNormalDistribution;
    std::normal_distribution<double> rotationNormalDistribution;

    std::uniform_real_distribution<double> uniformDistributionZeroTwoPi;
    std::uniform_real_distribution<double> uniformDistributionZeroOne;

    std::uniform_real_distribution<double> initialDisplacementDistribution;

    const CSimulationParameters SimulationParameters;

    template<class Archive>
    void serialize(Archive &archieve) {
        archieve(cereal::make_nvp("SimulationParameters", SimulationParameters));
        archieve(cereal::make_nvp("SimulationTime", GetSimulationTime()));
        archieve(cereal::make_nvp("OrderParameter", GetOrderParameter()));

        archieve(cereal::make_nvp("angle_distance", GetAverAngularDispl()));
#ifdef NON_INTERACTING
        archieve(cereal::make_nvp("distance", GetAverageDispl()));
        archieve(cereal::make_nvp("angle_distance", GetAverAngularDispl()));
        archieve(cereal::make_nvp("angle_distanceX", GetAverAngularDisplX()));
        archieve(cereal::make_nvp("angle_distanceY", GetAverAngularDisplY()));
        archieve(cereal::make_nvp("angle_distanceZ", GetAverAngularDisplZ()));

        {
            std::vector<double> tmp;
            for(auto& pt : particles_old){
                tmp.push_back(pt.Velocity);
            }
            archieve(cereal::make_nvp("v_z", tmp));
        }
        {
            std::vector<double> tmp;
            for(auto& pt : particles_old){
                tmp.push_back(pt.AngularVelocity.X);
            }
            archieve(cereal::make_nvp("omega_x", tmp));
        }
        {
            std::vector<double> tmp;
            for(auto& pt : particles_old){
                tmp.push_back(pt.AngularVelocity.Y);
            }
            archieve(cereal::make_nvp("omega_y", tmp));
        }
        {
            std::vector<double> tmp;
            for(auto& pt : particles_old){
                tmp.push_back(pt.AngularVelocity.Z);
            }
            archieve(cereal::make_nvp("omega_z", tmp));
        }
#else
        archieve(cereal::make_nvp("Energy", GeAveragetKineticEnergy()));
        if (SimulationParameters.PtCount != 2) {
            {
                std::vector<CParticleBase> pts_save;
                for (int i = 0; i < SimulationParameters.PtCount; ++i) {
                    pts_save.push_back(particles_old[i]);
                }

                const CParticleBase *tmp = &pts_save[0];
                archieve.saveBinaryValue(tmp, sizeof(CParticleBase) * particles_old.size(), "Particles");
            }
        }
        else {
            archieve(cereal::make_nvp("Coord1", particles_old[0].Coordinates));
            archieve(cereal::make_nvp("Coord2", particles_old[1].Coordinates));
            archieve(cereal::make_nvp("CosTheta2", particles_old[1].GetOrientation().Z));
        }
#endif
    }

    CSimulationController(CSimulationParameters d) : SimulationParameters(d) {
        Cycles = 0;
        epsLine = 1;

        InitRandomGenerator();
        for (int i = 0; i < SimulationParameters.PtCount; i++) {
            CParticle pt(SimulationParameters.YukawaA, SimulationParameters.YukawaK, SimulationParameters.SystemSize);
            pt.Coordinates = i > 0
                             ? i * (SimulationParameters.ParticleDiameter / SimulationParameters.Density) +
                               initialDisplacementDistribution(rnd_gen)
                             : 0;

            switch (SimulationParameters.InitialConfiguration) {
                case EInitialConfiguration::Random: {
                    pt.Rotation = GetRandomUnitQuaternion();
                    break;
                }

                case EInitialConfiguration::Aligned: {
                    pt.Rotation = CQuaternion(0, CVector::AxisZ);
                    break;
                }

                case EInitialConfiguration::AlignedTwoSides: {
                    pt.Rotation = CQuaternion(M_PI * (i % 2), CVector::AxisY);
                    break;
                }
            }
#ifdef NON_INTERACTING
            pt.Rotation = CQuaternion(0, CVector::z);
            pt.Coordinates = 0;
#endif
            particles_new.push_back(pt);
            particles_old.push_back(pt);
        }

        if (SimulationParameters.PtCount == 2) {
            particles_new[1].Coordinates = 1;
            particles_old[1].Coordinates = 1;

            particles_new[0].Rotation = CQuaternion(0, CVector(1, 1, 1));
            particles_old[0].Rotation = CQuaternion(0, CVector(1, 1, 1));

            particles_new[1].Rotation = CQuaternion(0, CVector(1, 1, 1));
            particles_old[1].Rotation = CQuaternion(0, CVector(1, 1, 1));
        }

        initialize_time = std::chrono::system_clock::now();
    }

    void InitRandomGenerator() {
        uniformDistribution = std::uniform_real_distribution<double>(-M_PI, M_PI);
        translationNormalDistribution = std::normal_distribution<double>(0, SimulationParameters.DistributionDeviationTranslation);
        rotationNormalDistribution = std::normal_distribution<double>(0, SimulationParameters.DistributionDeviationRotation);

        uniformDistributionZeroOne = std::uniform_real_distribution<double>(0, 1);
        uniformDistributionZeroTwoPi = std::uniform_real_distribution<double>(0, 2*M_PI);

        initialDisplacementDistribution = std::uniform_real_distribution<double>(
                -(SimulationParameters.ParticleDiameter/SimulationParameters.Density)/4,
                (SimulationParameters.ParticleDiameter/SimulationParameters.Density)/4);

        time_t rawtime;
        time(&rawtime);

        RandomSeed = rand() + rand() + rand() + rawtime;

        rnd_gen = std::mt19937_64(RandomSeed);
    };

    void RunSimulation(cereal::JSONOutputArchive &outputArchive, EPSPlot& pictureSaver) {
        outputArchive.makeArray();

        outputArchive(*this);
        SaveIntoEps(pictureSaver);
        std::chrono::time_point<std::chrono::system_clock> start_time, step_time;
        start_time = std::chrono::system_clock::now();
        uint64_t prev_measure = 0;

        uint64_t totalCycles = SimulationParameters.CyclesBetweenSaves * SimulationParameters.NumberOfSavePoints;

        int k = 1;
        for (uint64_t cycle = 1; cycle <= totalCycles; ++cycle) {
            DoCycle();
            
            if (0 == cycle % (SimulationParameters.CyclesBetweenSaves)) {
                outputArchive(*this);
            }

            if (0 == cycle % (totalCycles / SimulationParameters.NumberOfImageLines)) {
                SaveIntoEps(pictureSaver);
            }

            auto doFinish = PrintTimeExtrapolation(start_time, prev_measure, totalCycles, cycle);
            if (doFinish) {
                outputArchive(*this);
                SaveIntoEps(pictureSaver);
                break;
            }
        }
    }

    bool PrintTimeExtrapolation(std::chrono::time_point<std::chrono::system_clock> &start_time,
                                uint64_t &prev_measure, uint64_t totalCycles, uint64_t cycle) const {
        auto step_time = std::chrono::system_clock::now();

        std::chrono::duration<double> elapsed_secs = step_time - start_time;

        if ((elapsed_secs.count() > 15 && prev_measure == 0) || elapsed_secs.count() > 300) {
            auto sps = elapsed_secs / (cycle - prev_measure);

            auto req_s = sps * (totalCycles - cycle);

            auto req_h = std::chrono::duration_cast<std::chrono::hours>(req_s).count();
            auto req_min = std::chrono::duration_cast<std::chrono::minutes>(req_s).count() - req_h * 60;
            auto req_sec = std::chrono::duration_cast<std::chrono::seconds>(req_s).count() - req_h * 3600 - req_min * 60;

            std::cout << " will run for "
            << req_h << " hours "
            << req_min << " minutes "
            << req_sec << " seconds" << std::endl;
            start_time = std::chrono::system_clock::now();
            prev_measure = cycle;
        }

        elapsed_secs = step_time - initialize_time;
        auto hours_since_init = std::chrono::duration_cast<std::chrono::hours>(elapsed_secs).count();

        return hours_since_init == 23;
    }

    void DoCycle() {
        Cycles += 1;
        if (SimulationParameters.PtCount == 2) {
            RotateParticleVerlet(1);
            AccelerateRotateParticleVerlet(1);
        }
        else {
            for (int i = 0; i < SimulationParameters.PtCount; i++) {
                MoveParticleVerlet(i);
                RotateParticleVerlet(i);
            }
            for (int i = 0; i < SimulationParameters.PtCount; i++) {
                AccelerateMoveParticleVerlet(i);
                AccelerateRotateParticleVerlet(i);
            }
        }
        std::swap(particles_old, particles_new);
    }

    inline void MoveParticleVerlet(int ptIndex) {
        CParticle &pt_new = particles_new[ptIndex];
        CParticle &pt_old = particles_old[ptIndex];

        pt_new.ForceRandom = translationNormalDistribution(rnd_gen);

        pt_new.ForceOld = GetForceOld(ptIndex);

        auto av = SimulationParameters.VerletCoefficientTranslation * pt_old.Velocity
                  + SimulationParameters.VerletCoefficientTranslation * pt_new.ForceRandom / (2 * SimulationParameters.ParticleMass)
                  + SimulationParameters.VerletCoefficientTranslation * pt_new.ForceOld * SimulationParameters.TimeStep /
                    (2 * SimulationParameters.ParticleMass);

        pt_new.Coordinates =
                pt_old.Coordinates
                + av * SimulationParameters.TimeStep;
    }

    inline void AccelerateMoveParticleVerlet(int ptIndex) {
        CParticle &pt_new = particles_new[ptIndex];
        CParticle &pt_old = particles_old[ptIndex];

        auto dv = SimulationParameters.TimeStep * (pt_new.ForceOld + GetForceNew(ptIndex)) / (2 * SimulationParameters.ParticleMass)
                  - SimulationParameters.AlphaTranslational * (pt_new.Coordinates - pt_old.Coordinates) / SimulationParameters.ParticleMass
                  + pt_new.ForceRandom / SimulationParameters.ParticleMass;

        pt_new.Velocity = pt_old.Velocity + dv;

#ifndef NON_INTERACTING
        AccountForBorderAfterMove(pt_new);
#endif
    }

    inline void RotateParticleVerlet(int ptIndex) {
        CParticle &pt_new = particles_new[ptIndex];
        CParticle &pt_old = particles_old[ptIndex];

        pt_new.TorqueRandom = GetNormalRandomVector(rotationNormalDistribution);
        pt_new.TorqueOld = GetTorqueOld(ptIndex);

        std::vector<CVector> av = {
                SimulationParameters.VerletCoefficientRotation * pt_old.AngularVelocity,
                SimulationParameters.VerletCoefficientRotation * pt_new.TorqueRandom / (2 * SimulationParameters.Inertia),
                SimulationParameters.VerletCoefficientRotation * pt_new.TorqueOld * SimulationParameters.TimeStep /
                (2 * SimulationParameters.Inertia)
        };

        pt_new.Rotation = pt_old.GetRotationFromVelocity(av, SimulationParameters.TimeStep);

        pt_new.AngularDisplacement = pt_old.AngularDisplacement + CParticle::GetRotationDistance(pt_new, pt_old);
    }

    inline void AccelerateRotateParticleVerlet(int ptIndex) {
        CParticle &pt_new = particles_new[ptIndex];
        CParticle &pt_old = particles_old[ptIndex];

        pt_new.AngularVelocity = pt_old.AngularVelocity
                                 + SimulationParameters.TimeStep * (pt_new.TorqueOld + GetTorqueNew(ptIndex)) / (2 * SimulationParameters.Inertia)
                                 - SimulationParameters.AlphaRotational * (CParticle::GetRotationDistance(pt_new, pt_old)) / SimulationParameters.Inertia
                                 + pt_new.TorqueRandom / SimulationParameters.Inertia;
    }

    inline double GetForceOld(int ptIndex) {
#ifdef NON_INTERACTING
        return 0;
#else
        if (SimulationParameters.PtCount == 2) {
            switch (ptIndex) {
                case 0:
                    return particles_old[0].GetForceFromOtherTheoretically(particles_old[1],
                                                                           particles_old[1].GetDistanceLeft(particles_old[0], SimulationParameters.SystemSize));
                case 1:
                    return particles_old[1].GetForceFromOtherTheoretically(particles_old[0],
                                                                           particles_old[0].GetDistanceRight(particles_old[1], SimulationParameters.SystemSize));
            }
        }
        else {
            return particles_old[ptIndex].GetForceFromOther(particles_old[GetPrevious(ptIndex)],
                                                            particles_old[GetNext(ptIndex)]);
        }
#endif
    }

    inline double GetForceNew(int ptIndex) {
#ifdef NON_INTERACTING
        return 0;
#else
        if (SimulationParameters.PtCount == 2) {
            switch (ptIndex) {
                case 0:
                    return particles_new[0].GetForceFromOtherTheoretically(particles_old[1],
                                                                           particles_old[1].GetDistanceLeft(particles_old[0], SimulationParameters.SystemSize));
                case 1:
                    return particles_new[1].GetForceFromOtherTheoretically(particles_old[0],
                                                                           particles_old[0].GetDistanceRight(particles_old[1], SimulationParameters.SystemSize));
            }
        }
        else {
            return particles_new[ptIndex].GetForceFromOther(particles_old[GetPrevious(ptIndex)],
                                                            particles_old[GetNext(ptIndex)]);
        }
#endif
    }

    inline CVector GetTorqueOld(int ptIndex) {
#ifdef NON_INTERACTING
        return CVector(0, 0, 0);
#else
        if (SimulationParameters.PtCount == 2) {
            switch (ptIndex) {
                case 0:
                    return particles_old[0].GetTorqueFromOther(particles_old[1], particles_old[1].GetDistanceLeft(particles_old[0], SimulationParameters.SystemSize));
                case 1:
                    return particles_old[1].GetTorqueFromOther(particles_old[0], particles_old[0].GetDistanceRight(particles_old[1], SimulationParameters.SystemSize));
            }
        }
        else {
            return particles_old[ptIndex].GetTorqueFromOther(particles_old[GetPrevious(ptIndex)],
                                                             particles_old[GetNext(ptIndex)]);
        }
#endif

    }

    inline CVector GetTorqueNew(int ptIndex) {
#ifdef NON_INTERACTING
        return CVector(0, 0, 0);
#else
        if (SimulationParameters.PtCount == 2) {
            switch (ptIndex) {
                case 0:
                    return particles_new[0].GetTorqueFromOther(particles_new[1], particles_new[1].GetDistanceLeft(particles_new[0], SimulationParameters.SystemSize));
                case 1:
                    return particles_new[1].GetTorqueFromOther(particles_new[0], particles_new[0].GetDistanceRight(particles_new[1], SimulationParameters.SystemSize));
            }
        }
        else {
            return particles_new[ptIndex].GetTorqueFromOther(particles_new[GetPrevious(ptIndex)],
                                                             particles_new[GetNext(ptIndex)]);
        }
#endif
    }

    double GetAveragePotentialEnergy() {
        if (SimulationParameters.PtCount == 2) {
            return particles_old[0].GetPotentialEnergy(particles_old[1], particles_old[1].GetDistanceLeft(particles_old[0], SimulationParameters.SystemSize));
        }

        double ret = 0;

        for (int i = 0; i < SimulationParameters.PtCount; i++) {
            auto &pt = particles_old[i];

            ret += GetParticleEnergyOld(i);
        }

        return ret / 2.0 / SimulationParameters.PtCount;
    }

    double GeAveragetKineticEnergy() {
        double ret = 0;

        for (int i = 0; i < SimulationParameters.PtCount; i++) {
            auto &pt = particles_old[i];

            ret += SimulationParameters.ParticleMass * pt.Velocity * pt.Velocity / 2;
            ret += SimulationParameters.Inertia * pt.AngularVelocity.GetLengthSquared() / 2;
        }

        return ret/SimulationParameters.PtCount;
    }

    double GetRotationalEnergy() {
        double ret = 0;

        for (int i = 0; i < SimulationParameters.PtCount; i++) {
            auto &pt = particles_old[i];

            ret += SimulationParameters.Inertia * pt.AngularVelocity.GetLengthSquared() / 2;
        }

        return ret;
    }

    double GetTranslationalEnergy() {
        double ret = 0;

        for (int i = 0; i < SimulationParameters.PtCount; i++) {
            auto &pt = particles_old[i];

            ret += SimulationParameters.ParticleMass * pt.Velocity * pt.Velocity / 2;
        }

        return ret;
    }

    double GetSimulationTime() {
        return Cycles * SimulationParameters.TimeStep;
    }

    void SaveForPovray(std::fstream& ofstr) {
        for (int i = 0; i < SimulationParameters.PtCount; ++i) {
            auto rot = particles_old[i].GetOrientation();
            ofstr << "Colloid(<0, 0, " << particles_old[i].Coordinates/SimulationParameters.ParticleDiameter << ">, <" << rot.X << ", " << rot.Y << ", " << rot.Z << ">)" <<
            std::endl;
        }
    }

    unsigned int epsLine;

    void SaveIntoEps(EPSPlot &outFile) {
        epsLine++;
        for (int i = 0; i < SimulationParameters.PtCount; ++i) {
            auto& pt = particles_old[i];
            auto orient = pt.GetOrientation();

            float colR;
            float colG;
            float colB;
            if(orient.Z > 0){
                colB = 1 - (float) orient.Z;
                colG = colB;
                colR = 1;
            }
            else{
                colR = 1 - (float) std::abs(orient.Z);
                colG = colR;
                colB = 1;
            }

            outFile.drawPoint((float) pt.Coordinates,
                              (float) (epsLine * SimulationParameters.ParticleDiameter),
                              (float) SimulationParameters.ParticleDiameter,
                              colR, colG, colB);
        }
    }

    CQuaternion GetRandomUnitQuaternion() {
        auto u1 = uniformDistributionZeroOne(rnd_gen);
        auto u2 = uniformDistributionZeroTwoPi(rnd_gen);
        auto u3 = uniformDistributionZeroTwoPi(rnd_gen);

        auto ret = CQuaternion(
                sqrt(1 - u1) * sin(u2),
                sqrt(1 - u1) * cos(u2),
                sqrt(u1) * sin(u3),
                sqrt(u1) * cos(u3)
        );

        return ret;
    };

    std::vector<CParticle> particles_old;
    std::vector<CParticle> particles_new;
protected:
    double GetAverageDispl() {
        double ret = 0;

        for (int i = 0; i < particles_old.size(); ++i) {
            CParticle &pt_old = particles_old[i];

            auto dr = pow(pt_old.Coordinates, 2);

            ret += dr;
        }

        return ret / SimulationParameters.PtCount;
    }

    double GetAverAngularDispl() {
        double ret = 0;

        for (int i = 0; i < particles_old.size(); ++i) {
            CParticle &pt_old = particles_old[i];

            ret += pt_old.AngularDisplacement.GetLengthSquared();
        }

        return ret / SimulationParameters.PtCount;
    }

    double GetAverAngularDisplX() {
        double ret = 0;

        for (int i = 0; i < particles_old.size(); ++i) {
            CParticle &pt_old = particles_old[i];

            ret += pow(pt_old.AngularDisplacement.X, 2);
        }

        return ret / SimulationParameters.PtCount;
    }

    double GetAverAngularDisplY() {
        double ret = 0;

        for (int i = 0; i < particles_old.size(); ++i) {
            CParticle &pt_old = particles_old[i];

            ret += pow(pt_old.AngularDisplacement.Y, 2);
        }

        return ret / SimulationParameters.PtCount;
    }

    double GetAverAngularDisplZ() {
        double ret = 0;

        for (int i = 0; i < particles_old.size(); ++i) {
            CParticle &pt_old = particles_old[i];

            ret += pow(pt_old.AngularDisplacement.Z, 2);
        }

        return ret / SimulationParameters.PtCount;
    }

    double GetParticleEnergyOld(int ptIndex) {
        return particles_old[ptIndex].GetPotentialEnergy(particles_old[GetNext(ptIndex)],
                                                particles_old[GetNext(ptIndex)].GetDistanceLeft(particles_old[ptIndex], SimulationParameters.SystemSize))
               + particles_old[ptIndex].GetPotentialEnergy(particles_old[GetPrevious(ptIndex)],
                                                  particles_old[GetPrevious(ptIndex)].GetDistanceRight(particles_old[ptIndex], SimulationParameters.SystemSize));

    }

    inline CVector GetNormalRandomVector(std::normal_distribution<double> &normalDistribution) {
        return CVector(normalDistribution(rnd_gen), normalDistribution(rnd_gen), normalDistribution(rnd_gen));
    }

    inline CVector GetUniformRandomVector() {
        return CVector(uniformDistribution(rnd_gen), uniformDistribution(rnd_gen), uniformDistribution(rnd_gen));
    }

    double GetOrderParameter() {
        auto ptOrientations = GetParticlesOrientationZ();

        double op = 0;
        for (auto r : ptOrientations) {
            op += r * r;
        }
        op /= SimulationParameters.PtCount;

        op = (3 * op - 1) / 2;

        return op;
    }

    std::vector<double> GetParticlesOrientationZ() {
        std::vector<double> ret;

        for (auto &pt : particles_old) {
            ret.push_back(pt.GetOrientation().Z);
        }

        return ret;
    }

    std::vector<double> GetParticleCoordinatesZ() {
        std::vector<double> ret;

        for (auto &pt : particles_old) {
            ret.push_back(pt.Coordinates);
        }

        return ret;
    }

    inline int GetNext(int ptIndex) {
        int ret = ptIndex + 1;

        if (ret == SimulationParameters.PtCount) {
            return 0;
        }

        return ret;
    }

    inline int GetPrevious(int ptIndex) {
        int ret = ptIndex - 1;

        if (ret == -1) {
            return SimulationParameters.PtCount - 1;
        }

        return ret;
    }

    inline void AccountForBorderAfterMove(CParticle &pt_new) {
        if (SimulationParameters.PtCount == 2) {
            return;
        }
        else {
            if (pt_new.Coordinates >= SimulationParameters.SystemSize) {
                pt_new.Coordinates -= SimulationParameters.SystemSize;
            } else if (pt_new.Coordinates <= 0) {
                pt_new.Coordinates += SimulationParameters.SystemSize;
            }
        }
    }

    template<typename T>
    int GetNearestIndex(T arr, double val, int size) {
        int ret = 0;
        double dst_old = 100000;
        for (int i = 0; i < size; i++) {
            double dst = std::abs(val - arr[i]);
            if (dst < dst_old) {
                dst_old = dst;
                ret = i;
            }
        }
        return ret;
    }
};


#endif //SMALLTESTS_SIMULATIONCONTROLLER_H
