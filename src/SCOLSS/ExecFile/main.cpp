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

#include <SCOLSS/ExecFile/CTarball.h>

#include <SCOLSS/ExecFile/main.h>

int main(int argc, char **argv) {
    MPI::Init(argc, argv);
    std::cout.precision(10);
    std::cout << std::fixed;
    InitializeSimulations(argc, argv);

    std::cout << "exit\n";

    MPI::Finalize();
}

void InitializeSimulations(int argc, char **argv) {
    int p = MPI::COMM_WORLD.Get_size();
    int id = MPI::COMM_WORLD.Get_rank();

    std::string simType = argv[2];

    ESimulationType simT;
    if (simType == "MC") {
        simT = ESimulationType::MonteCarlo;
    } else if (simType == "LD") {
        simT = ESimulationType::LangevinDynamics;
    } else {
        std::cout << "Unknown simulation type " << simType << ". Exiting." << std::endl;
        return;
    }

    std::string simDataFileName = argv[1];

    if (id == p-1) {
        InitTar(simDataFileName);
    }
    std::string samples = argv[3];
    for (int i = 1; i <= std::stoi(samples); i++) {
        std::ifstream simDataFileStream(simDataFileName);

        cereal::JSONInputArchive simDataArchieve(simDataFileStream);

        std::shared_ptr<CBaseSimCtrl> sim;

        switch (simT) {
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

        SaveDataToFile(sim, fullSaveFileName, miniSaveFileName, simDataFileName, 0);

        RunSimulations(sim, fullSaveFileName, miniSaveFileName, pictSaveFileName, simDataFileName);

        printf("Sample N %i\n", i);
    }

    if (id == p-1) {
        FinishTar(simDataFileName);
    }
}

void RunSimulations(std::shared_ptr<CBaseSimCtrl> contr,
                    std::string &fullSaveFileName, std::string &miniSaveFileName, std::string &pictSaveFileName,
                    std::string &simDataFileName) {

    EPSPlot savePictureFile;

    InitEpsFile(contr, savePictureFile, pictSaveFileName);

    std::chrono::time_point<std::chrono::system_clock> start_time;
    start_time = std::chrono::system_clock::now();
    uint64_t prev_measure = 0;

    uint64_t totalCycles = contr->SimulationParameters.CyclesBetweenSaves * contr->SimulationParameters.NumberOfSavePoints;
    contr->SimulationParameters.NumberOfImageLines = std::min(totalCycles, contr->SimulationParameters.NumberOfImageLines);

    for (uint64_t cycle = 1; cycle <= totalCycles; ++cycle) {
        contr->DoCycle();

        if (0 == cycle % (contr->SimulationParameters.CyclesBetweenSaves)) {
            SaveDataToFile(contr, fullSaveFileName, miniSaveFileName, simDataFileName, cycle);
        }

        if (0 == cycle % (totalCycles / contr->SimulationParameters.NumberOfImageLines)) {
            SavePictureEps(contr, savePictureFile);
        }

        auto doFinish = contr->PrintTimeExtrapolation(start_time, prev_measure, totalCycles, cycle);
        if (doFinish) {
            SaveDataToFile(contr, fullSaveFileName, miniSaveFileName, simDataFileName, cycle);
            SavePictureEps(contr, savePictureFile);
            break;
        }
    }

    FinishEpsFile(contr, savePictureFile, pictSaveFileName, simDataFileName);
}

void InitEpsFile(std::shared_ptr<CBaseSimCtrl> & contr, EPSPlot& savePictureFile, std::string &pictSaveFileName) {
    int id = MPI::COMM_WORLD.Get_rank();
    if (id == contr->ManagerProcId && contr->SimulationParameters.SaveEpsPicture) {
        savePictureFile.Init(pictSaveFileName.c_str(),
                             0,
                             0,
                             contr->SimulationParameters.GetEpsDimensionX(),
                             contr->SimulationParameters.GetEpsDimensionY());

        savePictureFile.initParticleSavings(contr->SimulationParameters.ParticleDiameter);

        contr->SaveIntoEps(savePictureFile);
    }
}

void FinishEpsFile(std::shared_ptr<CBaseSimCtrl> & contr, EPSPlot& savePictureFile,
                   std::string &pictSaveFileName,
                   std::string &simDataFileName) {
    int id = MPI::COMM_WORLD.Get_rank();

    if (id == contr->ManagerProcId && contr->SimulationParameters.SaveEpsPicture) {
        savePictureFile.Close();

        std::fstream pictSaveTarStream("PictData_" + simDataFileName + ".tar", std::ios::app | std::ios::out);

        Tar pictTarBall(pictSaveTarStream);
        if (contr->SimulationParameters.SaveEpsPicture) {
            pictTarBall.putFile(pictSaveFileName.c_str());
//            unlink(pictSaveFileName.c_str());
        }
    }
}

void SavePictureEps(std::shared_ptr<CBaseSimCtrl> & contr, EPSPlot& savePictureFile) {
    int id = MPI::COMM_WORLD.Get_rank();

    contr->SyncBeforeSave();
    if(contr->SimulationParameters.SaveEpsPicture && id == contr->ManagerProcId) {
        contr->SaveIntoEps(savePictureFile);
    }
}

void SaveDataToFile(std::shared_ptr<CBaseSimCtrl> &contr,
                    std::string &fullSaveFileName,
                    std::string &miniSaveFileName,
                    std::string &simDataFileName,
                    uint64_t cycle) {
    int id = MPI::COMM_WORLD.Get_rank();

    contr->SyncBeforeSave();

    if (id == contr->ManagerProcId) {
        std::fstream fullSaveTarStream("FullData_" + simDataFileName + ".tar", std::ios::app | std::ios::out);
        std::fstream miniSaveTarStream("MiniData_" + simDataFileName + ".tar", std::ios::app | std::ios::out);

        Tar fullTarBall(fullSaveTarStream);
        Tar miniTarBall(miniSaveTarStream);

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
}

void InitTar(std::string &simDataFileName) {
    std::fstream fullSaveTarStream("FullData_" + simDataFileName + ".tar", std::ios_base::out);
    std::fstream miniSaveTarStream("MiniData_" + simDataFileName + ".tar", std::ios_base::out);
    std::fstream pictSaveTarStream("PictData_" + simDataFileName + ".tar", std::ios_base::out);

    Tar fullTarBall(fullSaveTarStream);
    Tar miniTarBall(miniSaveTarStream);
    Tar pictTarBall(pictSaveTarStream);
}

void FinishTar(std::string &simDataFileName) {
    std::fstream fullSaveTarStream("FullData_" + simDataFileName + ".tar", std::ios::app | std::ios::out);
    std::fstream miniSaveTarStream("MiniData_" + simDataFileName + ".tar", std::ios::app | std::ios::out);
    std::fstream pictSaveTarStream("PictData_" + simDataFileName + ".tar", std::ios::app | std::ios::out);

    Tar fullTarBall(fullSaveTarStream);
    Tar miniTarBall(miniSaveTarStream);
    Tar pictTarBall(pictSaveTarStream);

    fullTarBall.finish();
    miniTarBall.finish();
    pictTarBall.finish();
}
