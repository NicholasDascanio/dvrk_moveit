#
#
# (C) Copyright 2011-2012 Johns Hopkins University (JHU), All Rights
# Reserved.
#
# --- begin cisst license - do not edit ---
#
# This software is provided "as is" under an open source license, with
# no warranty.  The complete license can be found in license.txt and
# http://www.cisst.org/cisst/license.txt.
#
# --- end cisst license ---

set(
  REQUIRED_CISST_LIBRARIES
  cisstCommon
  cisstVector
  cisstOSAbstraction
  cisstMultiTask )

find_package( cisst REQUIRED ${REQUIRED_CISST_LIBRARIES} )

if( cisst_FOUND_AS_REQUIRED )

  set( sawCANBus_DEFINITIONS )
  set( sawCANBus_LIBRARIES )

  # create/configure file for find_package (sawCANBus)
  set( sawCANBus_INCLUDE_DIR "${sawCANBus_SOURCE_DIR}/include" )
  set( sawCANBus_LIBRARY_DIR "${LIBRARY_OUTPUT_PATH}" )
  set( sawCANBus_LIBRARIES sawCANBus )

  include_directories (${sawCANBus_INCLUDE_DIR})
  set (sawCANBus_HEADER_DIR "${sawCANBus_INCLUDE_DIR}/sawCANBus")

  # load cisst configuration
  include( ${CISST_USE_FILE} )

  set(
    HEADER_FILES
    ${sawCANBus_HEADER_DIR}/osaCANBus.h
    ${sawCANBus_HEADER_DIR}/osaCANBusFrame.h
    ${sawCANBus_HEADER_DIR}/mtsCANBus.h
    ${sawCANBus_HEADER_DIR}/mtsCANBusFrame.h )

  set(
    SOURCE_FILES
    osaCANBus.cpp
    osaCANBusFrame.cpp
    mtsCANBus.cpp
    mtsCANBusFrame.cpp )

  #Check for Socket CAN
  find_package( SocketCAN )
  if( SocketCANFound )
    set( HEADER_FILES ${HEADER_FILES} ${sawCANBus_HEADER_DIR}/osaSocketCAN.h )
    set( SOURCE_FILES ${SOURCE_FILES} osaSocketCAN.cpp )
  endif( SocketCANFound )

  # Check for RT Socket CAN
  find_package( RTSocketCAN )
  if( RTSocketCANFound )

    set(
      HEADER_FILES
      ${HEADER_FILES}
      ${sawCANBus_HEADER_DIR}/osaRTSocketCAN.h
      ${sawCANBus_HEADER_DIR}/mtsRTSocketCAN.h )

    set(
      SOURCE_FILES
      ${SOURCE_FILES}
      osaRTSocketCAN.cpp )

    set( sawCANBus_HAS_RTSocketCAN ON)
    set( sawCANBus_LIBRARIES_PATH "${RTDM_LIBRARIES_PATH}" )
    set( sawCANBus_LIBRARIES "${RTDM_LIBRARIES}" )
    set( sawCANBus_DEFINITIONS "${RTDM_DEFINITIONS}" )
    include_directories( ${RTDM_INCLUDE_DIR} )

  endif( RTSocketCANFound )

  # Check for BitCtrl
  find_package( BitCtrl )
  if( BitCtrlFound )

    set(
      HEADER_FILES
      ${HEADER_FILES}
      ${sawCANBus_HEADER_DIR}/CiA301.h
      ${sawCANBus_HEADER_DIR}/CiA402.h
      ${sawCANBus_HEADER_DIR}/osaCANopen.h
      ${sawCANBus_HEADER_DIR}/osaBitCtrl.h )

    set(
      SOURCE_FILES
      ${SOURCE_FILES}
      osaCANopen.cpp
      osaBitCtrl.cpp )

    include_directories( ${BITCTRL_INCLUDE_DIR} )

  endif( BitCtrlFound )

  add_definitions( ${sawCANBus_DEFINITIONS} )

  add_library( sawCANBus ${HEADER_FILES} ${SOURCE_FILES} )
  target_link_libraries( sawCANBus ${sawCANBus_LIBRARIES} )
  cisst_target_link_libraries( sawCANBus ${REQUIRED_CISST_LIBRARIES} )


  configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/sawCANBusConfig.cmake.in"
    "${sawCANBus_BINARY_DIR}/sawCANBusConfig.cmake"
    @ONLY@ )

  # Install targets for headers and library
  install( FILES ${HEADER_FILES}
    DESTINATION include/sawCANBus)

  install( TARGETS sawCANBus
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib)

  install( FILES "${sawComponents_BINARY_DIR}/sawCANBusConfig.cmake"
    DESTINATION "${CISST_CMAKE_INSTALL_SUFFIX}/saw")


endif( cisst_FOUND_AS_REQUIRED )
