//
// Created by mpolovyi on 22/01/16.
//

#include <iostream>
#include <SCOLSS/MathLibrary/CVector.h>

#include <SCOLSS/SimulationController/CLangevinSimCtrl.h>
#include <SCOLSS/SimulationController/CMonteCarloSimParams.h>
#include <SCOLSS/SimulationController/CMonteCarloSimCtrl.h>

void RunSimulations(int argc, char **argv);

int main(int argc, char **argv) {
    RunSimulations(argc, argv);
}

void RunSimulations(int argc, char **argv) {
    std::string samples = argv[2];

    for(int i = 1; i <= std::stoi(samples); i++) {
        std::string fname = argv[1];
        std::ifstream simDataFileStream(fname);

        cereal::JSONInputArchive simDataArchieve(simDataFileStream);

        CMonteCarloSimParams data;

        simDataArchieve(data);

        CMonteCarloSimCtrl sim(data);

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