#include <iostream>
#include <fstream>
#include <sstream>
#include <thread>

#include "CSimulationController.h"

void SimulateBySteps(std::string fname, CSimulationParameters data) {
    CSimulationController sim(data);

    std::string mainSaveFileName = "Results_" + fname;
    std::fstream mainSaveFileStream(mainSaveFileName.c_str(), std::fstream::out);
    cereal::JSONOutputArchive mainSaveFileArchieve(mainSaveFileStream);

    std::string pictureSaveFileName = "Picture_" + fname + ".eps";

    EPSPlot savePictureFile(pictureSaveFileName.c_str(), 0, 0, data.GetEpsDimensionX(), data.GetEpsDimensionY());

    sim.RunSimulation(mainSaveFileArchieve, savePictureFile);
}

void Simulate(std::string fname){

    std::fstream paramsFileStream(fname);

    cereal::JSONInputArchive paramsInptArchieve(paramsFileStream);
    CSimulationParameters data;
    paramsInptArchieve(data);

    SimulateBySteps(fname, data);

    paramsFileStream << "]";
}

int main(int argc, char** argv) {
    if(argc > 1) {
        Simulate(argv[1]);
    }
    return 0;
}