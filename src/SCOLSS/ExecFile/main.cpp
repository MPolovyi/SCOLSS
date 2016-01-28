//
// Created by mpolovyi on 22/01/16.
//

#include <iostream>
#include <cereal/archives/json.hpp>
#include <cereal/types/polymorphic.hpp>

#include <SCOLSS/SimulationController/MonteCarloSimCtrl/CMonteCarloSimParams.h>
#include <SCOLSS/SimulationController/MonteCarloSimCtrl/CMonteCarloSimCtrl.h>
#include <SCOLSS/SimulationController/LangevinSimCtrl/CLangevinSimParams.h>
#include <SCOLSS/SimulationController/LangevinSimCtrl/CLangevinSimCtrl.h>

#include <SCOLSS/ParticlePhysics/CYukawaDipolePt.h>

void InitializeSimulations(int argc, char **argv);
void RunSimulations(std::shared_ptr<CBaseSimCtrl> sim, std::string &mainSaveFileName, EPSPlot &pictureSaver);

void SaveToFile(const std::shared_ptr<CBaseSimCtrl> &contr, const std::string &mainSaveFileName, uint64_t cycle);

int main(int argc, char **argv) {
//    CYukawaDipolePt pt1(1000, 10, 10);
//    CYukawaDipolePt pt2(1000, 10, 10);
//    pt1.Coordinates = 4;
//    pt2.Coordinates = 0;
//    pt2.SetRotation(CQuaternion(0, CVector::AxisX));
//
//    std::cout << "Particle one " << pt1.Orientation << std::endl;
//    std::cout << "Particle two " << pt2.Orientation << std::endl;
//
//    cereal::JSONOutputArchive arch(std::cout);
//
//    arch.saveBinaryValue((CParticleBase*)&pt2, sizeof(CParticleBase));
//    arch.finishNode();
//    auto tmp = pt1.GetForceFromOtherTheoretically(pt2, pt2.GetDistanceLeft(pt1, 10));
//    std::cout << tmp << std::endl;
//
//    tmp = pt2.GetForceFromOtherTheoretically(pt1, pt1.GetDistanceRight(pt2, 10));
//    std::cout << tmp << std::endl;

    InitializeSimulations(argc, argv);
}

void InitializeSimulations(int argc, char **argv) {
    std::string simType = argv[2];
    ESimulationType simT;
    if(simType == "MC"){
        simT = ESimulationType::MonteCarlo;
    } else if (simType == "LD"){
        simT = ESimulationType::LangevinDynamics;
    } else {
        std::cout << "Unknown simulation type. Exiting." << std::endl;
        return;
    }

    std::string samples = argv[3];
    for(int i = 1; i <= std::stoi(samples); i++) {
        std::string fname = argv[1];
        std::ifstream simDataFileStream(fname);

        cereal::JSONInputArchive simDataArchieve(simDataFileStream);

        std::shared_ptr<CBaseSimCtrl> sim;

        switch (simT){
            case MonteCarlo: {
                CMonteCarloSimParams data;
                simDataArchieve(data);
                sim = std::make_shared<CMonteCarloSimCtrl>(CMonteCarloSimCtrl(data));
                break;
            };
            case LangevinDynamics: {
                CLangevinSimParams data;
                simDataArchieve(data);
                sim = std::make_shared<CLangevinSimCtrl>(CLangevinSimCtrl(data));
                break;
            };
        }

        std::stringstream ssMainSaveFileName;

        ssMainSaveFileName << "Results_" << i << fname;
        std::string mainSaveFileName = ssMainSaveFileName.str();

        std::string pictureSaveFileName = "Picture_" + fname + ".eps";

        EPSPlot savePictureFile(pictureSaveFileName.c_str(),
                                0,
                                0,
                                sim->SimulationParameters.GetEpsDimensionX(),
                                sim->SimulationParameters.GetEpsDimensionY());

        savePictureFile.initParticleSavings(sim->SimulationParameters.ParticleDiameter);

        RunSimulations(sim, mainSaveFileName, savePictureFile);
        std::cout << i << std::endl;
    }
}

void RunSimulations(std::shared_ptr<CBaseSimCtrl> sim, std::string &mainSaveFileName, EPSPlot &pictureSaver) {
    SaveToFile(sim, mainSaveFileName, 0);

    sim->SaveIntoEps(pictureSaver);
    std::chrono::time_point<std::chrono::system_clock> start_time, step_time;
    start_time = std::chrono::system_clock::now();
    uint64_t prev_measure = 0;

    uint64_t totalCycles = sim->SimulationParameters.CyclesBetweenSaves * sim->SimulationParameters.NumberOfSavePoints;
    sim->SimulationParameters.NumberOfImageLines = std::min(totalCycles, sim->SimulationParameters.NumberOfImageLines);
    for (uint64_t cycle = 1; cycle <= totalCycles; ++cycle) {
        sim->DoCycle();

        if (0 == cycle % (sim->SimulationParameters.CyclesBetweenSaves)) {
            SaveToFile(sim, mainSaveFileName, cycle);
        }

        if (0 == cycle % (totalCycles / sim->SimulationParameters.NumberOfImageLines)) {
            sim->SaveIntoEps(pictureSaver);
        }

        auto doFinish = sim->PrintTimeExtrapolation(start_time, prev_measure, totalCycles, cycle);
        if (doFinish) {
            SaveToFile(sim, mainSaveFileName, cycle);

            sim->SaveIntoEps(pictureSaver);
            break;
        }
    }
}

void SaveToFile(const std::shared_ptr<CBaseSimCtrl> &contr, const std::string &mainSaveFileName, uint64_t cycle) {
    std::fstream saveFileStream((mainSaveFileName + std::to_string(cycle)).c_str(), std::ios_base::out);
    cereal::JSONOutputArchive saveFileArchieve(saveFileStream);

    saveFileArchieve(contr);
}
