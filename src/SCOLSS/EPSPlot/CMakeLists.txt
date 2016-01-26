set(EPS_PLOT_HEADERS
        ${EPS_PLOT_DIR}/EPSPlot.h
        )

set(EPS_PLOT_SOURCES
        EPSPlot.cpp
        )

setup_headers("${EPS_PLOT_HEADERS}" "${EPS_PLOT_DIR}")

add_library(EPSPlotLibrary STATIC ${EPS_PLOT_SOURCES})
setup_custom_target(EPSPlotLibrary ${SRC_ROOT_DIR}${EPS_PLOT_DIR})