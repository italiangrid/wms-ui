include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/autogen
	${PYTHON_INCLUDE}
	${CLASSAD_INCLUDE_DIRS}
)

add_custom_command(OUTPUT 
  ${CMAKE_CURRENT_SOURCE_DIR}/autogen/LbWrapper_wrap.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/autogen/LogWrapper_wrap.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/autogen/UcWrapper_wrap.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/autogen/AdWrapper_wrap.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/autogen/SdWrapper_wrap.cpp
  DEPENDS
  ${CMAKE_CURRENT_SOURCE_DIR}/glite_wmsui_LogWrapper.i
  ${CMAKE_CURRENT_SOURCE_DIR}/glite_wmsui_LbWrapper.i
  ${CMAKE_CURRENT_SOURCE_DIR}/glite_wmsui_UcWrapper.i
  ${CMAKE_CURRENT_SOURCE_DIR}/glite_wmsui_AdWrapper.i
  ${CMAKE_CURRENT_SOURCE_DIR}/glite_wmsui_SdWrapper.i
  COMMAND mkdir -p ${CMAKE_CURRENT_SOURCE_DIR}/autogen
  COMMAND
  swig  -cpperraswarn -python -shadow -c++ -o
  ${CMAKE_CURRENT_SOURCE_DIR}/autogen/LbWrapper_wrap.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/glite_wmsui_LbWrapper.i
  COMMAND
  swig  -cpperraswarn -python -shadow -c++ -o
  ${CMAKE_CURRENT_SOURCE_DIR}/autogen/LogWrapper_wrap.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/glite_wmsui_LogWrapper.i
  COMMAND
  swig  -cpperraswarn -python -shadow -c++ -o
  ${CMAKE_CURRENT_SOURCE_DIR}/autogen/UcWrapper_wrap.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/glite_wmsui_UcWrapper.i
  COMMAND
  swig  -cpperraswarn -python -shadow -c++ -o
  ${CMAKE_CURRENT_SOURCE_DIR}/autogen/AdWrapper_wrap.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/glite_wmsui_AdWrapper.i
  COMMAND
  swig  -cpperraswarn -python -shadow -c++ -o
  ${CMAKE_CURRENT_SOURCE_DIR}/autogen/SdWrapper_wrap.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/glite_wmsui_SdWrapper.i
  )

add_custom_target(GENWRAPPER ALL DEPENDS
  ${CMAKE_CURRENT_SOURCE_DIR}/autogen/LbWrapper_wrap.cpp
)

add_library(glite_wmsui_LbWrapper
  SHARED 
  autogen/LbWrapper_wrap.cpp
  LbWrapper.cpp
)

add_library(glite_wmsui_LogWrapper
  SHARED
  autogen/LogWrapper_wrap.cpp 
  LogWrapper.cpp 
)

add_library(glite_wmsui_UcWrapper
  SHARED
  autogen/UcWrapper_wrap.cpp
  UcWrapper.cpp
)

add_library(glite_wmsui_AdWrapper
  SHARED
  autogen/AdWrapper_wrap.cpp
  AdWrapper.cpp
)

add_library(glite_wmsui_SdWrapper
  SHARED
  autogen/SdWrapper_wrap.cpp
  SdWrapper.cpp
)

add_custom_target(SYMLINKS ALL DEPENDS
  glite_wmsui_LbWrapper
  glite_wmsui_LogWrapper
  glite_wmsui_UcWrapper
  glite_wmsui_AdWrapper
  glite_wmsui_SdWrapper
  COMMAND
  ln -sf libglite_wmsui_SdWrapper.so.0.0.0 _glite_wmsui_SdWrapper.so.0.0.0
  COMMAND
  ln -sf libglite_wmsui_SdWrapper.so.0 _glite_wmsui_SdWrapper.so.0
  COMMAND 
  ln -sf libglite_wmsui_SdWrapper.so _glite_wmsui_SdWrapper.so
  COMMAND
  ln -sf libglite_wmsui_UcWrapper.so.0.0.0 _glite_wmsui_UcWrapper.so.0.0.0
  COMMAND
  ln -sf libglite_wmsui_UcWrapper.so.0 _glite_wmsui_UcWrapper.so.0
  COMMAND
  ln -sf libglite_wmsui_UcWrapper.so _glite_wmsui_UcWrapper.so
  COMMAND
  ln -sf libglite_wmsui_AdWrapper.so.0.0.0 _glite_wmsui_AdWrapper.so.0.0.0
  COMMAND
  ln -sf libglite_wmsui_AdWrapper.so.0 _glite_wmsui_AdWrapper.so.0
  COMMAND
  ln -sf libglite_wmsui_AdWrapper.so _glite_wmsui_AdWrapper.so
  COMMAND
  ln -sf libglite_wmsui_LogWrapper.so.0.0.0 _glite_wmsui_LogWrapper.so.0.0.0
  COMMAND
  ln -sf libglite_wmsui_LogWrapper.so.0 _glite_wmsui_LogWrapper.so.0
  COMMAND
  ln -sf libglite_wmsui_LogWrapper.so _glite_wmsui_LogWrapper.so
  COMMAND
  ln -sf libglite_wmsui_LbWrapper.so.0.0.0 _glite_wmsui_LbWrapper.so.0.0.0
  COMMAND
  ln -sf libglite_wmsui_LbWrapper.so.0 _glite_wmsui_LbWrapper.so.0
  COMMAND
  ln -sf libglite_wmsui_LbWrapper.so _glite_wmsui_LbWrapper.so
)

