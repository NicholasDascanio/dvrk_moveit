
set( 
  CMAKE_MODULE_PATH 
  ${CMAKE_MODULE_PATH} 
  ${CMAKE_CURRENT_SOURCE_DIR}/code/cmake/Modules
  )

project( sawBarrett )
add_subdirectory( code )
add_subdirectory( examples )

