configure_file(${CMAKE_CURRENT_SOURCE_DIR}/glite_wmsui_cmd_err.conf.in
               ${CMAKE_CURRENT_BINARY_DIR}/glite_wmsui_cmd_err.conf
               @ONLY)

#configure_file(${CMAKE_CURRENT_SOURCE_DIR}/glite_wmsui_cmd_var.conf
#               ${CMAKE_CURRENT_BINARY_DIR}/glite_wmsui_cmd_var.conf.template
#               @ONLY)



install(FILES 
  ${CMAKE_CURRENT_BINARY_DIR}/glite_wmsui_cmd_var.conf
  DESTINATION ${SYSCONFIGDIR}/
  )


install(FILES 
  ${CMAKE_CURRENT_BINARY_DIR}/glite_wmsui_cmd_help.conf
  DESTINATION ${SYSCONFIGDIR}/
  )


install(FILES 
  ${CMAKE_CURRENT_BINARY_DIR}/glite_wmsui_cmd_err.conf
  DESTINATION ${SYSCONFIGDIR}/
  )

