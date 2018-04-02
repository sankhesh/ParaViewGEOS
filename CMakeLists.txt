cmake_minimum_required(VERSION 3.3)

project(ParaViewGEOS)

set(PROJECT_VERSION_MAJOR "0")
set(PROJECT_VERSION_MINOR "0")
set(PROJECT_VERSION_PATCH "1")
set(PROJECT_VERSION
  "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}")
add_definitions(-DPROJECT_VERSION=\"${PROJECT_VERSION}\")
set(PROJECT_VENDOR "Kitware, Inc.")
set(PROJECT_COPYRIGHT_YEAR "2018")

#------------------------------------------------------------------------------
# Find ParaView
find_package(ParaView REQUIRED)
include(${PARAVIEW_USE_FILE})
if (NOT PARAVIEW_BUILD_QT_GUI)
  message(FATAL_ERROR
    "${PROJECT_NAME} example requires PARAVIEW_BUILD_QT_GUI to be enabled. "
    "Please rebuild ParaView (or point to a different build of ParaView) "
    "with PARAVIEW_BUILD_QT_GUI set to TRUE")
endif()

#------------------------------------------------------------------------------
# ParaView custom application includes
include(ParaViewBranding)
pv_find_package_qt(Qt_Targets
  QT5_COMPONENTS Widgets)

#------------------------------------------------------------------------------
# Some useful variables
set(PVGIS_RESOURCES_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Resources")
set(PVGIS_CMAKE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/CMake")

#------------------------------------------------------------------------------
include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR})

#------------------------------------------------------------------------------
# Add extra library containing custom code for the client.
pv_qt_wrap_cpp(MOC_BUILT_SRCS
  pvgLoadDataReaction.h
  pvgMainWindow.h
)
qt5_wrap_ui(UI_BUILT_SRCS ${PVGIS_RESOURCES_DIR}/UI/pvgMainWindow.ui)

source_group("Generated" FILES
  ${MOC_BUILT_SRCS}
  ${UI_BUILT_SRCS}
)

set(PVGIS_SRC_FILES
  pvgLoadDataReaction.cxx
  pvgLoadDataReaction.h
  pvgMainWindow.cxx
  pvgMainWindow.h
  ${MOC_BUILT_SRCS}
  ${UI_BUILT_SRCS}
)

#------------------------------------------------------------------------------
build_paraview_client(
  ${PROJECT_NAME}
  APPLICATION_NAME "${PROJECT_NAME}"
  TITLE "${PROJECT_NAME}"
  ORGANIZATION "Kitware, Inc."
  SPLASH_IMAGE "${PVGIS_RESOURCES_DIR}/Icons/PVSplashScreen.png"
  VERSION_MAJOR ${PROJECT_VERSION_MAJOR}
  VERSION_MINOR ${PROJECT_VERSION_MINOR}
  VERSION_PATCH ${PROJECT_VERSION_PATCH}
  BUNDLE_ICON "${PVGIS_RESOURCES_DIR}/Icons/pvIcon.icns"
  APPLICATION_ICON "${PVGIS_RESOURCES_DIR}/Icons/pvIcon.ico"
  PVMAIN_WINDOW "pvgMainWindow"
  PVMAIN_WINDOW_INCLUDE "pvgMainWindow.h"
  GUI_CONFIGURATION_XMLS
    ${PVGIS_RESOURCES_DIR}/XML/pvgFilters.xml
    ${PVGIS_RESOURCES_DIR}/XML/pvgReaders.xml
    ${PVGIS_RESOURCES_DIR}/XML/pvgSources.xml
  SOURCES ${PVGIS_SRC_FILES}
)
target_link_libraries(${PROJECT_NAME} LINK_PRIVATE ${Qt_Targets})
