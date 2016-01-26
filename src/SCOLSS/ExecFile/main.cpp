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

void RunSimulations(int argc, char **argv);
void RunSimulations(std::shared_ptr<CBaseSimCtrl> contr, cereal::JSONOutputArchive &outputArchive, EPSPlot &pictureSaver);

int main(int argc, char **argv) {
    RunSimulations(argc, argv);
}

void RunSimulations(int argc, char **argv) {
    std::string simType = argv[2];
    ESimulationType simT = (ESimulationType)std::stoi(simType);

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
                sim = std::make_shared<CBaseSimCtrl>(CMonteCarloSimCtrl(data));
                break;
            };
            case LangevinDynamics: {
                CLangevinSimParams data;
                simDataArchieve(data);
                sim = std::make_shared<CBaseSimCtrl>(CLangevinSimCtrl(data));
                break;
            };
        }

        std::stringstream ssMainSaveFileName;

        ssMainSaveFileName << "Results_" << i << fname;

        std::string mainSaveFileName = ssMainSaveFileName.str();
        std::fstream mainSaveFileStream(mainSaveFileName.c_str(), std::fstream::out);
        cereal::JSONOutputArchive mainSaveFileArchieve(mainSaveFileStream);

        std::string pictureSaveFileName = "Picture_" + fname + ".eps";

        EPSPlot savePictureFile(pictureSaveFileName.c_str(),
                                0,
                                0,
                                sim->GetSimulationParameters().GetEpsDimensionX(),
                                sim->GetSimulationParameters().GetEpsDimensionY());

        RunSimulations(sim, mainSaveFileArchieve, savePictureFile);
        std::cout << i << std::endl;
    }
}

void RunSimulations(std::shared_ptr<CBaseSimCtrl> contr, cereal::JSONOutputArchive &outputArchive, EPSPlot &pictureSaver) {
    outputArchive.makeArray();

    outputArchive(contr);

    contr->SaveIntoEps(pictureSaver);
    std::chrono::time_point<std::chrono::system_clock> start_time, step_time;
    start_time = std::chrono::system_clock::now();
    uint64_t prev_measure = 0;

    uint64_t totalCycles = contr->GetSimulationParameters().CyclesBetweenSaves * contr->GetSimulationParameters().NumberOfSavePoints;

    size_t k = 1;
    for (uint64_t cycle = 1; cycle <= totalCycles; ++cycle) {
        contr->DoCycle();

        if (0 == cycle % (contr->GetSimulationParameters().CyclesBetweenSaves)) {
            outputArchive(contr);
        }

        if (0 == cycle % (totalCycles / contr->GetSimulationParameters().NumberOfImageLines)) {
            contr->SaveIntoEps(pictureSaver);
        }

        auto doFinish = contr->PrintTimeExtrapolation(start_time, prev_measure, totalCycles, cycle);
        if (doFinish) {
            outputArchive(contr);
            contr->SaveIntoEps(pictureSaver);
            break;
        }
    }
}
