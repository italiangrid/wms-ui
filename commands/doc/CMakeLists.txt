
if(EXISTS "/usr/share/sgml/docbook/xsl-stylesheets/manpages/docbook.xsl")
  set(DOCBOOK_MAN_PAGES_STYLE "/usr/share/sgml/docbook/xsl-stylesheets/manpages/docbook.xsl")
endif(EXISTS "/usr/share/sgml/docbook/xsl-stylesheets/manpages/docbook.xsl")

if(EXISTS "/usr/share/xml/docbook/stylesheet/docbook-xsl/manpages/docbook.xsl")
  set(DOCBOOK_MAN_PAGES "/usr/share/xml/docbook/stylesheet/docbook-xsl/manpages/docbook.xsl")
endif(EXISTS "/usr/share/xml/docbook/stylesheet/docbook-xsl/manpages/docbook.xsl")

add_custom_command(OUTPUT ${PROJECT_SOURCE_DIR}/doc/glite-wms-job-status.1
  DEPENDS ${PROJECT_SOURCE_DIR}/doc/glite-wms-job-status.xml
  COMMAND xsltproc ${DOCBOOK_MAN_PAGES_STYLE} ${PROJECT_SOURCE_DIR}/doc/glite-wms-job-status.xml
  COMMAND gzip -c -f ${PROJECT_SOURCE_DIR}/doc/glite-wms-job-status.1 > ${PROJECT_SOURCE_DIR}/doc/glite-wms-job-status.1.gz
  )

add_custom_command(OUTPUT ${PROJECT_SOURCE_DIR}/doc/glite-wms-job-submit.1
  DEPENDS ${PROJECT_SOURCE_DIR}/doc/glite-wms-job-submit.xml
  COMMAND xsltproc ${DOCBOOK_MAN_PAGES_STYLE} ${PROJECT_SOURCE_DIR}/doc/glite-wms-job-submit.xml
  COMMAND gzip -c -f ${PROJECT_SOURCE_DIR}/doc/glite-wms-job-submit.1 > ${PROJECT_SOURCE_DIR}/doc/glite-wms-job-submit.1.gz
  )

add_custom_command(OUTPUT ${PROJECT_SOURCE_DIR}/doc/glite-wms-job-perusal.1
  DEPENDS ${PROJECT_SOURCE_DIR}/doc/glite-wms-job-perusal.xml
  COMMAND xsltproc ${DOCBOOK_MAN_PAGES_STYLE} ${PROJECT_SOURCE_DIR}/doc/glite-wms-job-perusal.xml
  COMMAND gzip -c -f ${PROJECT_SOURCE_DIR}/doc/glite-wms-job-perusal.1 > ${PROJECT_SOURCE_DIR}/doc/glite-wms-job-perusal.1.gz
  )

add_custom_command(OUTPUT ${PROJECT_SOURCE_DIR}/doc/glite-wms-job-output.1
  DEPENDS ${PROJECT_SOURCE_DIR}/doc/glite-wms-job-output.xml
  COMMAND xsltproc ${DOCBOOK_MAN_PAGES_STYLE} ${PROJECT_SOURCE_DIR}/doc/glite-wms-job-output.xml
  COMMAND gzip -c -f ${PROJECT_SOURCE_DIR}/doc/glite-wms-job-output.1 > ${PROJECT_SOURCE_DIR}/doc/glite-wms-job-output.1.gz
  )

add_custom_command(OUTPUT ${PROJECT_SOURCE_DIR}/doc/glite-wms-job-logging-info.1
  DEPENDS ${PROJECT_SOURCE_DIR}/doc/glite-wms-job-logging-info.xml
  COMMAND xsltproc ${DOCBOOK_MAN_PAGES_STYLE} ${PROJECT_SOURCE_DIR}/doc/glite-wms-job-logging-info.xml
  COMMAND gzip -c -f ${PROJECT_SOURCE_DIR}/doc/glite-wms-job-logging-info.1 > ${PROJECT_SOURCE_DIR}/doc/glite-wms-job-logging-info.1.gz
  )

