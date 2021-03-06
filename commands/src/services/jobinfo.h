/*
Copyright (c) Members of the EGEE Collaboration. 2004.
See http://www.eu-egee.org/partners/ for details on the
copyright holders.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

/**
*        Copyright (c) Members of the EGEE Collaboration. 2004.
*        See http://public.eu-egee.org/partners/ for details on the copyright holders.
*        For license conditions see the license file or http://www.eu-egee.org/license.html
*
* 	Authors:	Alessandro Maraschini <alessandro.maraschini@datamat.it>
* 			Marco Sottilaro <marco.sottilaro@datamat.it>
*
*/

// 	$Id$


#ifndef GLITE_WMS_CLIENT_SERVICES_PROXYINFO_H
#define GLITE_WMS_CLIENT_SERVICES_PROXYINFO_H

// inheritance
#include "job.h"
// options utilities
#include "utilities/options_utils.h"
#include "utilities/utils.h"
// wmproxy API
#include "glite/wms/wmproxyapi/wmproxy_api.h"
// LOG
#include "utilities/logman.h"
//openSSL
#include <openssl/asn1.h>

namespace glite {
namespace wms {
namespace client {
namespace services {

class JobInfo : public Job {

	public :
		/**
		*	Default constructor
		*/
		JobInfo ( );
                /**
		*	Default destructor
		*/
		~JobInfo( ) ;
		/**
		*	Reads the command-line user arguments and sets all the class attributes
		*	@param argc number of the input arguments
		*	@param argv string of the input arguments
		*/
		void readOptions (int argc,char **argv) ;
                /**
		*	Retrieves the information
		*/
		void retrieveInfo ( ) ;
	private:
		const std::string adToLines (const std::string& jdl) ;
		const std::string timeString(const long &time) ;
		const std::string field (const std::string &label, const std::string &value);
		const std::string getDateString(const long &data);
		/**
		* Returns a string with the information of the delegated proxy retrieved by the server
		* @param info the struct containing the information to be printed
		*/
		const std::string printProxyInfo (glite::wms::wmproxyapi::ProxyInfoStructType info);
		/**
		* Specific user input option
		*/

		std::string m_inOpt ;
		/**
                *	Boolean input arguments
                */
		bool nointOpt ;
		bool jdlOpt ;
		bool origOpt ;
		bool proxyOpt ;
		/**
		* JobId
		*/
                std::string jobId ;
};
}}}} // ending namespaces
#endif //GLITE_WMS_CLIENT_SERVICES_PROXYINFO_H

