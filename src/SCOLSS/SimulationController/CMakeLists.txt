set(BASE_SIM_DIR "/BaseSimCtrl")
set(LD_SIM_DIR "/LangevinSimCtrl")
set(MC_SIM_DIR "/MonteCarloSimCtrl")

set(CORE_SIMULATOR_HEADERS
        ${CORE_SIMULATOR_DIR}/${BASE_SIM_DIR}/CBaseSimCtrl.h
        ${CORE_SIMULATOR_DIR}/${BASE_SIM_DIR}/CBaseSimParams.h
        ${CORE_SIMULATOR_DIR}/${MC_SIM_DIR}/CMonteCarloSimParams.h
        ${CORE_SIMULATOR_DIR}/${MC_SIM_DIR}/CMonteCarloSimCtrl.h
        ${CORE_SIMULATOR_DIR}/${LD_SIM_DIR}/CLangevinSimParams.h
        ${CORE_SIMULATOR_DIR}/${LD_SIM_DIR}/CLangevinSimCtrl.h
        )


set(BASE_SIM_CTRL_HEADERS
        ${CORE_SIMULATOR_DIR}/${BASE_SIM_DIR}/CBaseSimCtrl.h
        ${CORE_SIMULATOR_DIR}/${BASE_SIM_DIR}/CBaseSimParams.h
        )

set(LD_SIM_CTRL_HEADERS
        ${CORE_SIMULATOR_DIR}/${LD_SIM_DIR}/CLangevinSimParams.h
        ${CORE_SIMULATOR_DIR}/${LD_SIM_DIR}/CLangevinSimCtrl.h
        )

set(MC_SIM_CTRL_HEADERS
        ${CORE_SIMULATOR_DIR}/${MC_SIM_DIR}/CMonteCarloSimParams.h
        ${CORE_SIMULATOR_DIR}/${MC_SIM_DIR}/CMonteCarloSimCtrl.h
        )

setup_headers("${BASE_SIM_CTRL_HEADERS}" "${CORE_SIMULATOR_DIR}/${BASE_SIM_DIR}")
setup_headers("${LD_SIM_CTRL_HEADERS}" "${CORE_SIMULATOR_DIR}/${LD_SIM_DIR}")
setup_headers("${MC_SIM_CTRL_HEADERS}" "${CORE_SIMULATOR_DIR}/${MC_SIM_DIR}")

setup_headers("${CORE_SIMULATOR_HEADERS}" "${CORE_SIMULATOR_DIR}")

set(CORE_SIMULATOR_SOURCES
        BaseSimCtrl/CBaseSimCtrl.cpp
        BaseSimCtrl/CBaseSimParams.cpp
        MonteCarloSimCtrl/CMonteCarloSimParams.cpp
        MonteCarloSimCtrl/CMonteCarloSimCtrl.cpp
        LangevinSimCtrl/CLangevinSimCtrl.cpp
        LangevinSimCtrl/CLangevinSimParams.cpp
        )

add_library(CoreSimulatorLibrary STATIC ${CORE_SIMULATOR_SOURCES})
setup_custom_target(CoreSimulatorLibrary ${SRC_ROOT_DIR}${CORE_SIMULATOR_DIR})

target_link_libraries(CoreSimulatorLibrary EPSPlotLibrary MathLibrary ParticlePhysLibrary)