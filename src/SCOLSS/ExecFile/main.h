//
// Created by mpolovyi on 11/03/16.
//

#ifndef PROJECT_MAIN_H_H
#define PROJECT_MAIN_H_H
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

int main(int argc, char **argv);

void InitializeSimulations(int argc, char **argv) ;

void RunSimulations(std::shared_ptr<CBaseSimCtrl> contr,
                    std::string &fullSaveFileName, std::string &miniSaveFileName, std::string &pictSaveFileName,
                    std::string &simDataFileName) ;

void InitEpsFile(std::shared_ptr<CBaseSimCtrl> & contr, EPSPlot& savePictureFile, std::string &pictSaveFileName) ;

void FinishEpsFile(std::shared_ptr<CBaseSimCtrl> & contr, EPSPlot& savePictureFile,
                   std::string &pictSaveFileName,
                   std::string &simDataFileName) ;

void SavePictureEps(std::shared_ptr<CBaseSimCtrl> & contr, EPSPlot& savePictureFile) ;

void SaveDataToFile(std::shared_ptr<CBaseSimCtrl> &contr,
                    std::string &fullSaveFileName,
                    std::string &miniSaveFileName,
                    std::string &simDataFileName,
                    uint64_t cycle) ;

void InitTar(std::string &simDataFileName) ;

void FinishTar(std::string &simDataFileName) ;

#endif //PROJECT_MAIN_H_H
