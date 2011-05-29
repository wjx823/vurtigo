# List of source files.
set(INFUSION_MONITOR_SRC_FILES
  InfusionMonitorPlugin.cpp
  InfusionMonitorUI.cpp
)

# MOC files are the ones the use Q_OBJECT
set (INFUSION_MONITOR_MOC_FILES
  InfusionMonitorPlugin.h
  InfusionMonitorUI.h
)

set (UI_FILES_INFUSION_MONITOR
  InfusionMonitor.ui
)

qt4_wrap_ui(UI_INFUSION_MONITOR_OUT ${UI_FILES_INFUSION_MONITOR})
qt4_wrap_cpp(INFUSION_MONITOR_MOC_FILES_OUT ${INFUSION_MONITOR_MOC_FILES} )

include_directories( 
  ${CMAKE_BINARY_DIR}/Plugins/InfusionMonitor2D
)

add_library(InfusionMonitor SHARED
  ${INFUSION_MONITOR_SRC_FILES}
  ${UI_INFUSION_MONITOR_OUT}
  ${INFUSION_MONITOR_MOC_FILES_OUT}
)

target_link_libraries(InfusionMonitor
  ${QT_LIBRARIES} 
  VurtigoBase
)
