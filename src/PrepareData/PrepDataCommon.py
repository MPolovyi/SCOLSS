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


def submit_to_queue(folder_script_pairs, max_on_queue=600):
    qsub = sh.Command("qsub")
    for run_line in folder_script_pairs:
        with MiscFunctions.cd(os.path.join("./", run_line[0])):
            last_submitted = qsub(run_line[1])
            print last_submitted
            time.sleep(5)

        while get_tasks_on_queue() > max_on_queue:
            print "{0} tasks on queue is greater then maximum allowed {1}," \
                  " waiting 20 min from {2}".format(get_tasks_on_queue(), max_on_queue, datetime.datetime.now().time())
            time.sleep(20 * 60)

    return int(last_submitted.split(" ")[2])


def folder_name_generator(sim_data, sim_type):
    if sim_type == "LD":
        return "T_{0:.2f}_I_{1:d}_K_{2:.2f}_P_{3:d}_D_{4:.2f}".format(
            sim_data["TimeBetweenSaves"],
            sim_data["Base"]["InitialConfiguration"],
            sim_data["Base"]["KbT"],
            sim_data["Base"]["PtCount"],
            sim_data["Base"]["Density"])

    elif sim_type == "MC":
        return "C_{0:d}_I_{1:d}_K_{2:.2f}_P_{3:d}_D_{4:.2f}".format(
            sim_data["CyclesBetweenSaves"],
            sim_data["Base"]["InitialConfiguration"],
            sim_data["Base"]["KbT"],
            sim_data["Base"]["PtCount"],
            sim_data["Base"]["Density"])


def iterate_over_folders(full_sim_data, func_list, sim_type, *args):
    ret = []

    for ptc in full_sim_data["Base"]["PtCount"]:
        for rho in full_sim_data["Base"]["Density"]:
            for kbt in full_sim_data["Base"]["KbT"]:
                for ic in full_sim_data["Base"]["InitialConfiguration"]:
                    sim_data_to_save = dict(full_sim_data)
                    sim_data_to_save["Base"] = dict(full_sim_data["Base"])
                    sim_data_to_save["Base"]["KbT"] = kbt
                    sim_data_to_save["Base"]["PtCount"] = ptc
                    sim_data_to_save["Base"]["Density"] = rho
                    sim_data_to_save["Base"]["InitialConfiguration"] = ic

                    folder_name = folder_name_generator(sim_data_to_save, sim_type)

                    for func in func_list:
                        ret.append(func(folder_name, sim_data_to_save, sim_type, args))

    return ret


def copy_exec_file(*args):
    folder_name = args[0]
    if os.path.exists(folder_name):
        shutil.copy("ExecFile", folder_name + "/ExecFile")
    else:
        os.makedirs(folder_name)
        shutil.copy("ExecFile", folder_name + "/ExecFile")


class CreateSampleRunFiles:
    def __init__(self, samples_per_run_count, simulation_type):
        self.Index = 1
        self.SamplesPerRun = samples_per_run_count
        self.FolderScriptPairs = []
        self.SimulationType = simulation_type

    def __call__(self, *args):
        folder_name = args[0]
        sim_data_to_save = args[1]

        run_index_string = str(self.Index)

        tar_files = "MiniData_Data_{0}.json.tar"

        if sim_data_to_save["Base"]["SaveParticlesInfo"]:
            save_full_data = "true"
        else:
            save_full_data = "false"

        if sim_data_to_save["Base"]["SaveEpsPicture"]:
            save_pics_data = "true"
        else:
            save_pics_data = "false"

        run_file_lines = ('#$ -S /bin/sh\n'
                          '#$ -j y\n'
                          '#$ -m eas\n'
                          '#$ -cwd\n'
                          '#$ -l virtual_free=800M -l h_vmem=800M\n'
                          '#$ -q {0}\n'
                          '\n'
                          'MAX_NUMBER_OF_CP=5\n'
                          '\n'
                          'function limited_copy {{'
                          '  # arguments:\n'
                          '  # $1 = from\n'
                          '  # $2 = to\n'
                          '  # $3 = sleep time (optional)\n'
                          '\n'
                          '  NUMBER_OF_CP_ACTIVE=$(ps -A | grep " cp" | wc -l)\n'
                          '\n'
                          '  while [[ NUMBER_OF_CP_ACTIVE -gt MAX_NUMBER_OF_CP ]]; do\n'
                          '    sleep ${{3:-20s}}  \n'
                          '    NUMBER_OF_CP_ACTIVE=$(ps -U mpolovyi | grep " cp" | wc -l)\n'
                          '  done\n'
                          '\n'
                          '  cp ${{1}} ${{2}}\n'
                          '}}\n'
                          '\n'
                          'cp $SGE_O_WORKDIR/ExecFile $TMPDIR\n'
                          '\n'
                          'limited_copy $SGE_O_WORKDIR/Data_{1}.json $TMPDIR\n'
                          '\n'
                          'cd $TMPDIR\n'
                          '(time ./ExecFile Data_{1}.json {2} {3}) >&time_{1}.txt\n'
                          '\n'
                          'cp MiniData_Data_{1}.json.tar $SGE_O_WORKDIR\n'
                          '\n'
                          'if {4}; then\n'
                          '  limited_copy FullData_Data_{1}.json.tar $SGE_O_WORKDIR 5m\n'
                          'fi\n'
                          '\n'
                          'if {5}; then\n'
                          '  limited_copy PicsData_Data_{1}.json.tar $SGE_O_WORKDIR 5m\n'
                          'fi\n'
                          '\n'
                          'rm *\n'
                          ).format(sim_data_to_save["Queue"], run_index_string, self.SimulationType,
                                   str(self.SamplesPerRun), save_full_data, save_pics_data)

        temp_run_file_name = "r{0}".format(run_index_string)
        with open(temp_run_file_name, "w") as run_file:
            run_file.write(run_file_lines)

        run_file_name = "{3}T{0:.2f}_N{1:d}r{2:s}".format(sim_data_to_save["Base"]["KbT"],
                                                          sim_data_to_save["Base"]["PtCount"],
                                                          run_index_string, self.SimulationType)

        shutil.move(temp_run_file_name, os.path.join(folder_name, run_file_name))
        self.FolderScriptPairs.append((folder_name, run_file_name))

        with open("Data0.json", "w") as outfile:
            json.dump(sim_data_to_save, outfile, sort_keys=True, indent=4, separators=(',', ': '))

        fname = "Data_{0}.json".format(run_index_string)
        shutil.move("Data0.json", folder_name + "/" + fname)