add_custom_command(OUTPUT ${PROJECT_SOURCE_DIR}/doc/glite-wms-job-list-match.1
  DEPENDS ${PROJECT_SOURCE_DIR}/doc/glite-wms-job-list-match.xml
  COMMAND xsltproc ${DOCBOOK_MAN_PAGES_STYLE} ${PROJECT_SOURCE_DIR}/doc/glite-wms-job-list-match.xml
  COMMAND gzip -c -f ${PROJECT_SOURCE_DIR}/doc/glite-wms-job-list-match.1 > ${PROJECT_SOURCE_DIR}/doc/glite-wms-job-list-match.1.gz
  )

add_custom_command(OUTPUT ${PROJECT_SOURCE_DIR}/doc/glite-wms-job-info.1
  DEPENDS ${PROJECT_SOURCE_DIR}/doc/glite-wms-job-info.xml
  COMMAND xsltproc ${DOCBOOK_MAN_PAGES_STYLE} ${PROJECT_SOURCE_DIR}/doc/glite-wms-job-info.xml
  COMMAND gzip -c -f ${PROJECT_SOURCE_DIR}/doc/glite-wms-job-info.1 > ${PROJECT_SOURCE_DIR}/doc/glite-wms-job-info.1.gz
  )

add_custom_command(OUTPUT ${PROJECT_SOURCE_DIR}/doc/glite-wms-job-delegate-proxy.1
  DEPENDS ${PROJECT_SOURCE_DIR}/doc/glite-wms-job-delegate-proxy.xml
  COMMAND xsltproc ${DOCBOOK_MAN_PAGES_STYLE} ${PROJECT_SOURCE_DIR}/doc/glite-wms-job-delegate-proxy.xml
  COMMAND gzip -c -f ${PROJECT_SOURCE_DIR}/doc/glite-wms-job-delegate-proxy.1 > ${PROJECT_SOURCE_DIR}/doc/glite-wms-job-delegate-proxy.1.gz
  )

add_custom_command(OUTPUT ${PROJECT_SOURCE_DIR}/doc/glite-wms-job-cancel.1
  DEPENDS ${PROJECT_SOURCE_DIR}/doc/glite-wms-job-cancel.xml
  COMMAND xsltproc ${DOCBOOK_MAN_PAGES_STYLE} ${PROJECT_SOURCE_DIR}/doc/glite-wms-job-cancel.xml
  COMMAND gzip -c -f ${PROJECT_SOURCE_DIR}/doc/glite-wms-job-cancel.1 > ${PROJECT_SOURCE_DIR}/doc/glite-wms-job-cancel.1.gz
  )

add_custom_target(GENMANS ALL 
  DEPENDS
  glite-wms-job-cancel.1
  glite-wms-job-delegate-proxy.1
  glite-wms-job-info.1
  glite-wms-job-list-match.1
  glite-wms-job-logging-info.1
  glite-wms-job-output.1
  glite-wms-job-perusal.1
  glite-wms-job-status.1
  glite-wms-job-submit.1 )

install(FILES 
  glite-wms-job-cancel.1.gz
  glite-wms-job-delegate-proxy.1.gz
  glite-wms-job-info.1.gz
  glite-wms-job-list-match.1.gz
  glite-wms-job-logging-info.1.gz
  glite-wms-job-output.1.gz
  glite-wms-job-perusal.1.gz
  glite-wms-job-status.1.gz
  glite-wms-job-submit.1.gz
  DESTINATION
  ${PREFIX}/usr/share/man/man1
)

if(DEFINED INSTALL_BUILD)
install(FILES
  glite-wms-job-cancel.1.gz
  glite-wms-job-delegate-proxy.1.gz
  glite-wms-job-info.1.gz
  glite-wms-job-list-match.1.gz
  glite-wms-job-logging-info.1.gz
  glite-wms-job-output.1.gz
  glite-wms-job-perusal.1.gz
  glite-wms-job-status.1.gz
  glite-wms-job-submit.1.gz
  DESTINATION
  ${INSTALL_BUILD}/share/man/man1
)

endif(DEFINED INSTALL_BUILD)

