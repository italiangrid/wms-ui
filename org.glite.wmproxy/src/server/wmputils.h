/*
	Copyright (c) Members of the EGEE Collaboration. 2004.
	See http://public.eu-egee.org/partners/ for details on the copyright holders.
	For license conditions see the license file or http://www.eu-egee.org/license.html
*/

#ifndef GLITE_WMS_WMPROXY_WMPUTILS_H
#define GLITE_WMS_WMPROXY_WMPUTILS_H

void waitForSeconds(int seconds);
std::string to_filename(glite::wmsutils::jobid::JobId j,int level = 0,bool extended_path = true);

#endif // GLITE_WMS_WMPROXY_WMPUTILS_H
