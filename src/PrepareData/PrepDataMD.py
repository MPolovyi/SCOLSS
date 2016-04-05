#!/usr/bin/env python

import PrepDataCommon

__author__ = 'mpolovyi'

simData = {
    "Base": {
        "Density": [0.25, 0.5, 0.75], # 0.5, 0.75],
        "InitialConfiguration": [0, 1],
        "KbT": [1],# 1.2, 1.8],
        "LoadSavedState": 0,
        "NumberOfSavePoints": 10,
        "NumberOfImageLines": 1,
        "PtCount": [1600, 3200, 6400],
        "SavedParticles": "",
        "SaveParticlesInfo": True,
        "SaveEpsPicture": False
    },
    "CyclesBetweenSaves": 200000,
    "TimeBetweenSaves": 0.2,
    "Queue": "LONG",
    "Threads": 4
}


if __name__ == "__main__":
    folder_script_names = PrepDataCommon.create_folder_task_pairs(simData, "LD")

    last_submitted = PrepDataCommon.submit_to_queue(folder_script_names)
    PrepDataCommon.save_data(last_submitted, simData, "LD")
    PrepDataCommon.create_data_saved_files(simData, "LD")
