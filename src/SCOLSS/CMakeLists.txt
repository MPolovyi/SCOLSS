project(SCOLSS)

set(MATH_DIR "${SCOLSS_ROOT_DIR}/MathLibrary")
set(EXEC_DIR "${SCOLSS_ROOT_DIR}/ExecFile")
set(PARTICLE_PHYS_DIR "${SCOLSS_ROOT_DIR}/ParticlePhysics")
set(MONTECARLO_SIMULATOR_DIR "${SCOLSS_ROOT_DIR}/MonteCarloSimulator")
set(LANGEVINDYNAMICS_SIMULATOR_DIR "${SCOLSS_ROOT_DIR}/LangevinDynamicsSimulator")
set(EPS_PLOT_DIR "${SCOLSS_ROOT_DIR}/EPSPlot")

set(SCOLSS_ROOT_DIR
        ${MATH_DIR}
        ${EXEC_DIR}
        ${PARTICLE_PHYS_DIR}
        ${MONTECARLO_SIMULATOR_DIR}
        ${LANGEVINDYNAMICS_SIMULATOR_DIR}
        ${EPS_PLOT_DIR})

include_directories("${SOURCES_INCLUDE_ROOT_DIR}")

message(STATUS "included directories ${INCLUDE_ROOT_DIR}")

# Add the sub-directories to cmake process.
foreach(CURRENT_SUBDIR ${SCOLSS_ROOT_DIR})
    add_subdirectory("${SOURCES_ROOT_DIR}${CURRENT_SUBDIR}")
endforeach(CURRENT_SUBDIR)