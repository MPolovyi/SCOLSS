import json
import os
import shutil as sh
import xml.etree.ElementTree
import time


__author__ = 'mpolovyi'

simData = {
    "Density": 0.5,
    "EquilibriumCycle": 100000,
    "NumberOfSavePoints": 10,
    "NumberOfImageLines": 1000,
    "KbT": 0.5,
    "PtCount": 500,
    "LoadSavedState": 0,
    "SavedParticles": ""
}


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

                    sh.copy("ColloidMC", folder_name + "/ColloidMC")


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
                                      "#$ -q LONG \n",
                                      "\n",
                                      "cp $SGE_O_WORKDIR//ColloidMC $TMPDIR\n",
                                      "cp $SGE_O_WORKDIR//Data_" + run_index_string + ".json" + " $TMPDIR\n",
                                      "\n",
                                      "cd $TMPDIR\n",
                                      # "(time .//ColloidMC Data_" + run_index_string + ".json" + ") >&time_" + run_index_string + ".txt\n",
                                      "cp * $SGE_O_WORKDIR//\n",
                                      "rm *\n"]

                    with open("r"+run_index_string, "w") as run_file:
                        run_file.writelines(run_file_lines)

                    run_file_name = "CT"+str(round(simDataToSave["value0"]["KbT"], 2))+"_N"+str(simDataToSave["value0"]["PtCount"]) + "r"+run_index_string
                    sh.move("r"+run_index_string, folder_name + "//" + run_file_name)
                    run_all_file_lines.append([folder_name, run_file_name])

                    with open("Data0.json", "w") as outfile:
                        json.dump(simDataToSave, outfile, sort_keys=True, indent=4, separators=(',', ': '))

                    fname = "Data_" + run_index_string + ".json"
                    sh.copy("Data0.json", folder_name + "/" + fname)


def runInChuncks(run_all_lines, max_qued = 1500):
    os.system("qstat | wc -l > qstat.txt")

    with open("qstat.txt") as qstat_file:
        qstat_tasks_count = eval(qstat_file.readline())

    start_index = 0
    end_index = len(run_all_lines)

    while start_index != end_index:
        with open("run_chunk.sh", "w") as run_file:
            for index in xrange(start_index, min(max_qued - qstat_tasks_count, end_index)):
                run_line = run_all_lines[index]
                run_file.write("cd " + run_line[0] + "\n")
                run_file.write("pwd \n")
                # run_file.write("qsub " + run_line[1] + "\n")
                run_file.write("cd .. \n")

        os.chmod("run_chunk.sh", 0750)
        os.system("./run_chunk.sh")
        start_index = min(max_qued - qstat_tasks_count, end_index)
        time.sleep(300)

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

data = ""
while len(data) == 0:
    data = raw_input("Run all? (y/n) ")
    if len(data) != 0:
        if data[0] == "y":
            os.system("./run_all.sh")
        if data[0] == "n":
            runInChuncks(run_all_file_lines)
