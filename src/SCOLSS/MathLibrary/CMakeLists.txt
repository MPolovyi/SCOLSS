set(MATH_LIBRARY_HEADERS
        ${MATH_DIR}/CQuaternion.h
        ${MATH_DIR}/CVector.h
        ${MATH_DIR}/FunctionOptimization.h)

set(MATH_LIBRARY_SOURCES
        CQuaternion.cpp
        CVector.cpp
        FunctionOptimization.cpp)

setup_headers("${MATH_LIBRARY_HEADERS}" "${MATH_DIR}")

add_library(MathLibrary STATIC "${MATH_LIBRARY_SOURCES}")
setup_custom_target(MathLibrary "${SRC_ROOT_DIR}${MATH_DIR}")

