#!/usr/bin/env python

import json
import os
import sh
import shutil
import sys
import time
import datetime
import MiscFunctions

__author__ = 'mpolovyi'

simData = {
    "Base": {
        "Density": [0.25, 0.5, 0.75],
        "InitialConfiguration": [0],
        "KbT": [0.6, 1.2, 1.8],
        "LoadSavedState": 0,
        "NumberOfSavePoints": 100000,
        "NumberOfImageLines": 1,
        "PtCount": [3200],
        "SavedParticles": "",
        "SaveParticlesInfo": True,
        "SaveEpsPicture": False
    },
    "CyclesBetweenSaves": 200,
    "TimeBetweenSaves": 0.25,
    "Queue": "LONG"
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

    print(json.dumps(sim_data, indent=2, sort_keys=True))

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
                    sim_data_to_save = dict(sim_data)
                    sim_data_to_save["Base"] = dict(sim_data["Base"])
                    sim_data_to_save["Base"]["KbT"] = kbt
                    sim_data_to_save["Base"]["PtCount"] = ptc
                    sim_data_to_save["Base"]["Density"] = rho
                    sim_data_to_save["Base"]["InitialConfiguration"] = ic

                    folder_name = "T_{0:.2f}_I_{1:d}_K_{2:.2f}_P_{3:d}_D_{4:.2f}".format(
                        sim_data_to_save["TimeBetweenSaves"],
                        sim_data_to_save["Base"]["InitialConfiguration"], kbt, ptc, rho)

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

        tar_files = "MiniData_Data_{0}.json.tar"

        if sim_data_to_save["Base"]["SaveParticlesInfo"]:
            tar_files += " FullData_Data_{0}.json.tar"

        if sim_data_to_save["Base"]["SaveEpsPicture"]:
            tar_files += " PictData_Data_{0}.json.tar"

        run_file_lines = ('#$ -S /bin/sh\n'
                          '#$ -j y\n'
                          '#$ -m eas\n'
                          '#$ -cwd\n'
                          '#$ -l virtual_free=800M -l h_vmem=800M\n'
                          '#$ -q {2}\n'
                          '\n'
                          'cp $SGE_O_WORKDIR/ExecFile $TMPDIR\n'
                          'cp $SGE_O_WORKDIR/Data_{0}.json $TMPDIR\n'
                          '\n'
                          'cd $TMPDIR\n'
                          '(time ./ExecFile Data_{0}.json LD {1}) >&time_{0}.txt\n'
                          'tar -cpf Data_Sample_{0}.tar {3}\n'
                          '\n'
                          'cp Data_Sample_{0}.tar $SGE_O_WORKDIR/\n'
                          'rm *\n'
                          'cd $SGE_O_WORKDIR\n'
                          'tar -xf Data_Sample_{0}.tar\n').format("{0}", "{1}", "{2}", tar_files).format(run_index_string, str(self.SamplesPerRun), simData["Queue"])

        with open("r{0}".format(run_index_string), "w") as run_file:
            run_file.write(run_file_lines)

        run_file_name = "DT{0:.2f}_N{1:d}r{2:s}".format(sim_data_to_save["Base"]["KbT"],
                                                        sim_data_to_save["Base"]["PtCount"],
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
        print(json.dumps(err[0], indent=2, sort_keys=True))
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
        try:
            in_queue = int(wc(grep(qstat(), "-E", ' r| qw'), "-l"))
        except:
            return 0
        return in_queue
    else:
        try:
            in_queue = int(wc(grep(qstat(), match), "-l"))
        except:
            return 0
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
    sim_data = args[1]

    print "Writing results form folder {0} to include file".format(folder_name)

    tar_save_lines = 'tar -cpf MD_Mini_{0}.tar ./{0}/MiniData*.tar\n'

    if sim_data["Base"]["SaveParticlesInfo"]:
        tar_save_lines += ' tar -czpf MD_Full_{0}.tar.gz ./{0}/FullData*.tar\n'

    if sim_data["Base"]["SaveEpsPicture"]:
        tar_save_lines += ' tar -cpf MD_Pics_{0}.tar.gz ./{0}/PicsData*.tar\n'

    run_file_lines = ('\n'
                      '#$ -S /bin/sh\n'
                      '#$ -j y\n'
                      '#$ -m eas\n'
                      '#$ -cwd\n'
                      '#$ -l virtual_free=800M -l h_vmem=800M\n'
                      '#$ -q SHORT\n'
                      '\n'
                      '{0}\n'
                      'rm SaveData_{1}*').format(tar_save_lines, folder_name).format(folder_name)

    with open("SaveData_{0}".format(folder_name), "w") as sub_file:
        sub_file.write(run_file_lines)

    return os.getcwd(), "SaveData_{0}".format(folder_name)


def save_data(last_submitted, sim_data):
    qstat = sh.Command("qstat")
    grep = sh.Command("grep")

    are_on_queue = True

    while are_on_queue:
        try:
            grep(qstat(), str(last_submitted))
            print "{0} submitted last still on queue," \
                  " waiting 20 min from {1}".format(last_submitted, datetime.datetime.now().time())
            time.sleep(20 * 60)
        except:
            are_on_queue = False

    save_all_file_lines = iterate_over_folders(sim_data, [create_task_to_archive_results_from_folder])

    last_saver = submit_to_queue(save_all_file_lines)

    are_on_queue = True
    while are_on_queue:
        try:
            grep(qstat(), str(last_saver))
            print "{0} saver still on queue," \
                  " waiting 20 min from {1}".format(last_submitted, datetime.datetime.now().time())
            time.sleep(20 * 60)
        except:
            are_on_queue = False


def create_min_tar_names(*args):
    return "MD_Mini_{0}.tar".format(args[0])


def create_full_tar_names(*args):
    return "MD_Full_{0}.tar.gz".format(args[0])


def create_pics_tar_names(*args):
    return "MD_Pics_{0}.tar.gz".format(args[0])


def create_data_saved_files(sim_data):
    if os.path.exists("mini_data_saved"):
        print "Previous saved MINI data exists, download it! \n" \
              " waiting 20 min from {0}".format(datetime.datetime.now().time())
        time.sleep(20 * 60)
    else:
        iterate_over_folders(sim_data, [])
        with open("mini_data_saved.tmp", "w") as data_saved:
            for name in iterate_over_folders(sim_data, [create_min_tar_names]):
                data_saved.write("scp grace:" + os.path.join(os.getcwd(), name) + " " + name + "\n")
                data_saved.write("tar -xf " + name + "\n")
                data_saved.write("rm " + name + "\n")

        shutil.move("mini_data_saved.tmp", "mini_data_saved")

    if sim_data["Base"]["SaveParticlesInfo"]:
        if os.path.exists("full_data_saved"):
            print "Previous saved FULL data exists, download it! \n" \
                  " waiting 20 min from {0}".format(datetime.datetime.now().time())
            time.sleep(20 * 60)
        else:
            iterate_over_folders(sim_data, [])
            with open("full_data_saved.tmp", "w") as data_saved:
                for name in iterate_over_folders(sim_data, [create_min_tar_names]):
                    data_saved.write("scp grace:" + os.path.join(os.getcwd(), name) + " " + name + "\n")
                    data_saved.write("tar -xf " + name + "\n")
                    data_saved.write("rm " + name + "\n")

            shutil.move("full_data_saved.tmp", "full_data_saved")

    if sim_data["Base"]["SaveEpsPicture"]:
        if os.path.exists("pics_data_saved"):
            print "Previous saved PICTURES data exists, download it! \n" \
                  " waiting 20 min from {0}".format(datetime.datetime.now().time())
            time.sleep(20 * 60)
        else:
            iterate_over_folders(sim_data, [])
            with open("pics_data_saved.tmp", "w") as data_saved:
                for name in iterate_over_folders(sim_data, [create_min_tar_names]):
                    data_saved.write("scp grace:" + os.path.join(os.getcwd(), name) + " " + name + "\n")
                    data_saved.write("tar -xf " + name + "\n")
                    data_saved.write("rm " + name + "\n")

            shutil.move("pics_data_saved.tmp", "pics_data_saved")

if __name__ == "__main__":
    run_all_file_lines = []
    populate_data(simData, run_all_file_lines)

    data = ""

    last_submitted = submit_to_queue(run_all_file_lines)
    save_data(last_submitted, simData)
    create_data_saved_files(simData)
