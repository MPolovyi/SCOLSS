//#include "Quaternion.h"

#include <iostream>
#include <math.h>

#include <fstream>
#include <cereal/archives/json.hpp>
#include <cereal/types/vector.hpp>
#include <iomanip>
#include <chrono>

#include "CSimulationController.h"

void RunMCSimulations(int argc, char **argv);

int main(int argc, char **argv) {
    RunMCSimulations(argc, argv);
}

void RunMCSimulations(int argc, char **argv) {
    std::string samples = argv[2];

    for(int i = 1; i <= std::stoi(samples); i++) {
        std::string fname = argv[1];
        std::ifstream simDataFileStream(fname);

        cereal::JSONInputArchive simDataArchieve(simDataFileStream);

        CSimulationParameters data;

        simDataArchieve(data);

        CSimulationController sim(data);

        std::stringstream ssMainSaveFileName;

        ssMainSaveFileName << "Results_" << i << fname;

        std::string mainSaveFileName = ssMainSaveFileName.str();
        std::fstream mainSaveFileStream(mainSaveFileName.c_str(), std::fstream::out);
        cereal::JSONOutputArchive mainSaveFileArchieve(mainSaveFileStream);

        std::string pictureSaveFileName = "Picture_" + fname + ".eps";

        EPSPlot savePictureFile(pictureSaveFileName.c_str(), 0, 0, data.GetEpsDimensionX(), data.GetEpsDimensionY());

        sim.RunSimulation(mainSaveFileArchieve, savePictureFile);
        std::cout << i << std::endl;
    }
}
