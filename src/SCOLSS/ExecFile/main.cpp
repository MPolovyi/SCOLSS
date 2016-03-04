//
// Created by mpolovyi on 22/01/16.
//

#include <iostream>
#include <unistd.h>

#include <stdlib.h>

#include <cereal/archives/json.hpp>
#include <cereal/types/polymorphic.hpp>

#include <SCOLSS/SimulationController/MonteCarloSimCtrl/CMonteCarloSimParams.h>
#include <SCOLSS/SimulationController/MonteCarloSimCtrl/CMonteCarloSimCtrl.h>
#include <SCOLSS/SimulationController/LangevinSimCtrl/CLangevinSimParams.h>
#include <SCOLSS/SimulationController/LangevinSimCtrl/CLangevinSimCtrl.h>

#include <SCOLSS/ParticlePhysics/CYukawaDipolePt.h>

#include <SCOLSS/ExecFile/CTarball.h>

void InitializeSimulations(int argc, char **argv);

void RunSimulations(std::shared_ptr<CBaseSimCtrl> sim,
                    std::string &fullSaveFileName, std::string &miniSaveFileName, std::string &pictSaveFileName,
                    Tar &fullTarBall, Tar &miniTarBall, Tar &pictTarBall);

void SaveToFile(std::shared_ptr<CBaseSimCtrl> &contr,
                std::string &fullSaveFileName, std::string &miniSaveFileName,
                Tar &fullTarBall, Tar &miniTarBall,
                uint64_t cycle);

int main(int argc, char **argv) {
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

    std::string simDataFileName = argv[1];

    std::fstream fullSaveTarName("FullData_" + simDataFileName + ".tar", std::ios_base::out);
    std::fstream miniSaveTarName("MiniData_" + simDataFileName + ".tar", std::ios_base::out);
    std::fstream pictSaveTarName("PictData_" + simDataFileName + ".tar", std::ios_base::out);

    Tar fullTarBall(fullSaveTarName);
    Tar miniTarBall(miniSaveTarName);
    Tar pictTarBall(pictSaveTarName);

    std::string samples = argv[3];
    for(int i = 1; i <= std::stoi(samples); i++) {
        std::ifstream simDataFileStream(simDataFileName);

        cereal::JSONInputArchive simDataArchieve(simDataFileStream);

        std::shared_ptr<CBaseSimCtrl> sim;

        switch (simT){
            case MonteCarlo: {
                CMonteCarloSimParams data;
                data.load(simDataArchieve);
                sim = std::make_shared<CMonteCarloSimCtrl>(CMonteCarloSimCtrl(data));
                break;
            };
            case LangevinDynamics: {
                CLangevinSimParams data;
                data.load(simDataArchieve);
                sim = std::make_shared<CLangevinSimCtrl>(CLangevinSimCtrl(data));
                break;
            };
        }

        std::string fullSaveFileName = "FullData_" + std::to_string(i) + "_" + simDataFileName;
        std::string miniSaveFileName = "MiniData_" + std::to_string(i) + "_" + simDataFileName;
        std::string pictSaveFileName = "PictData_" + std::to_string(i) + "_" + simDataFileName + ".eps";

        SaveToFile(sim, fullSaveFileName, miniSaveFileName, fullTarBall, miniTarBall, 0);

        RunSimulations(sim, fullSaveFileName, miniSaveFileName, pictSaveFileName,
                       fullTarBall,
                       miniTarBall,
                       pictTarBall);

        std::cout << i << std::endl;
    }
    fullTarBall.finish();
    miniTarBall.finish();
    pictTarBall.finish();
}

void RunSimulations(std::shared_ptr<CBaseSimCtrl> sim,
                    std::string &fullSaveFileName, std::string &miniSaveFileName, std::string &pictSaveFileName,
                    Tar &fullTarBall, Tar &miniTarBall, Tar &pictTarBall) {
    {

        EPSPlot savePictureFile(pictSaveFileName.c_str(),
                                0,
                                0,
                                sim->SimulationParameters.GetEpsDimensionX(),
                                sim->SimulationParameters.GetEpsDimensionY());

        savePictureFile.initParticleSavings(sim->SimulationParameters.ParticleDiameter);

        sim->SaveIntoEps(savePictureFile);

        std::chrono::time_point<std::chrono::system_clock> start_time, step_time;
        start_time = std::chrono::system_clock::now();
        uint64_t prev_measure = 0;

        uint64_t totalCycles = sim->SimulationParameters.CyclesBetweenSaves * sim->SimulationParameters.NumberOfSavePoints;
        sim->SimulationParameters.NumberOfImageLines = std::min(totalCycles, sim->SimulationParameters.NumberOfImageLines);

        for (uint64_t cycle = 1; cycle <= totalCycles; ++cycle) {
            sim->DoCycle();

            if (0 == cycle % (sim->SimulationParameters.CyclesBetweenSaves)) {
                SaveToFile(sim, fullSaveFileName, miniSaveFileName, fullTarBall, miniTarBall, cycle);
            }

            if (sim->SimulationParameters.SaveEpsPicture && 0 == cycle % (totalCycles / sim->SimulationParameters.NumberOfImageLines)) {
                sim->SaveIntoEps(savePictureFile);
            }

            auto doFinish = sim->PrintTimeExtrapolation(start_time, prev_measure, totalCycles, cycle);
            if (doFinish) {
                SaveToFile(sim, fullSaveFileName, miniSaveFileName, fullTarBall, miniTarBall, cycle);

                sim->SaveIntoEps(savePictureFile);
                break;
            }
        }

    }
    if(sim->SimulationParameters.SaveEpsPicture) {
        pictTarBall.putFile(pictSaveFileName.c_str());
        unlink(pictSaveFileName.c_str());
    }
}

void SaveToFile(std::shared_ptr<CBaseSimCtrl> &contr,
                std::string &fullSaveFileName, std::string &miniSaveFileName,
                Tar &fullTarBall, Tar &miniTarBall,
                uint64_t cycle) {
    {
        std::fstream minimalFileStream((miniSaveFileName + std::to_string(cycle)).c_str(), std::ios_base::out);
        cereal::JSONOutputArchive minimalFileArchive(minimalFileStream);

        switch (contr->SimulationParameters.SaveParticlesInfo) {
            case true: {
                std::fstream mainFileStream((fullSaveFileName + std::to_string(cycle)).c_str(), std::ios_base::out);
                cereal::JSONOutputArchive mainFileArchive(mainFileStream);

                mainFileArchive(contr);
                contr->SimulationParameters.SaveParticlesInfo = false;
                minimalFileArchive(contr);
                contr->SimulationParameters.SaveParticlesInfo = true;
                break;
            }
            case false: {
                minimalFileArchive(contr);
                break;
            }
        }
    }

    switch (contr->SimulationParameters.SaveParticlesInfo) {
        case true:
            fullTarBall.putFile((fullSaveFileName + std::to_string(cycle)).c_str());
            unlink((fullSaveFileName + std::to_string(cycle)).c_str());

            miniTarBall.putFile((miniSaveFileName + std::to_string(cycle)).c_str());
            unlink((miniSaveFileName + std::to_string(cycle)).c_str());
            break;

        case false:
            miniTarBall.putFile((miniSaveFileName + std::to_string(cycle)).c_str());
            unlink((miniSaveFileName + std::to_string(cycle)).c_str());
            break;
    }
}
