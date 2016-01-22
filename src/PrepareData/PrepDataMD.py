import json
import os
import shutil as sh
import sys

__author__ = 'mpolovyi'


def populateData(simData, run_all_file_lines):
    data = raw_input("KbT = ")
    if len(data) != 0:
        simData["KbT"] = eval(data)

    data = raw_input("Particle count = ")
    if len(data) != 0:
        simData["PtCount"] = eval(data)

    data = raw_input("Density = ")
    if len(data) != 0:
        simData["Density"] = eval(data)

    data = raw_input("InitialConfiguration = ")
    if len(data) != 0:
        simData["InitialConfiguration"] = int(data)

    print(str(simData))

    data = ""

    while len(data) == 0:
        data = raw_input("Use data? (y/n) ")

        if len(data) != 0:
            if data[0] == "n":
                return
            if data[0] == "y":
                continue

    data = raw_input("How many samples? ")
    samplesCount = 1
    if len(data) != 0:
        samplesCount = int(data)

    for ptc in simData["PtCount"]:
            for rho in simData["Density"]:
                for kbt in simData["KbT"]:
                    simDataToSave = {"value0": dict(simData)}
                    simDataToSave["value0"]["KbT"] = kbt
                    simDataToSave["value0"]["PtCount"] = ptc
                    simDataToSave["value0"]["Density"] = rho

                    folder_name = ""
                    for key in simDataToSave["value0"].keys():
                        if key[:1] != "_":
                            folder_name += (key[:1] + "_" + str(simDataToSave["value0"][key]) + "_")

                    if not os.path.exists(folder_name):
                        os.makedirs(folder_name)
                    else:
                        pass

                    sh.copy("ColloidMD", folder_name + "/ColloidMD")


    for index in range(0, samplesCount):
        for ptc in simData["PtCount"]:
            for rho in simData["Density"]:
                for kbt in simData["KbT"]:
                    simDataToSave = {"value0": dict(simData)}
                    simDataToSave["value0"]["KbT"] = kbt
                    simDataToSave["value0"]["PtCount"] = ptc
                    simDataToSave["value0"]["Density"] = rho

                    folder_name = ""
                    for key in simDataToSave["value0"].keys():
                        if key[:1] != "_":
                            folder_name += (key[:1] + "_" + str(simDataToSave["value0"][key]) + "_")

                    run_index_string = str(index)

                    run_file_lines = ["#$ -S //bin//sh \n",
                                      "#$ -j y \n",
                                      "#$ -m bes \n",
                                      "#$ -M max.polovyi@gmail.com \n",
                                      "#$ -V \n",
                                      "#$ -cwd \n",
                                      "#$ -l virtual_free=500M -l h_vmem=800M \n",
                                      "#$ -q SHORT\n",
                                      "\n",
                                      "cp $SGE_O_WORKDIR//ColloidMD $TMPDIR\n",
                                      "cp $SGE_O_WORKDIR//Data_" + run_index_string + ".json" + " $TMPDIR\n",
                                      "\n",
                                      "cd $TMPDIR\n",
                                      "(time .//ColloidMD Data_" + run_index_string + ".json" + ") >&time_" + run_index_string + ".txt\n",
                                      "cp * $SGE_O_WORKDIR//\n",
                                      "rm *\n"]

                    with open("r"+run_index_string, "w") as run_file:
                        run_file.writelines(run_file_lines)

                    run_file_name = "DT"+str(round(simDataToSave["value0"]["KbT"], 2))+"_N"+str(simDataToSave["value0"]["PtCount"]) + "r"+run_index_string
                    sh.move("r"+run_index_string, folder_name + "/" + run_file_name)
                    run_all_file_lines.append([folder_name, run_file_name])

                    with open("Data0.json", "w") as outfile:
                        json.dump(simDataToSave, outfile, sort_keys=True, indent=4, separators=(',', ': '))

                    fname = "Data_" + run_index_string + ".json"
                    sh.copy("Data0.json", folder_name + "/" + fname)

simData = {
              "Density": 0.95,
              "TimeBetweenSaves": 1,
              "KbT": 2,
              "LoadSavedState": 0,
              "NumberOfImageLines": 1,
              "NumberOfSavePoints": 100,
              "PtCount": 800,
              "InitialConfiguration": 0,
              "SavedParticles": ""
            }

run_all_file_lines = []
populateData(simData, run_all_file_lines)

data = ""

while len(data) == 0:
    data = raw_input("Do more? (y/n) ")
    if len(data) != 0:
        if data[0] == "n":
            break
        if data[0] == "y":
            populateData(simData, run_all_file_lines)
            data = ""

with open("run_all.sh", "w") as run_file:
    for run_line in run_all_file_lines:
        run_file.write("cd " + run_line[0] + "\n")
        run_file.write("qsub " + run_line[1] + "\n")
        run_file.write("cd .. \n")

os.chmod("run_all.sh", 0750)
