set(CORE_SIMULATOR_HEADERS
        ${CORE_SIMULATOR_DIR}/BaseSimCtrl/CBaseSimCtrl.h
        ${CORE_SIMULATOR_DIR}/BaseSimCtrl/CBaseSimParams.h
        ${CORE_SIMULATOR_DIR}/MonteCarloSimCtrl/CMonteCarloSimParams.h
        ${CORE_SIMULATOR_DIR}/MonteCarloSimCtrl/CMonteCarloSimCtrl.h
        ${CORE_SIMULATOR_DIR}/LangevinSimCtrl/CLangevinSimParams.h
        ${CORE_SIMULATOR_DIR}/LangevinSimCtrl/CLangevinSimCtrl.h
        )

set(CORE_SIMULATOR_SOURCES
        BaseSimCtrl/CBaseSimCtrl.cpp
        BaseSimCtrl/CBaseSimParams.cpp
        MonteCarloSimCtrl/CMonteCarloSimParams.cpp
        MonteCarloSimCtrl/CMonteCarloSimCtrl.cpp
        LangevinSimCtrl/CLangevinSimCtrl.cpp
        LangevinSimCtrl/CLangevinSimParams.cpp
        )

setup_headers("${CORE_SIMULATOR_HEADERS}" "${CORE_SIMULATOR_DIR}")

add_library(CoreSimulatorLibrary STATIC "${CORE_SIMULATOR_SOURCES}")
setup_custom_target(CoreSimulatorLibrary "${SRC_ROOT_DIR}${CORE_SIMULATOR_DIR}")

target_link_libraries(CoreSimulatorLibrary EPSPlotLibrary MathLibrary ParticlePhysLibrary)