
include_directories( 
  ${WMPROXY_API_CPP_INCLUDE_DIRS}
  ${JOBMAN_EXCEPTION_INCLUDE_DIRS}
  ${JDL_API_CPP_INCLUDE_DIRS}
  ${WMSAPI_INCLUDE_DIRS}
  )

add_custom_target(WMSUIUTILITIES DEPENDS 
  adutils.cpp 
  excman.cpp 
  logman.cpp 
  options_utils.cpp 
  utils.cpp )


add_library(glite_wmsui_commands_utilities 
  SHARED 
  adutils.cpp 
  excman.cpp 
  logman.cpp 
  options_utils.cpp 
  utils.cpp )


set_target_properties(glite_wmsui_commands_utilities  PROPERTIES VERSION 0.0.0 SOVERSION 0)


install(TARGETS glite_wmsui_commands_utilities 
  LIBRARY 
  DESTINATION ${PREFIX}/usr/${LIBDIR} )


if(DEFINED INSTALL_BUILD)
  install(TARGETS glite_wmsui_commands_utilities
    DESTINATION ${INSTALL_BUILD}/usr/${BINDIR})
endif(DEFINED INSTALL_BUILD)

