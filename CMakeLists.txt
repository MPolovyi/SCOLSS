cmake_minimum_required(VERSION 2.8)

set(PROJECT_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}")

# This defines the basic input (src) and output (lib & include) directories.
set(SOURCES_ROOT_DIR "${PROJECT_ROOT_DIR}/src")
set(SOURCES_INCLUDE_ROOT_DIR "${PROJECT_ROOT_DIR}/src")
#set(SOURCES_INCLUDE_ROOT_DIR "${PROJECT_ROOT_DIR}/include")
set(DOXIGEN_ROOT_DIR "${PROJECT_ROOT_DIR}/dox")

if(NOT EXISTS "${OUTPUT_ROOT_DIR}")
    set(OUTPUT_ROOT_DIR "${PROJECT_ROOT_DIR}/build")
endif()

message(STATUS "Configured to output to: ${OUTPUT_ROOT_DIR}")

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()
message(STATUS "Configured for build-type: ${CMAKE_BUILD_TYPE}")

if (WIN32)
    if (MSVC)
        #     set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Ox /W3 /bigobj -D_SCL_SECURE_NO_STATUSS")
        set(CMAKE_CXX_FLAGS_DEBUG
                "${CMAKE_CXX_FLAGS_DEBUG} /W3 /bigobj -D_SCL_SECURE_NO_STATUSS")
        set(CMAKE_CXX_FLAGS_RELEASE
                "${CMAKE_CXX_FLAGS_RELEASE} /Ox /W3 /bigobj -D_SCL_SECURE_NO_STATUSS")
        set(LIBRARY_ROOT_DIR "${OUTPUT_ROOT_DIR}/lib_msvc")
        set(INCLUDE_ROOT_DIR "${OUTPUT_ROOT_DIR}/include")
        set(BINARY_ROOT_DIR "${OUTPUT_ROOT_DIR}/bin_msvc")
        message(STATUS "Configured compiler options and output directories for MSVC toolset.")
    else()
        #     set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -std=c++11 -O3 -Wall -Woverloaded-virtual -Wold-style-cast -Wnon-virtual-dtor")
        set(CMAKE_CXX_FLAGS_DEBUG
                "${CMAKE_CXX_FLAGS_DEBUG} -pthread -std=c++11 -Wl,--no-as-needed -Wall -Woverloaded-virtual -Wold-style-cast -Wnon-virtual-dtor")
        set(CMAKE_CXX_FLAGS_RELEASE
                "${CMAKE_CXX_FLAGS_RELEASE} -pthread -std=c++11 -Wl,--no-as-needed -O3 -Wall -Woverloaded-virtual -Wold-style-cast -Wnon-virtual-dtor")
        set(CMAKE_CXX_LINK_FLAGS "${CMAKE_CXX_LINK_FLAGS} --enable-stdcall-fixup")
        set(LIBRARY_ROOT_DIR "${OUTPUT_ROOT_DIR}/lib_w32")
        set(INCLUDE_ROOT_DIR "${OUTPUT_ROOT_DIR}/include")
        set(BINARY_ROOT_DIR "${OUTPUT_ROOT_DIR}/bin_w32")
        message(STATUS "Configured compiler options and output directories for MinGW toolset.")
    endif()
