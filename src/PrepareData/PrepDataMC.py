#!/usr/bin/env python

import PrepDataCommon

__author__ = 'mpolovyi'

simData = {
    "Base": {
        "Density": [0.25, 0.5, 0.75],
        "InitialConfiguration": [0],
        "KbT": [0.6, 1.2, 1.8],
        "LoadSavedState": 0,
        "NumberOfSavePoints": 100,
        "NumberOfImageLines": 1,
        "PtCount": [3200],
        "SavedParticles": "",
        "SaveParticlesInfo": True,
        "SaveEpsPicture": False
    },
    "CyclesBetweenSaves": 200000,
    "TimeBetweenSaves": 0.25,
    "Queue": "LONG"
}


if __name__ == "__main__":
    folder_script_names = PrepDataCommon.create_folder_task_pairs(simData, "MC")

    last_submitted = PrepDataCommon.submit_to_queue(folder_script_names)
    PrepDataCommon.save_data(last_submitted, simData, "MC")
    PrepDataCommon.create_data_saved_files(simData, "MC")
