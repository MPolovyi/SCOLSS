set(R_EXTENSION_HEADERS
        ${R_EXTENSION_DIR}/ExtensionDefs.h
        )

set(R_EXTENSION_SOURCES
        Extension.cpp
        )

setup_headers("${CORE_SIMULATOR_HEADERS}" "${CORE_SIMULATOR_DIR}")

add_library(RExtensionLibrary SHARED ${R_EXTENSION_SOURCES})
target_link_libraries(RExtensionLibrary MathLibrary ParticlePhysLibrary)
setup_custom_target(RExtensionLibrary ${SRC_ROOT_DIR}${CORE_SIMULATOR_DIR})

add_executable(RExtensionTest main.cpp)
target_link_libraries(RExtensionTest ${CMAKE_DL_LIBS} RExtensionLibrary)
setup_custom_target(RExtensionTest ${SRC_ROOT_DIR}${CORE_SIMULATOR_DIR})
