message("*\n*\n*\n* Configuring build for wms-ui.apy-python component\n*\n*\n*")

set(PROJECT_SOURCE_DIR "${MAIN_PROJECT_SOURCE_DIR}/apy-python/")

add_definitions( -I${PYTHON_INCLUDES} ${CLASSAD_CFLAGS} ${PYTHON_INCLUDES} ${JOBMAN_EXCEPTION_CFLAGS})

add_subdirectory(src)
