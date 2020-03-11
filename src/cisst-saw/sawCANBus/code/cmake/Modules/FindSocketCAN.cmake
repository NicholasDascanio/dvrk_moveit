
set( SocketCANFound FALSE )

# Find if SocketCAN header files are there

find_path( SOCKETCAN_INCLUDE_DIR can.h HINTS /usr/include /usr/include/linux )

if( SOCKETCAN_INCLUDE_DIR )
  set( SocketCANFound True )
  set( SocketCAN_INCLUDE_DIRS ${SOCKETCAN_INCLUDE_DIR} )
endif( SOCKETCAN_INCLUDE_DIR )

mark_as_advanced( SOCKETCAN_INCLUDE_DIR )