def create_folder_task_pairs(sim_data, sim_type):
    samples_count, samples_per_run_count = (1, 1)
    try:
        samples_count, samples_per_run_count = prompt_for_data(sim_data)
    except ValueError as err:
        print(json.dumps(err[0], indent=2, sort_keys=True))
        if err[1]:
            return

    iterate_over_folders(sim_data, [copy_exec_file], sim_type)

    sample_creator = CreateSampleRunFiles(samples_per_run_count, sim_type)

    for index in range(0, samples_count):
        sample_creator.Index = index

        iterate_over_folders(sim_data, [sample_creator], sim_type)

    return sample_creator.FolderScriptPairs


def create_task_to_archive_results_from_folder_md(*args):
    folder_name = args[0]
    sim_data = args[1]
    sim_type = args[2]

    print "Writing results form folder {0} to include file".format(folder_name)

    tar_save_lines = 'tar -czpf ' + sim_type + '_Mini_{0}.tar ./{0}/MiniData*.tar\n'

    if sim_data["Base"]["SaveParticlesInfo"]:
        tar_save_lines += ' tar -czpf ' + sim_type + '_Full_{0}.tar.gz ./{0}/FullData*.tar\n'

    if sim_data["Base"]["SaveEpsPicture"]:
        tar_save_lines += ' tar -cpf ' + sim_type + '_Pics_{0}.tar.gz ./{0}/PicsData*.tar\n'

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


def save_data(last_submitted, sim_data, sim_type):
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

    tar = sh.Command("tar")

    for name, tar_pattern in iterate_over_folders(sim_data, [create_mini_tar_names], sim_type):
        tar("-czpf", name, tar_pattern)

    for name, tar_pattern in iterate_over_folders(sim_data, [create_full_tar_names], sim_type):
        try:
            tar("-czpf", name, tar_pattern)
        except:
            pass

    for name, tar_pattern in iterate_over_folders(sim_data, [create_pics_tar_names], sim_type):
        try:
            tar("-czpf", name, tar_pattern)
        except:
            pass


def create_mini_tar_names(*args):
    folder_name = args[0]
    return "{0}_Mini_{1}.tar.gz".format(args[2], folder_name), "./{0}/Mini*".format(folder_name)


def create_full_tar_names(*args):
    folder_name = args[0]
    return "{0}_Full_{1}.tar.gz".format(args[2], folder_name), "./{0}/Full*".format(folder_name)


def create_pics_tar_names(*args):
    folder_name = args[0]
    return "{0}_Pics_{1}.tar.gz".format(args[2], folder_name), "./{0}/Pics*".format(folder_name)


def create_data_saved_files(sim_data, sim_type):
    create_data_saved_file(sim_data, sim_type, create_mini_tar_names, "mini_data_saved")

    if sim_data["Base"]["SaveParticlesInfo"]:
        create_data_saved_file(sim_data, sim_type, create_full_tar_names, "full_data_saved")

    if sim_data["Base"]["SaveEpsPicture"]:
        create_data_saved_file(sim_data, sim_type, create_pics_tar_names, "pics_data_saved")


def create_data_saved_file(sim_data, sim_type, name_creator, save_file_name):
    if os.path.exists(save_file_name):
        print "Previous saved {1} data exists, download it! \n" \
              " waiting 20 min from {0}".format(datetime.datetime.now().time(), save_file_name.upper())
        time.sleep(20 * 60)
    else:
        with open(save_file_name + ".tmp", "w") as data_saved:
            for name in iterate_over_folders(sim_data, [name_creator], sim_type):
                data_saved.write("scp grace:" + os.path.join(os.getcwd(), name) + " " + name + "\n")
                data_saved.write("tar -xzf " + name + "\n")
                data_saved.write("rm " + name + "\n")

        shutil.move(save_file_name + ".tmp", save_file_name)
