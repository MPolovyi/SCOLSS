set(MONTECARLO_SIMULATOR_SOURCES
        main.cpp
        CSimulationController.cpp
        CSimulationParameters.h
        CSimulationController.h
        )

add_executable(MC_dipole ${MONTECARLO_SIMULATOR_SOURCES})
setup_custom_target(MC_dipole "${SRC_ROOT_DIR}${MONTECARLO_SIMULATOR_DIR}")

target_link_libraries(MC_dipole ParticlePhysLibrary MathLibrary)