else()
    if(${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU")
        set(CMAKE_CXX_FLAGS_DEBUG
                "${CMAKE_CXX_FLAGS_DEBUG} -fPIC -pthread -std=c++11") # -Wl,--no-as-needed -Wall -Woverloaded-virtual -Wold-style-cast -Wnon-virtual-dtor")
        set(CMAKE_CXX_FLAGS_RELEASE
                "${CMAKE_CXX_FLAGS_RELEASE} -fPIC -pthread -std=c++11 -Wl,--no-as-needed -O3 -Wall -Woverloaded-virtual -Wold-style-cast -Wnon-virtual-dtor")
    elseif(${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
        set(CMAKE_CXX_FLAGS_DEBUG
                "${CMAKE_CXX_FLAGS_DEBUG} -fPIC -pthread -std=c++11 -nostdinc++ -isystem /usr/include/c++/4.8 -isystem /usr/include/x86_64-linux-gnu/c++/4.8 -Wall -Woverloaded-virtual") # -Wold-style-cast -Wnon-virtual-dtor")
        set(CMAKE_CXX_FLAGS_RELEASE
                "${CMAKE_CXX_FLAGS_RELEASE} -fPIC -pthread -std=c++11 -nostdinc++ -isystem /usr/include/c++/4.8 -isystem /usr/include/x86_64-linux-gnu/c++/4.8 -O3 -Wall -Woverloaded-virtual -Wold-style-cast -Wnon-virtual-dtor")
    else()
        set(CMAKE_CXX_FLAGS_DEBUG
                "${CMAKE_CXX_FLAGS_DEBUG} -fPIC -pthread") # -Wall -Woverloaded-virtual -Wold-style-cast -Wnon-virtual-dtor")
        set(CMAKE_CXX_FLAGS_RELEASE
                "${CMAKE_CXX_FLAGS_RELEASE} -fPIC -pthread -O3 -Wall -Woverloaded-virtual -Wold-style-cast -Wnon-virtual-dtor")
    endif()
    set(LIBRARY_ROOT_DIR "${OUTPUT_ROOT_DIR}/lib")

    set(INCLUDE_ROOT_DIR "${OUTPUT_ROOT_DIR}/include")

    set(BINARY_ROOT_DIR "${OUTPUT_ROOT_DIR}/bin")
    message(STATUS "Configured compiler options and output directories for *nix GCC toolset.")
endif()

# Now set the global macros for setting up targets.
macro(setup_custom_target target_name CUSTOM_OUTPUT_PATH)
    set_property(TARGET ${target_name} PROPERTY RUNTIME_OUTPUT_DIRECTORY "${BINARY_ROOT_DIR}")
    set_property(TARGET ${target_name} PROPERTY LIBRARY_OUTPUT_DIRECTORY "${LIBRARY_ROOT_DIR}")
    set_property(TARGET ${target_name} PROPERTY ARCHIVE_OUTPUT_DIRECTORY "${LIBRARY_ROOT_DIR}")
    get_target_property(is_target_excluded_from_all ${target_name} EXCLUDE_FROM_ALL)
    if( NOT is_target_excluded_from_all )
        install(TARGETS ${target_name} RUNTIME DESTINATION "${BINARY_ROOT_DIR}" LIBRARY DESTINATION "${LIBRARY_ROOT_DIR}" ARCHIVE DESTINATION "${LIBRARY_ROOT_DIR}")
    endif()
    message(STATUS "Registered target ${target_name}.")
endmacro(setup_custom_target)

macro(setup_headers HEADER_FILES HEADER_PATH)
    foreach(CURRENT_HEADER_FILE ${HEADER_FILES})
        install(FILES "${SOURCES_ROOT_DIR}${CURRENT_HEADER_FILE}" DESTINATION "${INCLUDE_ROOT_DIR}${HEADER_PATH}")
#        message(WARNING "Copied header from ${SOURCES_ROOT_DIR}${CURRENT_HEADER_FILE} to ${INCLUDE_ROOT_DIR}${HEADER_PATH}.")
    endforeach(CURRENT_HEADER_FILE)
endmacro(setup_headers)

macro(setup_scripts SCRIPT_FILES SCRIPTS_PATH)
    foreach(CURRENT_SCRIPT_FILE ${SCRIPT_FILES})
        install(FILES "${SOURCES_ROOT_DIR}${CURRENT_SCRIPT_FILE}" DESTINATION "${BINARY_ROOT_DIR}${SCRIPTS_PATH}")
#        message(WARNING "Copied script from ${SOURCES_ROOT_DIR}${CURRENT_SCRIPT_FILE} to ${BINARY_ROOT_DIR}${SCRIPTS_PATH}.")
    endforeach(CURRENT_SCRIPT_FILE)
endmacro(setup_scripts)

# These are the main sub-directories
set(SCOLSS_ROOT_DIR "/SCOLSS")
set(SCOLSS_PREPARE_DATA_DIR "/PrepareData")

set(SUBDIRS
        ${SCOLSS_ROOT_DIR}
        ${SCOLSS_PREPARE_DATA_DIR})

# Add the sub-directories to cmake process.
foreach(CURRENT_SUBDIR ${SUBDIRS})
    add_subdirectory("${SOURCES_ROOT_DIR}${CURRENT_SUBDIR}")
endforeach(CURRENT_SUBDIR)