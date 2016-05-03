//
// Created by mpolovyi on 25/01/16.
//

#include "CBaseSimCtrl.h"

CBaseSimCtrl::CBaseSimCtrl(CBaseSimParams d) : SimulationParameters(d) {
    int currentId = MPI::COMM_WORLD.Get_rank();
    ProcCount = MPI::COMM_WORLD.Get_size();
    ManagerProcId = ProcCount - 1;
    ChildProcCount = ProcCount - 1;

    Cycles = 0;
    epsLine = 1;

    InitRandomGenerator();
    for (size_t i = 0; i < SimulationParameters.PtCount; i++) {
        auto pt = getPt(i);
        particles_new.push_back(pt);
        particles_old.push_back(pt);
    }

    PerProcCount = SimulationParameters.PtCount / ProcCount;

    CreateDataMapping();
}

void CBaseSimCtrl::CreateDataMapping() {
    int currentId = MPI::COMM_WORLD.Get_rank();
    
    bool cleared = false;
    if (ProcessMapFull.size() != ProcCount
        || ProcessMap_old.size() != ProcCount
        || ProcessMap_new.size() != ProcCount) {

        ProcessMapFull.clear();
        ProcessMap_old.clear();
        ProcessMap_new.clear();

        cleared = true;
    }

    for (int procId = 0; procId < ProcCount; ++procId) {
        if (cleared) {
            ProcessMapFull.push_back(CDataChunk<CYukawaDipolePt>());
            ProcessMap_old.push_back(CDataChunk<CYukawaDipolePt>());
            ProcessMap_new.push_back(CDataChunk<CYukawaDipolePt>());
        }

        ProcessMapFull[procId].Init(&particles_old[procId * PerProcCount], PerProcCount, procId);
        ProcessMap_old[procId].Init(&particles_old[procId * PerProcCount], PerProcCount, procId);
        ProcessMap_new[procId].Init(&particles_new[procId * PerProcCount], PerProcCount, procId);
    }


    for (int procId = 0; procId < ProcCount; ++procId) {
        ProcessMap_old[procId].Init(&particles_old[procId * PerProcCount], PerProcCount, procId,
                                    procId == ProcCount - 1 ? ProcessMap_old[0].begin() : ProcessMap_old[procId + 1].begin(),
                                    procId == 0 ? ProcessMap_old[ProcCount - 1].last() : ProcessMap_old[procId - 1].last());

        ProcessMap_new[procId].Init(&particles_new[procId * PerProcCount], PerProcCount, procId,
                                    procId == ProcCount - 1 ? ProcessMap_new[0].begin() : ProcessMap_new[procId + 1].begin(),
                                    procId == 0 ? ProcessMap_new[ProcCount - 1].last() : ProcessMap_new[procId - 1].last());
    }
}

CYukawaDipolePt CBaseSimCtrl::getPt(size_t i) {
    CYukawaDipolePt pt(SimulationParameters.YukawaA, SimulationParameters.YukawaK, SimulationParameters.SystemSize);
    switch (SimulationParameters.InitialConfiguration) {
            case Random: {
                pt.SetRotation(GetRandomUnitQuaternion());
                pt.Coordinates = i > 0
                                 ? i * (SimulationParameters.ParticleDiameter / SimulationParameters.Density) +
                                   initialDisplacementDistribution(rnd_gen)
                                 : 0;
                break;
            }

            case RandomUnmoving: {
                pt.SetRotation(GetRandomUnitQuaternion());
                pt.Coordinates = i * (SimulationParameters.ParticleDiameter / SimulationParameters.Density);
                break;
            }

            case Aligned: {
                pt.SetRotation(CQuaternion(0, CVector::AxisZ));
                pt.Coordinates = i > 0
                                 ? i * (SimulationParameters.ParticleDiameter / SimulationParameters.Density) +
                                   initialDisplacementDistribution(rnd_gen)
                                 : 0;
                break;
            }

            case AlignedTwoSides: {
                pt.SetRotation(CQuaternion(M_PI * (i % 2), CVector::AxisY));
                pt.Coordinates = i > 0
                                 ? i * (SimulationParameters.ParticleDiameter / SimulationParameters.Density) +
                                   initialDisplacementDistribution(rnd_gen)
                                 : 0;
                break;
            }

            case AlingnedUnmoving: {
                pt.SetRotation(CQuaternion(M_PI * (i % 2), CVector::AxisY));
                pt.Coordinates = i * (SimulationParameters.ParticleDiameter / SimulationParameters.Density);
                break;
            }
            case OneCluster: {
                pt.SetRotation(CQuaternion(0, CVector::AxisZ));
                pt.Coordinates = SimulationParameters.SystemSize / 4 + SimulationParameters.ParticleDiameter * i;
            }
        }
    return pt;
}

