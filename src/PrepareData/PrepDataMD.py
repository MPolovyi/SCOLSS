#!/usr/bin/env python

import PrepDataCommon

__author__ = 'mpolovyi'

simData = {
    "Base": {
        "Density": [0.01], # 0.5, 0.75],
        "InitialConfiguration": [5],
        "KbT": [0.6, 1.2, 1.8],
        "LoadSavedState": 0,
        "NumberOfSavePoints": 1000,
        "NumberOfImageLines": 1,
        "PtCount": [200],
        "SavedParticles": "",
        "SaveParticlesInfo": True,
        "SaveEpsPicture": False
    },
    "CyclesBetweenSaves": 500,
    "TimeBetweenSaves": 0.25,
    "Queue": "SHORT"
}


if __name__ == "__main__":
    folder_script_names = PrepDataCommon.create_folder_task_pairs(simData, "LD")

    last_submitted = PrepDataCommon.submit_to_queue(folder_script_names)
    PrepDataCommon.save_data(last_submitted, simData, "LD")
    PrepDataCommon.create_data_saved_files(simData, "LD")
