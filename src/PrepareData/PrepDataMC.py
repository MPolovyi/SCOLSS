#!/usr/bin/env python

import json
import os
import sh
import shutil
import sys
import time
import datetime
import MiscFunctions
import uuid

__author__ = 'mpolovyi'

simData = {
    "Base": {
        "Density": [0.1],
        "InitialConfiguration": [2],
        "KbT": [0.1],
        "LoadSavedState": 0,
        "NumberOfSavePoints": 200,
        "NumberOfImageLines": 1000,
        "PtCount": [100],
        "SavedParticles": "",
        "SaveParticlesInfo": False,
        "SaveEpsPicture": False
    },
    "CyclesBetweenSaves": 200,
    "TimeBetweenSaves": 0.05,
    "Queue": "SHORT"
}


def prompt_for_data(sim_data):
    if "-i" in sys.argv:
        data = raw_input("KbT = ")
        if len(data) != 0:
            sim_data["Base"]["KbT"] = eval(data)

        data = raw_input("Particle count = ")
        if len(data) != 0:
            sim_data["Base"]["PtCount"] = eval(data)

        data = raw_input("Density = ")
        if len(data) != 0:
            sim_data["Base"]["Density"] = eval(data)

        data = raw_input("InitialConfiguration = ")
        if len(data) != 0:
            sim_data["Base"]["InitialConfiguration"] = eval(data)

    print(str(sim_data))

    data = ""
    while len(data) == 0:
        data = raw_input("Use data? (y/n) ")
        if len(data) != 0:
            if data[0] == "n":
                raise ValueError("Don't do anything, exiting", True)
            if data[0] == "y":
                continue

    data = raw_input("How many samples? ")
    samples_count = 1
    if len(data) != 0:
        samples_count = int(data)

    data = raw_input("How many samples per run? ")
    samples_per_run_count = 1
    if len(data) != 0:
        samples_per_run_count = int(data)

    return samples_count, samples_per_run_count


def iterate_over_folders(sim_data, func_list, *args):
    ret = []

    for ptc in sim_data["Base"]["PtCount"]:
        for rho in sim_data["Base"]["Density"]:
            for kbt in sim_data["Base"]["KbT"]:
                for ic in sim_data["Base"]["InitialConfiguration"]:
                    sim_data_to_save = {"value0": dict(sim_data)}
                    sim_data_to_save["value0"]["Base"] = dict(sim_data["Base"])
                    sim_data_to_save["value0"]["Base"]["KbT"] = kbt
                    sim_data_to_save["value0"]["Base"]["PtCount"] = ptc
                    sim_data_to_save["value0"]["Base"]["Density"] = rho
                    sim_data_to_save["value0"]["Base"]["InitialConfiguration"] = ic

                    folder_name = "C_{0:d}_I_{1:d}_K_{2:.2f}_P_{3:d}_D_{4:.2f}".format(
                        sim_data_to_save["value0"]["CyclesBetweenSaves"],
                        sim_data_to_save["value0"]["Base"]["InitialConfiguration"], kbt, ptc, rho)

                    for func in func_list:
                        ret.append(func(folder_name, sim_data_to_save, args))

    return ret


def copy_exec_file(*args):
    folder_name = args[0]
    if os.path.exists(folder_name):
        shutil.copy("ExecFile", folder_name + "/ExecFile")
    else:
        os.makedirs(folder_name)
        shutil.copy("ExecFile", folder_name + "/ExecFile")


class CreateSampleRunFiles:
    def __init__(self, samples_per_run_count, run_all_file_lines):
        self.Index = 1
        self.SamplesPerRun = samples_per_run_count
        self.RunAllFileLines = run_all_file_lines

    def __call__(self, *args):
        folder_name = args[0]
        sim_data_to_save = args[1]

        run_index_string = str(self.Index)

        run_file_lines = ('\n'
                          '#$ -S /bin/sh\n'
                          '#$ -j y\n'
                          '#$ -m eas\n'
                          '#$ -cwd\n'
                          '#$ -l virtual_free=800M -l h_vmem=800M\n'
                          '#$ -q {3}\n'
                          '\n'
                          'cp $SGE_O_WORKDIR/ExecFile $TMPDIR\n'
                          'cp $SGE_O_WORKDIR/Data_{0}.json $TMPDIR\n'
                          '\n'
                          'cd $TMPDIR\n'
                          '(time ./ExecFile Data_{0}.json MC {1}) >&time_{2}.txt\n'
                          'tar -czpf MC_dipole_Mini_{0}.tar.gz -T minInclude\n'
                          'tar -czpf MC_dipole_Full_{0}.tar.gz -T fullInclude\n'
                          'tar -czpf MC_dipole_Pict_{0}.tar.gz -T picInclude\n'
                          'tar -cpf MC_dipole_{0}.tar MC_dipole_Pict_{0}.tar.gz'
                          ' MC_dipole_Full_{0}.tar.gz MC_dipole_Mini_{0}.tar.gz\n'
                          '\n'
                          'cp MC_dipole_{0}.tar $SGE_O_WORKDIR/\n'
                          'rm *\n'
                          'cd $SGE_O_WORKDIR\n'
                          'tar -xf MC_dipole_{0}.tar\n').format(run_index_string, str(self.SamplesPerRun), run_index_string, simData["Queue"])

        with open("r{0}".format(run_index_string), "w") as run_file:
            run_file.write(run_file_lines)

        run_file_name = "CT{0:.2f}_N{1:d}r{2:s}".format(sim_data_to_save["value0"]["Base"]["KbT"],
                                                        sim_data_to_save["value0"]["Base"]["PtCount"],
                                                        run_index_string)
        shutil.move("r{0}".format(run_index_string), os.path.join(folder_name, run_file_name))
        self.RunAllFileLines.append([folder_name, run_file_name])

        with open("Data0.json", "w") as outfile:
            json.dump(sim_data_to_save, outfile, sort_keys=True, indent=4, separators=(',', ': '))

        fname = "Data_{0}.json".format(run_index_string)
        shutil.move("Data0.json", folder_name + "/" + fname)