bool CBaseSimCtrl::PrintTimeExtrapolation(std::chrono::time_point<std::chrono::system_clock> &start_time,
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

    return hours_since_init == 48;
}

double CBaseSimCtrl::GetAveragePotentialEnergy() const {
    double ret = 0;

    for (size_t i = 0; i < SimulationParameters.PtCount; i++) {
        auto &pt = particles_old[i];

        ret += GetParticlePotentialEnergy_const(i);
    }

    return ret / 2.0 / SimulationParameters.PtCount;
}

void CBaseSimCtrl::SaveForPovray(std::fstream &ofstr) {
    for (size_t i = 0; i < SimulationParameters.PtCount; ++i) {
        auto rot = particles_old[i].Orientation;
        ofstr
        << "Colloid(<0, 0, "
        << particles_old[i].Coordinates / SimulationParameters.ParticleDiameter
        << ">, <" << rot.X << ", " << rot.Y << ", " << rot.Z << ">)" <<
        std::endl;
    }
}

void CBaseSimCtrl::SaveIntoEps(EPSPlot &outFile) {
    epsLine++;
    for (size_t i = 0; i < SimulationParameters.PtCount; ++i) {
        auto &pt = particles_old[i];
        auto orient = pt.Orientation;

        float colR;
        float colG;
        float colB;
        if (orient.Z > 0) {
            colB = 1 - (float) orient.Z;
            colG = colB;
            colR = 1;
        }
        else {
            colR = 1 - (float) std::abs(orient.Z);
            colG = colR;
            colB = 1;
        }

        outFile.drawParticle((float) pt.Coordinates,
                          (float) (epsLine * SimulationParameters.ParticleDiameter),
                          colR, colG, colB);
    }
}

CQuaternion CBaseSimCtrl::GetRandomUnitQuaternion() {
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
}

double CBaseSimCtrl::GetParticlePotentialEnergy_const(size_t ptIndex) const {
    return particles_old[ptIndex].GetPotentialEnergy1D(particles_old[GetNext(ptIndex)],
                                                       particles_old[GetNext(ptIndex)].GetDistanceLeft(particles_old[ptIndex],
                                                                                                       SimulationParameters.SystemSize))
           + particles_old[ptIndex].GetPotentialEnergy1D(particles_old[GetPrevious(ptIndex)],
                                                         particles_old[GetPrevious(ptIndex)].GetDistanceRight(particles_old[ptIndex],
                                                                                                              SimulationParameters.SystemSize));

}

double CBaseSimCtrl::GetParticlePotentialEnergy(size_t ptIndex) {
    auto& next = particles_old[GetNext(ptIndex)];
    auto& prev = particles_old[GetPrevious(ptIndex)];

    if(prev.Moved || next.Moved || particles_old[ptIndex].Moved) {
        particles_old[ptIndex].NewPotentialEnergy =
                particles_old[ptIndex].GetPotentialEnergy1D(next, next.GetDistanceLeft(particles_old[ptIndex], SimulationParameters.SystemSize))
                + particles_old[ptIndex].GetPotentialEnergy1D(prev, prev.GetDistanceRight(particles_old[ptIndex], SimulationParameters.SystemSize));

        return particles_old[ptIndex].NewPotentialEnergy;
    } else {
        return particles_old[ptIndex].PotentialEnergy;
    }
}

double CBaseSimCtrl::GetOrderParameter() const {
    auto ptOrientations = GetParticlesOrientationZ();

    double op = 0;
    for (auto r : ptOrientations) {
        op += r * r;
    }
    op /= SimulationParameters.PtCount;

    op = (3 * op - 1) / 2;

    return op;
}

