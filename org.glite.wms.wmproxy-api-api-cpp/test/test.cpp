#include "glite/wms/wmproxyapi/wmproxy_api.h"

using namespace std;
using namespace glite::wms::wmproxyapi;

const std::string errMsg (const glite::wms::wmproxyapi::BaseException &b_ex ){
        string meth = b_ex.methodName.c_str();
        string *errcode =b_ex.ErrorCode ;
        string *description = b_ex.Description ;
        string excmsg = "";
        //description
        if (description){ if (description->size() > 0 ){
                excmsg += *description+"\n" ; }
        }
        if (b_ex.FaultCause) {
                int size = b_ex.FaultCause->size( );
                for (int i = 0; i < size; i++) {
                        excmsg += (*b_ex.FaultCause)[i]+"\n";
                }
        }
        //method
        if (meth.size() > 0){excmsg +="Method: "+meth+"\n";}
        //errorcode
        if (errcode){if (errcode->size()>0){excmsg += "Error code: " + *errcode+"\n";}}
        return excmsg;
}




int main (int argc,char **argv){
	string endpoint = "";
	string version = "";
	string delegID = "delegTest";
	if (argc < 2  ){
		cout << "usage " << argv[0] << " <endpoint-url>\n";
		exit(-1);
	}
	endpoint = argv[1];
	ConfigContext *cxt = new ConfigContext("", endpoint, "");
	cout << "Testing endpoint: " << endpoint << "\n";
	cout << "==========================================\n\n";

	try{
		cout << "Connecting to " << endpoint << "\n\n";
		version = getVersion(cxt) ;
		cout << "WMProxy Version = " << version << "\n\n" ;
	} catch (BaseException &exc){
		cout << "Failed :(((\n";
		cout << "Exception: " << errMsg(exc)<< "\n";
	}
	cout << "Test: getProxyReq\n";
	cout << "----------------------------------\n\n";
	cout << "Calling getProxyReq service con delegationId: "<< delegID << "\n";
	try{
		cout << "Connecting to " << endpoint << "\n\n";
		string proxy = grstGetProxyReq(delegID, cxt);
		cout << "Success !!!\n";
		cout << "Proxy:\n" << proxy << "\n\n";
	} catch (BaseException &exc){
		cout << "Failed :(((\n";
		cout << "Exception: " << errMsg(exc)<< "\n";
	}
	/*
	cout << "Test: getNewProxyReq\n";
	cout << "----------------------------------\n\n";
	cout << "Calling getNewProxyReq service .....\n";
	try{
		cout << "Connecting to " << endpoint << "\n\n";
		ProxyReqStruct newProxy = getNewProxyReq(cxt);
		cout << "Success !!!\n";
		cout << "DelegationId = " << newProxy.delegationId << "\n";
		cout << "Proxy:\n" << newProxy.proxy << "\n\n";
	} catch (BaseException &exc){
	 	cout << "Failed :(((\n";
		cout << "Exception: " << errMsg(exc)<< "\n";
	 }
	 */
	cout << "Test: getProxyTerminationTime\n";
	cout << "----------------------------------\n\n";
	cout << "Calling getProxyTerminationTime service .....\n";
	try{
		cout << "Connecting to " << endpoint << "\n\n";
		int tt = getProxyTerminationTime(delegID, cxt);
		cout << "Success !!!\n";
		cout << "Termination time = " << tt << "\n";
	} catch (BaseException &exc){
	 	cout << "Failed :(((\n";
		cout << "Exception: " << errMsg(exc)<< "\n";
	 }
	cout << "Test: proxyDestroy\n";
	cout << "----------------------------------\n\n";
	cout << "Calling proxyDestroy service .....\n";
	try{
		cout << "Connecting to " << endpoint << "\n\n";
		proxyDestroy(delegID, cxt);
		cout << "Success !!!\n";
	} catch (BaseException &exc){
	 	cout << "Failed :(((\n";
		cout << "Exception: " << errMsg(exc)<< "\n";
	 }
	return 0;
}
