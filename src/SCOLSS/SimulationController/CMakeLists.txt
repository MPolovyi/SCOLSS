set(CORE_SIMULATOR_HEADERS
        ${CORE_SIMULATOR_DIR}/CBaseSimCtrl.h
        ${CORE_SIMULATOR_DIR}/CBaseSimParams.h
        )

set(CORE_SIMULATOR_SOURCES
        CBaseSimCtrl.cpp
        CBaseSimParams.cpp CLangevinSimCtrl.cpp CLangevinSimCtrl.h CLangevinSimParams.cpp CLangevinSimParams.h CMonteCarloSimParams.cpp CMonteCarloSimParams.h)

setup_headers("${CORE_SIMULATOR_HEADERS}" "${CORE_SIMULATOR_DIR}")

add_library(CoreSimulatorLibrary STATIC "${CORE_SIMULATOR_SOURCES}")
setup_custom_target(CoreSimulatorLibrary "${SRC_ROOT_DIR}${CORE_SIMULATOR_DIR}")

target_link_libraries(CoreSimulatorLibrary EPSPlotLibrary MathLibrary ParticlePhysLibrary)