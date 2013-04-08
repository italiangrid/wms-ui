message("*\n*\n*\n* Configuring build for wms-ui.commands component\n*\n*\n*")

if(NOT DEFINED CLI_VERSION)
  message(WARNING " Undefined CLI_VERSION label. 0.0.0 will be used")
  set(CLI_VERSION "0.0.0")
endif(NOT DEFINED CLI_VERSION)

set(PROJECT_SOURCE_DIR "${MAIN_PROJECT_SOURCE_DIR}/commands/")

add_definitions(${CLASSAD_CFLAGS} -DWMP_CLT_SOFTWARE_VERSION="${CLI_VERSION}")

add_subdirectory(src)
add_subdirectory(doc)
add_subdirectory(config)
