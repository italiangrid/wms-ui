message("*\n*\n*\n* Configuring build for wms-ui.brokerinfo-access component\n*\n*\n*")

set(PROJECT_SOURCE_DIR "${MAIN_PROJECT_SOURCE_DIR}/brokerinfo-access/")

add_definitions( ${CLASSAD_CFLAGS} )

add_subdirectory(src)
add_subdirectory(doc)
