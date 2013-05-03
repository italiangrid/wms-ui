project(wms-ui C CXX)
cmake_minimum_required(VERSION 2.6)

if(NOT ${CMAKE_C_COMPILER} STREQUAL "CMAKE_C_COMPILER-NOTFOUND")
  set(OK_C_COMPILER "1")
endif(NOT ${CMAKE_C_COMPILER} STREQUAL "CMAKE_C_COMPILER-NOTFOUND")

if(NOT ${CMAKE_CXX_COMPILER} STREQUAL "CMAKE_CXX_COMPILER-NOTFOUND")
  set(OK_CXX_COMPILER "1")
endif(NOT ${CMAKE_CXX_COMPILER} STREQUAL "CMAKE_CXX_COMPILER-NOTFOUND")

EXEC_PROGRAM(/bin/date ARGS +'%a %b %d %Y' OUTPUT_VARIABLE DATE)

if(NOT DEFINED OS)
    message(FATAL_ERROR " Undefined Operating System name. Please use -DOS=<os_name>
to customize (choices are \"sl5\", \"sl6\" )")
endif(NOT DEFINED OS)

if(NOT DEFINED MOCKCFG)
    message(WARNING " Undefined mock configuration name. Please use
-DMOCKCFG=<name> for building in mock. Setting to 'none'")
endif(NOT DEFINED MOCKCFG)

if(NOT DEFINED AGE)
    message(WARNING " Undefined age number, selected 0 Please use -DAGE=<number> to customize")
    set(AGE "0")
endif(NOT DEFINED AGE)

if(NOT DEFINED PVER)
    message(WARNING " Undefined version number, selected 0.0.0. Please use
-DPVER=<version> to customize")
    set(PVER "0.0.0")
endif(NOT DEFINED PVER)

if(NOT DEFINED PREFIX)
    message(WARNING " Undefined prefix, selected /usr/local")
    set(PREFIX "/tmp/")
endif(NOT DEFINED PREFIX)

if(NOT DEFINED PKG_INSTALL_PREFIX)
    set(PKG_INSTALL_PREFIX "${PREFIX}")
endif(NOT DEFINED PKG_INSTALL_PREFIX)

if( ${CMAKE_VERSION} VERSION_LESS "2.8.2" )
    if( IS_DIRECTORY "/usr/lib64" )
        set(LIBDIR "lib64")
    else( IS_DIRECTORY "/usr/lib64" )
        set(LIBDIR "lib")
    endif( IS_DIRECTORY "/usr/lib64" )
else( ${CMAKE_VERSION} VERSION_LESS "2.8.2" )
    if( IS_DIRECTORY "/usr/lib64" AND NOT IS_SYMLINK "/usr/lib64" )
        set(LIBDIR "lib64")
    else( IS_DIRECTORY "/usr/lib64" AND NOT IS_SYMLINK "/usr/lib64" )
        set(LIBDIR "lib")
    endif( IS_DIRECTORY "/usr/lib64" AND NOT IS_SYMLINK "/usr/lib64" )
endif( ${CMAKE_VERSION} VERSION_LESS "2.8.2" )

IF (UNIX)
  # since it's unix-specific we will use bash
  ADD_CUSTOM_TARGET (distclean @echo cleaning ${FASTLIB_SOURCE_DIR}
for source distribution)
  ADD_CUSTOM_TARGET (RPM @echo packaging RPMS and SRPMS)
  ADD_CUSTOM_TARGET (MOCK @echo trying to build in the mock environment)

  ADD_CUSTOM_COMMAND(TARGET MOCK
    COMMAND mock ARGS -r ${MOCKCFG} rpmbuild/SRPMS/glite-wms-ui-${PVER}-${AGE}.${OS}.src.rpm
  )
  
  ADD_CUSTOM_COMMAND(TARGET RPM
    # this will create the stage area needed by rpm packager (just in case if
    # the uses didn't it already
    COMMAND make ARGS install
    COMMAND find ARGS ${PREFIX} -iname "*egg-info" -delete
    COMMAND rm ARGS -f /tmp/glite-wms-ui-${PVER}-${AGE}.${OS}.tar.gz
    COMMAND ${PROJECT_SOURCE_DIR}/maketarball.sh ARGS ${PVER} ${AGE} ${OS} "${PREFIX}"
${PROJECT_SOURCE_DIR}
    COMMAND mkdir ARGS -p rpmbuild/SPECS rpmbuild/SRPMS rpmbuild/BUILD rpmbuild/RPMS rpmbuild/SOURCES
    COMMAND mv ARGS /tmp/glite-wms-ui-${PVER}-${AGE}.${OS}.tar.gz rpmbuild/SOURCES/
    COMMAND sed ARGS -e 's/%{extversion}/${PVER}/g' -e 's/%{extage}/${AGE}/g'
-e 's/%{extdist}/${OS}/g' -e 's/%{extcdate}/${DATE}/g' -e 's/%{extclog}/Bug
fixing/g' < project/wms-ui.spec > rpmbuild/SPECS/wms-ui.spec
    COMMAND rpmbuild ARGS -ba --define "_topdir `pwd`/rpmbuild" --define
"extbuilddir ${PREFIX}" `pwd`/rpmbuild/SPECS/wms-ui.spec
  )
  
  ADD_CUSTOM_COMMAND(TARGET distclean
    COMMAND make ARGS clean
    COMMAND find ARGS ${FASTLIB_SOURCE_DIR} -iname CMakeCache.txt
-delete
    COMMAND find ARGS ${FASTLIB_SOURCE_DIR} -iname cmake_install.cmake
-delete
    COMMAND find ARGS ${FASTLIB_SOURCE_DIR} -iname Makefile -delete
    COMMAND find ARGS ${FASTLIB_SOURCE_DIR} -depth -type d -iname
CMakeFiles -exec rm -rf {} \;
    COMMAND rm ARGS -rf ${PREFIX}/stage install_manifest.txt
    COMMAND rm ARGS ./rpmbuild
    ./pkgconfig/brokerninfo-access.pc
    VERBATIM )
ENDIF(UNIX)


set(BINDIR "bin")

set(SBINDIR "sbin")

set(LIBEXEC "libexec")

set(ENV{PKG_CONFIG_PATH}
"${PREFIX}/usr/lib/pkgconfig/:${PREFIX}/stage/usr/lib64/pkgconfig/:$ENV{PKG_CONFIG_PATH}")

if(DEFINED ADDITIONAL_PKG_CONFIG_PATH)
  set(ENV{PKG_CONFIG_PATH}
"${ADDITIONAL_PKG_CONFIG_PATH}:$ENV{PKG_CONFIG_PATH}")
endif(DEFINED ADDITIONAL_PKG_CONFIG_PATH)

set(BOOST_ROOT "/usr/lib64:/usr/lib")

find_package(PkgConfig REQUIRED)
find_package(Perl REQUIRED)
find_program(Make REQUIRED)
find_package(Boost REQUIRED COMPONENTS filesystem date_time filesystem regex
thread program_options)
find_package(Doxygen)
pkg_check_modules(GRIDSITE_GLOBUS REQUIRED gridsite-globus)
pkg_check_modules(GRIDSITE_OPENSSL REQUIRED gridsite-openssl)
pkg_check_modules(GSOAP REQUIRED gsoap)
pkg_check_modules(GSOAP_PLUSPLUS REQUIRED gsoap++)
pkg_check_modules(GSOAP_SSL REQUIRED gsoapssl)
pkg_check_modules(GSOAP_SSL_PLUSPLUS REQUIRED gsoapssl++)
pkg_check_modules(VOMS REQUIRED voms-2.0)
pkg_check_modules(JDL_API_CPP REQUIRED jdl-api-cpp)
pkg_check_modules(CLASSAD REQUIRED emi-classad-ns)
pkg_check_modules(JOBID REQUIRED emi-jobid)
pkg_check_modules(LB_CLIENT REQUIRED emi-lb-client)
pkg_check_modules(LB_CLIENTPP REQUIRED emi-lb-clientpp)
pkg_check_modules(CLASSAD_UTILS REQUIRED classad-utils)
pkg_check_modules(JOBMAN_EXCEPTION REQUIRED jobman-exception)
pkg_check_modules(VOMS REQUIRED voms-2.0)
pkg_check_modules(GLOBUS_GSS_ASSIST REQUIRED globus-gss-assist)
pkg_check_modules(LIBXSLT REQUIRED libxslt)
pkg_check_modules(WMSAPI REQUIRED wms-api)
pkg_check_modules(CPPUNIT REQUIRED cppunit)
pkg_check_modules(LIBTAR REQUIRED emi-tar)
pkg_check_modules(CARES REQUIRED libcares)
pkg_check_modules(GLOBUS_GSSAPI_ERROR REQUIRED globus-gssapi-error)
pkg_check_modules(GLOBUS_FTP_CLIENT REQUIRED globus-ftp-client)
pkg_check_modules(GLOBUS_FTP_CONTROL REQUIRED globus-ftp-control)

set(OK_DOCBOOK "1")

if(NOT EXISTS "/usr/share/sgml/docbook/xsl-stylesheets/manpages/docbook.xsl"
   AND NOT EXISTS
"/usr/share/xml/docbook/stylesheet/docbook-xsl/manpages/docbook.xsl")
  message(WARNING "*** Missing docbook xsl style sheet required to parse
man pages")
  set(OK_DOCBOOK)
endif(NOT EXISTS
"/usr/share/sgml/docbook/xsl-stylesheets/manpages/docbook.xsl"
   AND NOT EXISTS
"/usr/share/xml/docbook/stylesheet/docbook-xsl/manpages/docbook.xsl")

if(NOT DEFINED OK_DOCBOOK)
  message(WARNING " *** Missing docbook style sheet")
endif(NOT DEFINED OK_DOCBOOK)

if(EXISTS "/usr/share/sgml/docbook/xsl-stylesheets/manpages/docbook.xsl")
  set(DOCBOOK_MAN_PAGES_STYLE
"/usr/share/sgml/docbook/xsl-stylesheets/manpages/docbook.xsl")
endif(EXISTS "/usr/share/sgml/docbook/xsl-stylesheets/manpages/docbook.xsl")

if(EXISTS
"/usr/share/xml/docbook/stylesheet/docbook-xsl/manpages/docbook.xsl")
  set(DOCBOOK_MAN_PAGES_STYLE
"/usr/share/xml/docbook/stylesheet/docbook-xsl/manpages/docbook.xsl")
endif(EXISTS
"/usr/share/xml/docbook/stylesheet/docbook-xsl/manpages/docbook.xsl")

FIND_PROGRAM(PYTHON_EXECUTABLE
	NAMES python
	PATHS /usr/bin /usr/local/bin /opt/local/bin 
	)

if(NOT "${PYTHON_EXECUTABLE}" STREQUAL "PYTHON_EXECUTABLE-NOTFOUND"  )
  set(OKPYTHON "1")
endif(NOT "${PYTHON_EXECUTABLE}" STREQUAL "PYTHON_EXECUTABLE-NOTFOUND"  )

if(EXISTS "/usr/bin/swig" 
    OR EXISTS "/bin/swig"
    OR EXISTS "/usr/local/bin/swig"
    OR EXISTS "/opt/local/bin/swig")
  set(SWIG_OK 1)
  else(EXISTS "/usr/bin/swig" 
    OR EXISTS "/bin/swig"
    OR EXISTS "/usr/local/bin/swig"
    OR EXISTS "/opt/local/bin/swig")
  message(WARNING "\n\n*** ERROR: Swig installation missing. Cannot build
this component\n\n")
endif(EXISTS "/usr/bin/swig" 
    OR EXISTS "/bin/swig"
    OR EXISTS "/usr/local/bin/swig"
    OR EXISTS "/opt/local/bin/swig")

set(PARAM "-c 'import platform\; a=platform.python_version().split(\".\")\; print \".\".join([a[0], a[1]])'"
)

EXEC_PROGRAM(python ARGS
  ${PARAM}
  OUTPUT_VARIABLE
  PYTHON_VERSION
)

set(PYTHON_INCLUDE "/usr/include/python${PYTHON_VERSION}")

if(NOT DEFINED OKPYTHON)
  message(WARNING " *** Missing python interpreter")
endif(NOT DEFINED OKPYTHON)

find_file(RPMBUILD "rpmbuild"
	  message(WARNING " *** Missing rpmbuild")
          PATHS "/usr/bin")

find_file(CHRPATH "chrpath"
          message(WARNING " *** Missing chrpath")
          PATHS "/usr/bin")

#message(WARNING " ---> CHRPATH=${CHRPATH}")
#message(WARNING " ---> RPMBUILD=${RPMBUILD}")
#message(WARNING " ---> GRIDSITE_GLOBUS_FOUND=${GRIDSITE_GLOBUS_FOUND}")
#message(WARNING " ---> GRIDSITE_OPENSSL_FOUND=${GRIDSITE_OPENSSL_FOUND}")
#message(WARNING " ---> OK_C_COMPILER=${OK_C_COMPILER}")
#message(WARNING " ---> OK_CXX_COMPILER=${OK_CXX_COMPILER}")
#message(WARNING " ---> VOMS_FOUND=${VOMS_FOUND}")
#message(WARNING " ---> OKPYTHON=${OKPYTHON}")
#message(WARNING " ---> GSOAP_FOUND=${GSOAP_FOUND}")
#message(WARNING " ---> OK_DOCBOOK=${OK_DOCBOOK}")
#message(WARNING " ---> GSOAP_PLUSPLUS_FOUND=${GSOAP_PLUSPLUS_FOUND}")
#message(WARNING " ---> GSOAP_SSL_FOUND=${GSOAP_SSL_FOUND}")
#message(WARNING " ---> GSOAP_SSL_PLUSPLUS_FOUND=${GSOAP_SSL_PLUSPLUS_FOUND}")
#message(WARNING " ---> Boost_FOUND=${Boost_FOUND}")
#message(WARNING " ---> JDL_API_CPP_FOUND=${JDL_API_CPP_FOUND}")
#message(WARNING " ---> CLASSAD_FOUND=${CLASSAD_FOUND}")
#message(WARNING " ---> JOBID_FOUND=${JOBID_FOUND}")
#message(WARNING " ---> LB_CLIENT_FOUND=${LB_CLIENT_FOUND}")
#message(WARNING " ---> LB_CLIENTPP_FOUND=${LB_CLIENTPP_FOUND}")
#message(WARNING " ---> CLASSAD_UTILS_FOUND=${CLASSAD_UTILS_FOUND}")
#message(WARNING " ---> JOBMAN_EXCEPTION_FOUND=${JOBMAN_EXCEPTION_FOUND}")
#message(WARNING " ---> GLOBUS_GSS_ASSIST_FOUND=${GLOBUS_GSS_ASSIST_FOUND}")
#message(WARNING " ---> LIBXSLT_FOUND=${LIBXSLT_FOUND}")
#message(WARNING " ---> WMSAPI_FOUND=${WMSAPI_FOUND}")
#message(WARNING " ---> Doxygen=${DOXYGEN_FOUND}" )
#message(WARNING " ---> OKPYTHON=${OKPYTHON}" )
#message(WARNING " ---> SWIG_OK=${SWIG_OK}" )
#message(WARNING " ---> CPPUNIT=${CPPUNIT_FOUND}" )

if( "${DEPS}" STREQUAL "yes")
if( NOT(CHRPATH AND RPMBUILD AND GRIDSITE_GLOBUS_FOUND AND GRIDSITE_OPENSSL_FOUND AND
OK_C_COMPILER AND OK_CXX_COMPILER AND VOMS_FOUND AND OKPYTHON AND
GSOAP_FOUND AND OK_DOCBOOK AND DOXYGEN_FOUND AND CPPUNIT_FOUND AND
GSOAP_PLUSPLUS_FOUND AND SWIG_OK AND CARES_FOUND AND LIBTAR_FOUND AND GLOBUS_GSSAPI_ERROR_FOUND AND GLOBUS_FTP_CLIENT_FOUND AND GLOBUS_FTP_CONTROL_FOUND AND
GSOAP_SSL_FOUND AND
GSOAP_SSL_PLUSPLUS_FOUND AND Boost_FOUND AND
JDL_API_CPP_FOUND AND CLASSAD_FOUND AND JOBID_FOUND AND LB_CLIENT_FOUND AND
LB_CLIENTPP_FOUND AND CLASSAD_UTILS_FOUND AND JOBMAN_EXCEPTION_FOUND AND
GLOBUS_GSS_ASSIST_FOUND AND LIBXSLT_FOUND AND WMSAPI_FOUND
) ) 
  message( WARNING " *** Some dependencies are missing. Will try to install
them via yum. Please intert your password" )
  set(RERUN "1")
  EXEC_PROGRAM(sudo ARGS yum -y install 
doxygen
python
python-devel
swig
boost
boost-devel
chrpath
emi-pkgconfig-compat
gridsite
gridsite-devel
gsoap
gsoap-devel
rpm-build
voms
voms-devel
glite-jobid-api-cpp-devel
glite-jobid-api-c-devel
glite-lb-client-devel
glite-lb-common-devel
glite-lbjp-common-gss-devel
libxslt-devel
docbook-style-xsl
libxslt
classads-devel
glite-wms-utils-classad
glite-wms-utils-exception-devel
glite-wms-api
globus-gss-assist-devel
glite-jdl-api-cpp-devel
glite-wms-api
cppunit-devel
libtar-devel
c-ares-devel
globus-gssapi-error-devel
globus-ftp-client-devel
globus-ftp-control-devel
gcc
gcc-c++
libstdc++
make
  )
endif(NOT(CHRPATH AND RPMBUILD AND GRIDSITE_GLOBUS_FOUND AND
GRIDSITE_OPENSSL_FOUND AND
OK_C_COMPILER AND OK_CXX_COMPILER AND VOMS_FOUND AND OKPYTHON AND
GSOAP_FOUND AND OK_DOCBOOK AND DOXYGEN_FOUND AND CPPUNIT_FOUND AND
GSOAP_PLUSPLUS_FOUND AND SWIG_OK AND CARES_FOUND AND LIBTAR_FOUND AND GLOBUS_GSSAPI_ERROR_FOUND AND GLOBUS_FTP_CLIENT_FOUND AND GLOBUS_FTP_CONTROL_FOUND AND
GSOAP_SSL_FOUND AND
GSOAP_SSL_PLUSPLUS_FOUND AND Boost_FOUND AND
JDL_API_CPP_FOUND AND CLASSAD_FOUND AND JOBID_FOUND AND LB_CLIENT_FOUND AND
LB_CLIENTPP_FOUND AND CLASSAD_UTILS_FOUND AND JOBMAN_EXCEPTION_FOUND AND
GLOBUS_GSS_ASSIST_FOUND AND LIBXSLT_FOUND AND WMSAPI_FOUND
))
endif( "${DEPS}" STREQUAL "yes")

add_definitions( -Wall -pipe -DHAVE_STRINGSTREAM=1 -Wno-deprecated)

if(DEFINED BUILD_DBG)
  add_definitions( -g -O0 )
else(DEFINED BUILD_DBG)
  add_definitions( -O2 )
endif(DEFINED BUILD_DBG)

set(MAIN_PROJECT_SOURCE_DIR "${PROJECT_SOURCE_DIR}")

#if(NOT DEFINED SYSCONFIGDIR)
set(SYSCONFIGDIR "${PREFIX}/etc/")
#endif(NOT DEFINED SYSCONFIGDIR)

install(FILES
        LICENSE CHANGES
        DESTINATION ${PREFIX}/usr/share/doc/glite-wms-ui-${PVER}/
)

add_subdirectory(commands)
add_subdirectory(brokerinfo-access)
add_subdirectory(api-python)
#add_subdirectory(wmproxy-api-cpp)
add_subdirectory(pkgconfig)
if(RERUN)
message(WARNING " Please re-run cmake once again...")
endif(RERUN)
