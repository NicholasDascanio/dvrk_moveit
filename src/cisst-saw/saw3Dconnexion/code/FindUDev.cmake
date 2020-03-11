# - Try to find UDev
# Once done this will define
#
#  UDEV_FOUND - system has UDev
#  UDEV_INCLUDE_DIR - the libudev include directory
#  UDEV_LIBS - The libudev libraries

find_path( UDEV_INCLUDE_DIR libudev.h )
find_library( UDEV_LIBS udev )

set( UDEV_FOUND FALSE )

if( UDEV_INCLUDE_DIR AND UDEV_LIBS )

    set( UDEV_FOUND TRUE )

endif( UDEV_INCLUDE_DIR AND UDEV_LIBS )

mark_as_advanced( UDEV_INCLUDE_DIR UDEV_LIBS )