def populate_data(sim_data, run_all_file_lines):
    samplesCount, samplesPerRunCount = (1, 1)
    try:
        samplesCount, samplesPerRunCount = prompt_for_data(sim_data)
    except ValueError as err:
        print(err[0])
        if err[1]:
            return

    iterate_over_folders(sim_data, [copy_exec_file])

    sample_creator = CreateSampleRunFiles(samplesPerRunCount, run_all_file_lines)

    for index in range(0, samplesCount):
        sample_creator.Index = index

        iterate_over_folders(sim_data, [sample_creator])


def get_tasks_on_queue(match=None):
    qstat = sh.Command("qstat")
    wc = sh.Command("wc")
    grep = sh.Command("grep")

    if match is None:
        in_queue = int(wc(grep(qstat(), "-E", ' r| qw'), "-l"))
        return in_queue
    else:
        in_queue = int(wc(grep(qstat(), match), "-l"))
        return in_queue


def submit_to_queue(run_all_file_lines, max_on_queue=600):
    qsub = sh.Command("qsub")
    for run_line in run_all_file_lines:
        with MiscFunctions.cd(os.path.join("./", run_line[0])):
            last_submitted = qsub(run_line[1])
            print last_submitted
            time.sleep(5)

        while get_tasks_on_queue() > max_on_queue:
            print "{0} tasks on queue is greater then maximum allowed {1}," \
                  " waiting 20 min from {2}".format(get_tasks_on_queue(), max_on_queue, datetime.datetime.now().time())
            time.sleep(20 * 60)

    return int(last_submitted.split(" ")[2])


def create_task_to_archive_results_from_folder(*args):
    folder_name = args[0]

    print "Writing results form folder {0} to include file".format(folder_name)

    picsFile = open("SimulationPictures_{0}".format(folder_name), "w")
    fullFile = open("SimulationFullData_{0}".format(folder_name), "w")
    miniFile = open("SimulationMiniData_{0}".format(folder_name), "w")

    for root, dirs, files in os.walk(os.path.join(os.getcwd(), folder_name)):
        for file_name in files:
            if "tar" in file_name:
                if "Pics" in file_name:
                    picsFile.write(os.path.join(folder_name, file_name))
                if "Full" in file_name:
                    fullFile.write(os.path.join(folder_name, file_name))
                if "Mini" in file_name:
                    miniFile.write(os.path.join(folder_name, file_name))

    picsFile.close()
    fullFile.close()
    miniFile.close()

    run_file_lines = ('\n'
                      '#$ -S /bin/sh\n'
                      '#$ -j y\n'
                      '#$ -m eas\n'
                      '#$ -cwd\n'
                      '#$ -l virtual_free=800M -l h_vmem=800M\n'
                      '#$ -q SHORT\n'
                      '\n'
                      'tar -cpf MC_Pics_{0}.tar -T {1}\n'
                      'tar -cpf MC_Full_{0}.tar -T {2}\n'
                      'tar -cpf MC_Mini_{0}.tar -T {3}\n').format(folder_name, picsFile, fullFile, miniFile)

    with open("SaveData_".format(folder_name)) as sub_file:
        sub_file.write(run_file_lines)

    return os.getcwd(), "SaveData_".format(folder_name)


def archived_mini_file_names(*args):
    folder_name = args[0]
    return "SaveData_".format(folder_name)


def save_data(last_submitted, sim_data):
    qstat = sh.Command("qstat")
    grep = sh.Command("grep")

    are_on_queue = True

    while are_on_queue:
        try:
            grep(qstat(), last_submitted)
            print "{0} submitted last still on queue," \
                  " waiting 20 min from {1}".format(last_submitted, datetime.datetime.now().time())
            time.sleep(20 * 60)
        except:
            are_on_queue = False

    save_all_file_lines = iterate_over_folders(sim_data, [create_task_to_archive_results_from_folder])

    submit_to_queue(save_all_file_lines)


def create_data_saved_file(tar_file_names):
    if os.path.exists("data_saved"):
        print "Previous saved data exists, download it! \n" \
              " waiting 20 min from {0}".format(datetime.datetime.now().time())
        time.sleep(20 * 60)
    else:
        with open("data_saved.tmp", "w") as data_saved:
            for name in tar_file_names:
                data_saved.write("scp grace:" + name + "\n")
                data_saved.write("tar -xf" + name.split("/")[len(name.split("/"))-1] + "\n")

        shutil.move("data_saved.tmp", "data_saved")


if __name__ == "__main__":
    run_all_file_lines = []
    populate_data(simData, run_all_file_lines)

    data = ""

    last_submitted = submit_to_queue(run_all_file_lines)
    tar_file_names = save_data(last_submitted, simData)
    create_data_saved_file(tar_file_names)