std::vector<double> CBaseSimCtrl::GetParticlesOrientationZ() const {
    std::vector<double> ret;

    for (auto &pt : particles_old) {
        ret.push_back(pt.Orientation.Z);
    }

    return ret;
}

std::vector<double> CBaseSimCtrl::GetParticleCoordinatesZ() const {
    std::vector<double> ret;

    for (auto &pt : particles_old) {
        ret.push_back(pt.Coordinates);
    }

    return ret;
}

size_t CBaseSimCtrl::GetNext(size_t ptIndex) const {
    size_t ret = ptIndex + 1;

    if (ret == SimulationParameters.PtCount) {
        return 0;
    }

    return ret;
}

size_t CBaseSimCtrl::GetPrevious(size_t ptIndex) const {
    size_t ret = ptIndex - 1;

    if (ret == -1) {
        return SimulationParameters.PtCount - 1;
    }

    return ret;
}

void CBaseSimCtrl::AccountForBorderAfterMove(CYukawaDipolePt &pt_new) {
    if (pt_new.Coordinates >= SimulationParameters.SystemSize) {
        pt_new.Coordinates -= SimulationParameters.SystemSize;
    } else if (pt_new.Coordinates <= 0) {
        pt_new.Coordinates += SimulationParameters.SystemSize;
    }

}

void CBaseSimCtrl::SyncBeforeSave() {
    SyncToMain();
}

void CBaseSimCtrl::SyncToMain() {
    CreateDataMapping();
    int currentId = MPI::COMM_WORLD.Get_rank();
    if (currentId == ManagerProcId) {
        for (int sourceId = 0; sourceId < ChildProcCount; ++sourceId) {
            MPI::Status status;
            MPI::COMM_WORLD.Recv(&ProcessMapFull[sourceId], ProcessMapFull[sourceId].size_in_bytes(), MPI::BYTE, sourceId, 0, status);
        }
    } else {
        MPI::COMM_WORLD.Send(&ProcessMapFull[currentId], ProcessMapFull[currentId].size_in_bytes(), MPI::BYTE, ManagerProcId, 0);
    }
}

void CBaseSimCtrl::SyncInCycle() {
    CreateDataMapping();
    int currentId = MPI::COMM_WORLD.Get_rank();

    int prevId = currentId - 1;
    int nexId = currentId + 1;

    if (prevId == -1) {
        prevId = ProcCount - 1;
    }
    if (nexId == ProcCount) {
        nexId = 0;
    }

    MPI::COMM_WORLD.Send(ProcessMap_old[currentId].begin(), ProcessMap_old[currentId].size_of_data(), MPI::BYTE, prevId, 0);
    MPI::COMM_WORLD.Send(ProcessMap_old[currentId].last(), ProcessMap_old[currentId].size_of_data(), MPI::BYTE, nexId, 0);

    MPI::Status status;
    MPI::COMM_WORLD.Recv(ProcessMap_old[currentId].linkToPrev(), ProcessMap_old[currentId].size_of_data(), MPI::BYTE, prevId, 0, status);
    MPI::COMM_WORLD.Recv(ProcessMap_old[currentId].linkToNext(), ProcessMap_old[currentId].size_of_data(), MPI::BYTE, nexId, 0, status);

    MPI::COMM_WORLD.Send(ProcessMap_new[currentId].begin(), ProcessMap_new[currentId].size_of_data(), MPI::BYTE, prevId, 0);
    MPI::COMM_WORLD.Send(ProcessMap_new[currentId].last(), ProcessMap_new[currentId].size_of_data(), MPI::BYTE, nexId, 0);

    MPI::COMM_WORLD.Recv(ProcessMap_new[currentId].linkToPrev(), ProcessMap_new[currentId].size_of_data(), MPI::BYTE, prevId, 0, status);
    MPI::COMM_WORLD.Recv(ProcessMap_new[currentId].linkToNext(), ProcessMap_new[currentId].size_of_data(), MPI::BYTE, nexId, 0, status);
}

CVector CBaseSimCtrl::GetNormalRandomVector(std::normal_distribution<double> &normalDistribution) {
    return CVector(normalDistribution(rnd_gen), normalDistribution(rnd_gen), normalDistribution(rnd_gen));
}