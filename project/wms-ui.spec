Summary: User Interface for the WMS
Name: glite-wms-ui
Version: %{extversion}
Release: %{extage}.%{extdist}
License: ASL 2.0
Vendor: EMI
URL: http://glite.cern.ch/
Group: System Environment/Libraries
BuildArch: %{_arch}
BuildRequires: %{!?extbuilddir: glite-wms-wmproxy-api-cpp-devel,glite-wms-wmproxy-api-cpp,} chrpath, cmake
BuildRequires: %{!?extbuilddir: gridsite-devel, glite-wms-utils-exception-devel,} classads-devel
BuildRequires: %{!?extbuilddir: glite-jobid-api-cpp-devel, glite-jdl-api-cpp-devel,} boost-devel
BuildRequires: %{!?extbuilddir: glite-lb-client-devel, } libtar-devel, swig
BuildRequires: %{!?extbuilddir: voms-devel, }  zlib-devel, doxygen, docbook-style-xsl
BuildRequires: %{!?extbuilddir:glite-build-common-cpp, } docbook-style-xsl, libxslt, c-ares-devel, libxslt-devel
BuildRequires: globus-common-devel, globus-callout-devel, globus-openssl-devel, python-devel
BuildRequires: globus-openssl-module-devel, globus-gsi-callback-devel, globus-gsi-cert-utils-devel
BuildRequires: globus-gsi-credential-devel, globus-gsi-openssl-error-devel, globus-gsi-proxy-core-devel
BuildRequires: globus-gsi-proxy-ssl-devel, globus-gsi-sysconfig-devel,globus-gssapi-error-devel
BuildRequires: globus-gssapi-gsi-devel, globus-gss-assist-devel, cppunit-devel
BuildRequires: globus-ftp-client-devel, globus-ftp-control-devel, libxml2-devel, emi-pkgconfig-compat
Requires: globus-gass-copy-progs, swig, python
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)
AutoReqProv: yes
Source: %{name}-%{version}-%{release}.tar.gz

%global debug_package %{nil}

%if ! (0%{?fedora} > 12 || 0%{?rhel} > 5)
%{!?python_sitelib: %global python_sitelib %(%{__python} -c "from distutils.sysconfig import get_python_lib; print(get_python_lib())")}
%{!?python_sitearch: %global python_sitearch %(%{__python} -c "from distutils.sysconfig import get_python_lib; print(get_python_lib(1))")}
%endif

%description
Command line user interface for the WMS

%prep
%{!?extbuilddir:%define extbuilddir "-"}
%setup -c -q

%build
if test "x%{extbuilddir}" == "x-" ; then
  cmake -DPREFIX:string=%{buildroot}/usr -DPVER:string=%{version} .
  make
fi

%install
rm -rf %{buildroot}
mkdir -p %{buildroot}
if test "x%{extbuilddir}" == "x-" ; then
  make install
  cp -R brokerinfo-access/doc/autodoc/html %{buildroot}/%{_docdir}/%{name}-%{version}
else
  cp -R %{extbuilddir}/* %{buildroot}
fi
chrpath --delete %{buildroot}/usr/bin/glite-wms-job-cancel
chrpath --delete %{buildroot}/usr/bin/glite-wms-job-delegate-proxy
chrpath --delete %{buildroot}/usr/bin/glite-wms-job-info
chrpath --delete %{buildroot}/usr/bin/glite-wms-job-list-match
chrpath --delete %{buildroot}/usr/bin/glite-wms-job-output
chrpath --delete %{buildroot}/usr/bin/glite-wms-job-perusal
chrpath --delete %{buildroot}/usr/bin/glite-wms-job-submit
chrpath --delete %{buildroot}/usr/lib64/*.so.0.0.0
strip -s %{buildroot}/usr/lib64/*.so.0.0.0

%clean
rm -rf %{buildroot}

%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig

%files
%defattr(-,root,root)
%config(noreplace) /etc/glite_wmsui_cmd_*
%dir /usr/share/doc/glite-wms-ui-%{version}/
%doc /usr/share/doc/glite-wms-ui-%{version}/LICENSE
%doc /usr/share/doc/glite-wms-ui-%{version}/CHANGES
%doc /usr/share/man/man1/*.1.gz
/usr/bin/glite-wms-job-*
/usr/bin/glite-brokerinfo
/usr/lib64/libglite*.so*
/usr/lib64/_glite_wmsui_*.so*
%dir /usr/lib64/python
/usr/lib64/python/glite_wmsui_AdWrapper.*
/usr/lib64/python/glite_wmsui_SdWrapper.*
/usr/lib64/python/glite_wmsui_LbWrapper.*
/usr/lib64/python/glite_wmsui_LogWrapper.*
/usr/lib64/python/glite_wmsui_UcWrapper.*
/usr/lib64/python/wmsui_api.*
/usr/lib64/python/wmsui_checks.*
/usr/lib64/python/wmsui_listener.*
/usr/lib64/python/wmsui_utils.*
%dir /usr/include/glite/
%dir /usr/include/glite/wms/
%dir /usr/include/glite/wms/brokerinfo-access/
/usr/include/glite/wms/brokerinfo-access/*.h
/usr/lib64/pkgconfig/brokerinfo-access.pc
/usr/lib64/libglite-brokerinfo.so

%package doc
Summary: Documentation files for the brokerinfo access component
Group: Documentation

%description doc
Documentation files for the brokerinfo access component

%files doc
%defattr(-,root,root)
%dir %{_docdir}/brokerinfo-access-%{version}/html
%doc %{_docdir}/brokerinfo-access-%{version}/html/*.html
%doc %{_docdir}/brokerinfo-access-%{version}/html/*.css
%doc %{_docdir}/brokerinfo-access-%{version}/html/*.png
%doc %{_docdir}/brokerinfo-access-%{version}/html/*.gif

%changelog
* %{extcdate} WMS group <wms-support@lists.infn.it> - %{extversion}-%{extage}.%{extdist}
- %{extclog}
