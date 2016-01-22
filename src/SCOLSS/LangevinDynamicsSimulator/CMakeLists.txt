set(MONTECARLO_SIMULATOR_SOURCES
        main.cpp
        CSimulationController.cpp
        CSimulationParameters.h
        CSimulationController.h
        )

add_executable(MD_dipole ${MONTECARLO_SIMULATOR_SOURCES})
setup_custom_target(MD_dipole "${SRC_ROOT_DIR}${LANGEVINDYNAMICS_SIMULATOR_DIR}")

target_link_libraries(MD_dipole ParticlePhysLibrary MathLibrary)