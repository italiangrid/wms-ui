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

/*
 * glite-wms-client-delegation.cpp text  fixture
 */

#include "glite/wmsutils/exception/Exception.h"
#include "glite/wmsutils/jobid/JobId.h"

#include "glite-wms-client-common.h"
#include "glite/wms/wmproxyapi/wmproxy_api_utilities.h"

using namespace std ;
// using namespace glite::wms::client::services ;
using namespace glite::wms::wmproxyapiutils;
using namespace glite::wms::wmproxyapi;

// TB inserted inside COMMON:
ConfigContext *cfs=NULL;
string wmproxyEndPoint ="https://ghemon.cnaf.infn.it:7443/glite_wms_wmproxy_server";
// string jobid ="https://ghemon.cnaf.infn.it:9000/8dyWbQFrf-Rb8EtIOt5aAA";  not needed
string protocol      = "gsiftp";
string delegationId  = "rask";

void initContext(){
	string proxyFile   = getProxyFile();
	string trustedCerts= getTrustedCert();
	cfs= new ConfigContext (proxyFile,wmproxyEndPoint,trustedCerts);
	string msg="VALUES USED: \n- Wmproxy= " +cfs->endpoint;
	msg+="\n- Proxy= "+ cfs->proxy_file;
	msg+="\n- trustedCerts= "+cfs->trusted_cert_dir;
// 	msg+="\n- jobid= "+jobid;
// 	if (jobid==""){
// 		cerr << "WARNING!!! JOBID NOT INITIALIZED!!!!!" << endl ;
// 	}
	title(msg);
}

// UNIT TEST
class DelegationTest : public CppUnit::TestFixture {
	CPPUNIT_TEST_SUITE( DelegationTest );
	CPPUNIT_TEST( testgetProxyReq );
	CPPUNIT_TEST( testDelegationNYI );
	CPPUNIT_TEST( testgetSomethingNYI);
	CPPUNIT_TEST( testOtherDelegationNYI );
	CPPUNIT_TEST_SUITE_END();
public:
	void testgetProxyReq() {
		grstGetProxyReq (delegationId,cfs);
	}
	void testDelegationNYI() {
		cerr << "testDelegationNYI NOT YET IMPLEMENTED" << endl ;
	}
	void testOtherDelegationNYI() {
		cerr << "testOtherDelegationNYI NOT YET IMPLEMENTED" << endl ;
	}
	void testgetSomethingNYI() {
		cerr << "testgetSomethingNYI NOT YET IMPLEMENTED" << endl ;
	}
};
int unitTest(){
	/*  CPP UNIT PART  */
	// Debug mode control:
	setWCDM(false);
	CppUnit::TestResult controller ;
	CppUnit::TestResultCollector result ;
	controller.addListener(&result);
	CppUnit::TestRunner runner ;
	/*   ACTUAL CPP UNIT NAME*/
	runner.addTest (DelegationTest::suite());
	runner.run (controller);
	CppUnit::TextOutputter outputter (&result , std::cerr);
	outputter.write();
	return   result.wasSuccessful()?0:1;
}

int main(int argc,char *argv[]){
	try{
		if( (argc<2) || (strcmp(argv[1],"--help")==0)|| (strcmp(argv[1],"help")==0)){
			cout << "Usage : " << argv[0] << " <test number> [<WMPROXY endpoint>] [<JobId>]" << endl ;
			cout <<"0) Unit Test!!!"<<endl; cout <<" - - - " << endl ;
			cout <<"1) testgetProxyReq"<<endl;
			cout <<"1) testDelegationNYI"<<endl;
			cout <<"3) testOtherDelegationNYI"<<endl;
			cout <<"4) testgetSomethingNYI"<<endl;
			return 0;
		}
		if (argc>2){wmproxyEndPoint=argv[2];}
		// if (argc>3){jobid=argv[3];}  // Additiona parameter not yet used
		DelegationTest test;
		initContext();
		switch (boost::lexical_cast<int>(argv[1])){
			case 0 : unitTest(); break;
			case 1: test.testgetProxyReq();     break;
			case 2: test.testDelegationNYI(); break;
			case 3: test.testOtherDelegationNYI();  break;
			case 4: test.testgetSomethingNYI();     break;
			default: break;
		}
		return 0;
	}catch  (glite::wmsutils::exception::Exception &exc){
		cout << "\nOperation not performed\n" ;
		cout <<"Stack...  " << exc.printStackTrace()<<  endl ;
		cout <<"Exc Name  " << exc.getExceptionName()<< endl;
		cout <<"What      " << exc.what()<< endl;
	}catch  (std::exception &exc){
		cout << "\nOperation not performed. std::exception found:\n" ;
		cout <<  exc.what() << endl ;
		cout << "\nUsage : " << argv[0] << " <test number> [<WMPROXY endpoint>] [<JobId>]" << endl ;
	}catch (...){
		cout << "\nOperation not performed. UNEXPECTED EXCEPTION RAISED"<< endl  ;
	}
	return 1;
};

