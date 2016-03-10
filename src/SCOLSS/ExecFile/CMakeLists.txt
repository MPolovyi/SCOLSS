set(EXEC_FILE_SOURCES
        main.cpp
        CTarball.cpp
        CTarball.h
        CMyException.h)

find_package(MPI REQUIRED)

include_directories(${MPI_INCLUDE_PATH})

add_executable(ExecFile ${EXEC_FILE_SOURCES})
setup_custom_target(ExecFile ${SRC_ROOT_DIR}${EXEC_DIR})

target_link_libraries(ExecFile MathLibrary EPSPlotLibrary MathLibrary ParticlePhysLibrary CoreSimulatorLibrary ${MPI_LIBRARIES})