set_target_properties(glite_wmsui_LbWrapper  PROPERTIES VERSION 0.0.0
SOVERSION 0)
set_target_properties(glite_wmsui_LogWrapper PROPERTIES VERSION 0.0.0
SOVERSION 0)
set_target_properties(glite_wmsui_UcWrapper  PROPERTIES VERSION 0.0.0
SOVERSION 0)
set_target_properties(glite_wmsui_AdWrapper   PROPERTIES VERSION 0.0.0
SOVERSION 0)
set_target_properties(glite_wmsui_SdWrapper   PROPERTIES VERSION 0.0.0
SOVERSION 0)

target_link_libraries(glite_wmsui_UcWrapper 
  ${VOMS_LIBRARIES} 
  ${GRIDSITE_OPENSSL_LIBRARIES}
  )

target_link_libraries(glite_wmsui_LbWrapper 
  ${VOMS_LIBRARIES} 
  ${CLASSAD_LIBRARIES} 
  ${LB_CLIENT_LIBRARIES}
  ${LB_CLIENTPP_LIBRARIES}
  ${JDL_API_CPP_LIBRARIES}
  ${Boost_LIBRARIES}
  ${JOBMAN_EXCEPTION_LIBRARIES}
  ${JOBID_LIBRARIES}
  )

target_link_libraries(glite_wmsui_AdWrapper
  ${JOBID_LIBRARIES}
  ${JDL_API_CPP_LIBRARIES}
  ${CLASSAD_UTILS_LIBRARIES}
  ${Boost_LIBRARIES}
  ${JOBMAN_EXCEPTION_LIBRARIES}
  ${CLASSAD_LIBRARIES}
  ${LB_CLIENT_LIBRARIES}
  ${LB_CLIENTPP_LIBRARIES}
  )

target_link_libraries(glite_wmsui_SdWrapper
  ${JOBID_LIBRARIES}
  ${JDL_API_CPP_LIBRARIES}
  ${CLASSAD_UTILS_LIBRARIES}
  ${Boost_LIBRARIES}
  ${JOBMAN_EXCEPTION_LIBRARIES}
  ${CLASSAD_LIBRARIES}
  ${LB_CLIENT_LIBRARIES}
  ${LB_CLIENTPP_LIBRARIES}
  )

target_link_libraries(glite_wmsui_LogWrapper
  ${JOBID_LIBRARIES}
  ${JDL_API_CPP_LIBRARIES}
  ${CLASSAD_UTILS_LIBRARIES}
  ${Boost_LIBRARIES}
  ${JOBMAN_EXCEPTION_LIBRARIES}
  ${CLASSAD_LIBRARIES}
  ${LB_CLIENT_LIBRARIES}
  ${LB_CLIENTPP_LIBRARIES}
  ${VOMS_LIBRARIES}
  )

install(TARGETS glite_wmsui_LbWrapper 
  LIBRARY DESTINATION ${PREFIX}/usr/${LIBDIR} )
install(TARGETS glite_wmsui_LogWrapper 
  LIBRARY DESTINATION ${PREFIX}/usr/${LIBDIR} )
install(TARGETS glite_wmsui_UcWrapper 
  LIBRARY DESTINATION ${PREFIX}/usr/${LIBDIR} )
install(TARGETS glite_wmsui_AdWrapper 
  LIBRARY DESTINATION ${PREFIX}/usr/${LIBDIR} )
install(TARGETS glite_wmsui_SdWrapper 
  LIBRARY DESTINATION ${PREFIX}/usr/${LIBDIR} )

install(FILES 
  autogen/glite_wmsui_LbWrapper.py
  autogen/glite_wmsui_LogWrapper.py
  autogen/glite_wmsui_UcWrapper.py
  autogen/glite_wmsui_AdWrapper.py
  autogen/glite_wmsui_SdWrapper.py
  DESTINATION 
  ${PREFIX}/usr/${LIBDIR}/python
)

install(FILES 
  _glite_wmsui_UcWrapper.so.0.0.0
  _glite_wmsui_UcWrapper.so.0
  _glite_wmsui_UcWrapper.so
  _glite_wmsui_AdWrapper.so.0.0.0
  _glite_wmsui_AdWrapper.so.0
  _glite_wmsui_AdWrapper.so
  _glite_wmsui_SdWrapper.so.0.0.0
  _glite_wmsui_SdWrapper.so.0
  _glite_wmsui_SdWrapper.so
  _glite_wmsui_LogWrapper.so.0.0.0
  _glite_wmsui_LogWrapper.so.0
  _glite_wmsui_LogWrapper.so
  _glite_wmsui_LbWrapper.so.0.0.0
  _glite_wmsui_LbWrapper.so.0
  _glite_wmsui_LbWrapper.so
  DESTINATION 
  ${PREFIX}/usr/${LIBDIR}/
)

