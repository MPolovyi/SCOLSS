set(PARTICLE_PHYS_LIBRARY_HEADERS
        ${PARTICLE_PHYS_DIR}/CYukawaDipolePt.h
        ${PARTICLE_PHYS_DIR}/CParticleBase.h)

set(PARTICLE_PHYS_LIBRARY_SOURCES
        CYukawaDipolePt.cpp
        CParticleBase.cpp)

setup_headers("${PARTICLE_PHYS_LIBRARY_HEADERS}" "${PARTICLE_PHYS_DIR}")

add_library(ParticlePhysLibrary STATIC ${PARTICLE_PHYS_LIBRARY_SOURCES})
setup_custom_target(ParticlePhysLibrary ${SRC_ROOT_DIR}${PARTICLE_PHYS_DIR})

target_link_libraries(ParticlePhysLibrary MathLibrary)