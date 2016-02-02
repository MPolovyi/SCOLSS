import json
import os
import shutil as sh
import sys

__author__ = 'mpolovyi'


def populateData(simData, run_all_file_lines):
    data = raw_input("KbT = ")
    if len(data) != 0:
        simData["Base"]["KbT"] = eval(data)

    data = raw_input("Particle count = ")
    if len(data) != 0:
        simData["Base"]["PtCount"] = eval(data)

    data = raw_input("Density = ")
    if len(data) != 0:
        simData["Base"]["Density"] = eval(data)

    data = raw_input("InitialConfiguration = ")
    if len(data) != 0:
        simData["Base"]["InitialConfiguration"] = eval(data)

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

    data = raw_input("How many samples per run? ")
    samplesPerRunCount = 1
    if len(data) != 0:
        samplesPerRunCount = int(data)

    for ptc in simData["Base"]["PtCount"]:
            for rho in simData["Base"]["Density"]:
                for kbt in simData["Base"]["KbT"]:
                    for ic in simData["Base"]["InitialConfiguration"]:
                        simDataToSave = {"value0": dict(simData)}
                        simDataToSave["value0"]["Base"] = dict(simData["Base"])
                        simDataToSave["value0"]["Base"]["KbT"] = kbt
                        simDataToSave["value0"]["Base"]["PtCount"] = ptc
                        simDataToSave["value0"]["Base"]["Density"] = rho
                        simDataToSave["value0"]["Base"]["InitialConfiguration"] = ic

                        folder_name = "T_" + str(simDataToSave["value0"]["TimeBetweenSaves"]) +\
                                      "_I_" + str(simDataToSave["value0"]["Base"]["InitialConfiguration"]) + \
                                      "_K_" + str(kbt) + "_P_" + str(ptc) + "_D_" + str(rho)

                        if not os.path.exists(folder_name):
                            os.makedirs(folder_name)
                        else:
                            pass

                        sh.copy("ExecFile", folder_name + "/ExecFile")


    for index in range(0, samplesCount):
        for ptc in simData["Base"]["PtCount"]:
            for rho in simData["Base"]["Density"]:
                for kbt in simData["Base"]["KbT"]:
                    for ic in simData["Base"]["InitialConfiguration"]:
                        simDataToSave = {"value0": dict(simData)}
                        simDataToSave["value0"]["Base"] = dict(simData["Base"])
                        simDataToSave["value0"]["Base"]["KbT"] = kbt
                        simDataToSave["value0"]["Base"]["PtCount"] = ptc
                        simDataToSave["value0"]["Base"]["Density"] = rho
                        simDataToSave["value0"]["Base"]["InitialConfiguration"] = ic

                        folder_name = "T_" + str(simDataToSave["value0"]["TimeBetweenSaves"]) +\
                                      "_I_" + str(simDataToSave["value0"]["Base"]["InitialConfiguration"]) + \
                                      "_K_" + str(kbt) + "_P_" + str(ptc) + "_D_" + str(rho)

                        run_index_string = str(index)

                        run_file_lines = ["#$ -S /bin/sh \n",
                                          "#$ -j y \n",
                                          "#$ -m eas \n",
                                          "#$ -cwd \n",
                                          "#$ -l virtual_free=800M -l h_vmem=800M \n",
                                          "#$ -q SHORT\n",
                                          "\n",
                                          "cp $SGE_O_WORKDIR/ExecFile $TMPDIR\n",
                                          "cp $SGE_O_WORKDIR/Data_" + run_index_string + ".json" + " $TMPDIR\n",
                                          "\n",
                                          "cd $TMPDIR\n",
                                          "(time ./ExecFile Data_" + run_index_string + ".json" + " LD " + str(samplesPerRunCount) + ") >&time_" + run_index_string + ".txt\n",
                                          "find . -type f -name \"Resul*.json*\" > include-file\n",
                                          "find . -type f -name \"Picture*.eps*\" >> include-file\n",
                                          "tar -cpf MD_dipole_" + run_index_string + ".tar -T include-file\n",

                                          "cp MD_dipole_" + run_index_string + ".tar $SGE_O_WORKDIR/\n",
                                          "rm *\n",
                                          "cd $SGE_O_WORKDIR\n",
                                          "tar -xf MD_dipole_" + run_index_string + ".tar --wildcards --no-anchored '*json*'\n"]

                        with open("r"+run_index_string, "w") as run_file:
                            run_file.writelines(run_file_lines)

                        run_file_name = "DT"+str(round(simDataToSave["value0"]["Base"]["KbT"], 2))+"_N"+str(simDataToSave["value0"]["Base"]["PtCount"]) + "r"+run_index_string
                        sh.move("r"+run_index_string, folder_name + "/" + run_file_name)
                        run_all_file_lines.append([folder_name, run_file_name])

                        with open("Data0.json", "w") as outfile:
                            json.dump(simDataToSave, outfile, sort_keys=True, indent=4, separators=(',', ': '))

                        fname = "Data_" + run_index_string + ".json"
                        sh.copy("Data0.json", folder_name + "/" + fname)

simData = {
    "Base": {
        "Density": [0.25, 0.5, 0.75],
        "InitialConfiguration": [0, 1],
        "KbT": [0.4, 1, 1.5, 2],
        "LoadSavedState": 0,
        "NumberOfSavePoints": 20,
        "NumberOfImageLines": 1000,
        "PtCount": [5000],
        "SavedParticles": ""
    },
    "CyclesBetweenSaves": 200000,
   "TimeBetweenSaves": 0.5 
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
