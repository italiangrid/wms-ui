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


#include <iostream>
// utilities
#include "utilities/options_utils.h"
// exceptions
#include "utilities/excman.h"
// method
#include "services/jobperusal.h"

using namespace std ;
using namespace glite::wms::client::services ;
using namespace glite::wmsutils::exception;
/*
*	main
*/
int main (int argc,char **argv){
	JobPerusal job ;
        // reads the user options
	try {
		job.readOptions(argc, argv);
                job.jobPerusal( );
	} catch (Exception &exc) {
		job.excMsg("", exc, argv[0]);
		return 1;
	}
	return 0;
};
