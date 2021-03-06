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

// JDL
#include "glite/jdl/JobAd.h"
#include "glite/jdl/ExpDagAd.h"
#include "glite/jdl/JDLAttributes.h"
#include "glite/jdl/PrivateAttributes.h"  // RETRYCOUNT Default attributes
#include "glite/jdl/jdl_attributes.h"
#include "glite/jdl/JdlAttributeList.h"
#include "glite/jdl/RequestAdExceptions.h"
#include "glite/jdl/collectionad.h"
// HEADER
#include "adutils.h"
#include "excman.h"
#include "utils.h"
// JobId
#include "glite/jobid/JobId.h"

using namespace std ;
using namespace glite::jdl;

namespace glite {
namespace wms{
namespace client {
namespace utilities {

Log* logInfo;

const string DEFAULT_UI_CLIENTCONFILE	=	"glite_wmsui.conf";  //Used in utils as well
const string JDL_WMS_CLIENT		=	"WmsClient";
/******************************
*Default constructor
******************************/
AdUtils::AdUtils(Options *wmcOpts){
        // VerbosityLevel
        if (wmcOpts){
        	vbLevel = (LogLevel)wmcOpts->getVerbosityLevel();
        } else{
        	// default
		vbLevel = WMSLOG_WARNING;
        }
        logInfo = new Log (vbLevel);
}
/******************************
*Default destructor
******************************/
AdUtils::~AdUtils( ){ }

/******************************
*  AdUtils class methods:
*******************************/

/**
* fill the classad attributes, if missing, from another classad.
* The only attributes copied are the configuration ones
* @param source the Ad instance which will be used for the filling
* @param destination the destination Ad instance that will be filled
*/
void AdUtils::fillConfigAttributes(glite::jdl::Ad& source, glite::jdl::Ad &destination) {

	// Scan all the config attributes inside the AD and if missing
	// find them insede the source AD
	for(int counter = 0; counter < C_CONFIG_ATTRIBUTES; counter++) {

		// Retrieve the attribute name to be checked
		string attrName = configuAttributes[counter];

		// Check if the attribute is missing in the destination AD
		if(!destination.hasAttribute(attrName)) {
		
			// Check if the attribute is present in the source AD
			if(source.hasAttribute(attrName)) {
				// Add the Attribute
				destination.setAttributeExpr(attrName, source.delAttribute(attrName));
			}
		}
	}
}

void AdUtils::checkDeprecatedAttributes(glite::jdl::Ad &ad,
					const std::string &path)
{
  Log *logInfo = new Log(vbLevel);

  string deprecatedWarning = "";
  string attributeSeparator = "";

  std::vector<std::string> deprecatedAttributes;

  // Set all the deprecated attributes outside JDL Default Attributes section
  deprecatedAttributes.push_back(JDL::VIRTUAL_ORGANISATION);
  deprecatedAttributes.push_back(JDL::RETRYCOUNT);
  deprecatedAttributes.push_back(JDL::SHALLOWRETRYCOUNT);
  deprecatedAttributes.push_back(JDL::RANK);
  deprecatedAttributes.push_back(JDL::REQUIREMENTS);
  deprecatedAttributes.push_back(JDL::MYPROXY);
  deprecatedAttributes.push_back(JDL::JOB_PROVENANCE);
  deprecatedAttributes.push_back(JDL::LB_ADDRESS);
  deprecatedAttributes.push_back(JDL::ALLOW_ZIPPED_ISB);
  deprecatedAttributes.push_back(JDL::PU_FILE_ENABLE);

  // Show all the warnings for each deprecated attributes
  for(unsigned int counter = 0; counter < deprecatedAttributes.size(); counter++) 
  {

    // Check if the current deprecated attributes is present outside JDL Default Attributes section
    if(ad.hasAttribute(deprecatedAttributes[counter])) 
    {
      // Add the current attribute to the list of deprecated attributes
      deprecatedWarning += attributeSeparator + deprecatedAttributes[counter];
      
      // Set Comma as attribute separator
      attributeSeparator = ", ";
    }

  }
  
  // Show a warning message if deprecated attributes have been found
  if(!deprecatedWarning.empty()) {
      logInfo->print(WMS_DEBUG, "Configuration file: " + path + " -", deprecatedWarning + " attribute(s) no more supported outside JDL Default Attributes section \"JdlDefaultAttributes\"", true, true);
  }

}

string AdUtils::generateVoPath(string& voName){
        string conf;
        char* home = getenv("HOME");
        if (home == NULL)  {
                logInfo->print(WMS_INFO, "Unable to find user HOME environment variable: ","Not Set",true,false);
                return conf;
        } else {
                // new approach
                conf = string(home)+"/.glite/"+glite_wms_client_toLower(voName)+"/"+DEFAULT_UI_CLIENTCONFILE;
                if (Utils::isFile(conf)) {
                        return conf;
              	}
	}
	return conf;
}

void AdUtils::parseVo(voSrc src, std::string& voPath, std::string& voName){
	switch (src){
		case CERT_EXTENSION:
		case VO_OPT:
			// Only vo is provided, generate file name:
			if (voPath==""){
				voPath=this->generateVoPath(voName);
			}
			break;
		default:
			// voPAth already provided in all other cases
			break;
	}
	// Parse File Name and extrapolate VoName (cross-check)
	glite::jdl::Ad ad;
	try{
		ad.fromFile(voPath);
	}catch (AdSemanticPathException &exc){
		// The file Does not exist
		switch (src){
			case CERT_EXTENSION:
			case VO_OPT:
				voPath="";
				//In these cases vo file was autogenerated:
				// user config file may not be there
				return;
			break;
			case JDL_FILE:
				// If voName already read then
				// user config file may not be there
				if (voName!=""){return;}
				// otherwise continue and raise Exception...
			default:
				// In these cases vo file was specified by user
				throw WmsClientException(__FILE__,__LINE__,"AdUtils::parseVo",DEFAULT_ERR_CODE,
					"Empty Value","Unable to find file:\n"+voPath);
		}
	}catch (AdSyntaxException &exc){
		throw WmsClientException(__FILE__,__LINE__,"AdUtils::parseVo",DEFAULT_ERR_CODE,
					"File corrupted",voPath+": "+string(exc.what()));
	}
	// check if WmsClient section is specified (only when is not a JDL)
	if (ad.hasAttribute(JDL_WMS_CLIENT) && (src!=JDL_FILE)) {
		Ad app=ad.getAd(JDL_WMS_CLIENT);
		ad.clear();
  		ad = app;
	}
	// Check VIRTUAL ORGANISATION attribute:
	if (!ad.hasAttribute(JDL::VIRTUAL_ORGANISATION)){
		if (voName==""){
			// In these cases Vo name is impossible to track (neither in conf nor in certificate extension)
			throw WmsClientException(__FILE__,__LINE__,
				"AdUtils::parseVo",DEFAULT_ERR_CODE,"Empty Value",
				"Unable to find Mandatory VirtualOrganisation inside the file:\n"+voPath);
		}else {return;}
	}
	// Check VoName definitely present (refer to previous check)
	if (voName==""){
		voName=ad.getString(JDL::VIRTUAL_ORGANISATION);

	}
}
bool AdUtils::checkConfigurationAd(glite::jdl::Ad& ad, const string& path){
	try{
		ad.fromFile(path);
	}catch (RequestAdException &exc){
		
		return true;
	}
	if (ad.hasAttribute(JDL_WMS_CLIENT)) {
		Ad app=ad.getAd(JDL_WMS_CLIENT);
		ad.clear();
		ad=app;
	}
	// Check WMP/LB ENDPOINTS attribute:
	string ListOfStringAttrs []= {JDL_WMPROXY_ENDPOINT, JDL_LB_ENDPOINT};
	for (unsigned int i = 0 ; i < 2; i++){
		if (ad.hasAttribute(ListOfStringAttrs[i])){
			classad::ExprTree *tree = ad.lookUp(ListOfStringAttrs[i]);
			// This Expression MUST be of List type
			if (tree->GetKind() != classad::ExprTree::EXPR_LIST_NODE){
				vector< classad::ExprTree * > vect;
				vect.push_back(tree->Copy());
				ad.delAttribute(ListOfStringAttrs[i]);
				ad.setAttributeExpr (ListOfStringAttrs[i],new classad::ExprList(vect));
			}
		}
	}

	// Check for deprecated attributes
	checkDeprecatedAttributes(ad, path);
	
	return false;
}
/******************************
*  General Static Methods
*******************************/
glite::jdl::Ad* AdUtils::loadConfiguration(const std::string& pathUser, const std::string& pathDefault, const std::string& voName){
	glite::jdl::Ad adUser, adDefault, configAd;

	// Load the User configuration file if it has been set
	if (pathUser!=""){
	
		if (!checkConfigurationAd(adUser,pathUser)){
			if (vbLevel==WMSLOG_DEBUG){errMsg(WMS_DEBUG, "Loaded user configuration file:\n",pathUser,true);}
		}

		// Fill the configuration AD from the User AD
		fillConfigAttributes(adUser, configAd);
	}

	// Load the configuration file specified by the VO if it has been set
	if (pathDefault!=""){
	
		if(!checkConfigurationAd(adDefault,pathDefault)){
			if (vbLevel==WMSLOG_DEBUG){errMsg(WMS_DEBUG, "Loaded Vo specific configuration file:\n",pathDefault,true);}
		}

		// Fill the configuration AD from the AD specified by the VO 
		fillConfigAttributes(adDefault, configAd);
	}

	//Checks for the VO in the JdlDefaultAttributes section
	if (configAd.lookUp(JDL_DEFAULT_ATTRIBUTES)) {
		classad::ClassAd *defaultClassAd = static_cast<classad::ClassAd*>(configAd.delAttribute(JDL_DEFAULT_ATTRIBUTES));
		Ad *defaultAttrAd = new Ad(*defaultClassAd);
		delete (defaultClassAd );
		// VO overriding
		if(voName!=""){
			if (defaultAttrAd->hasAttribute(JDL::VIRTUAL_ORGANISATION)){
				string mismatchVo = defaultAttrAd->getString(JDL::VIRTUAL_ORGANISATION);
				defaultAttrAd->delAttribute(JDL::VIRTUAL_ORGANISATION);
				if ( voName.compare(mismatchVo) != 0 ) {
					errMsg(WMS_WARNING,"VirtualOrganisation Value Mismatch: \n","Configuration VirtualOrganisation value("+mismatchVo+") will be overriden by Proxy certificate value ("+voName+")",true);
				}
			}
			defaultAttrAd->setAttribute(JDL::VIRTUAL_ORGANISATION,voName);
		}
		configAd.setAttribute(JDL_DEFAULT_ATTRIBUTES, defaultAttrAd);
		delete ( defaultAttrAd );
	}

	if (!configAd.isSet()){
		if (vbLevel==WMSLOG_DEBUG){errMsg(WMS_WARNING, "Unable to load any configuration file properly","",true);}
	}else if (vbLevel==WMSLOG_DEBUG){
		errMsg(WMS_DEBUG, "Loaded Configuration values:",configAd.toLines(),true);
	}
	return new Ad(configAd);
}
std::vector<std::string> AdUtils::getUnknown(Ad* jdl){
	std::vector< std::string > attributes = jdl->attributes();
	std::vector< std::string >::iterator iter;
	glite::jdl::JdlAttributeList jdlAttribute;
	for (iter=attributes.begin();iter!=attributes.end() ; ++iter){
		if (jdlAttribute.findAttribute(*iter)){
			attributes.erase(iter);
		}
	}
	return attributes;
} 

// STATIC METHOD: set missing STRING value
void setMissing(glite::jdl::Ad* jdl,const string& attrName, const string& attrValue, bool force=false){
	if (attrValue!=""){
		if(!jdl->hasAttribute(attrName)){
			jdl->setAttribute(attrName,attrValue);
		}else if (force){
			// Override previous value
			jdl->delAttribute(attrName);
			jdl->setAttribute(attrName,attrValue);
		}
	}
}
// STATIC METHOD: set missing BOOL value
void setMissing(glite::jdl::Ad* jdl,const string& attrName, bool attrValue){
	if(   (!jdl->hasAttribute(attrName)) &&  attrValue ){
		// Set Default Attribute ONLY when TRUE
		jdl->setAttribute(attrName,attrValue);
	}
}
// STATIC METHOD: set missing INT value
// it ALWAYS set the value (even if it is 0)
void setMissing(glite::jdl::Ad* jdl,const string& attrName, int attrValue){
	if(!jdl->hasAttribute(attrName)){
		// Set Default Attribute ONLY when TRUE
		jdl->setAttribute(attrName,attrValue);
	}
}
// STATIC METHOD: set missing value from a conf Ad (of Any type)
void setMissingString(glite::jdl::Ad* jdl,const string& attrName, glite::jdl::Ad& confAd, bool force=false){
		if (confAd.hasAttribute(attrName)){
			string attrValue=confAd.getString(attrName);
			confAd.delAttribute(attrName);
			if(!jdl->hasAttribute(attrName)){
				jdl->setAttribute(attrName,attrValue);
			}else if (force){
				// Override previous value
				jdl->delAttribute(attrName);
				jdl->setAttribute(attrName,attrValue);
			}
		}
}


// STATIC METHOD: set missing INT value from a conf Ad
void setMissingInt(glite::jdl::Ad* jdl,const string& attrName, glite::jdl::Ad& confAd, bool force=false){
	if (confAd.hasAttribute(attrName)){
		int attrValue=confAd.getInt(attrName);
		confAd.delAttribute(attrName);
		if(!jdl->hasAttribute(attrName)){
			jdl->setAttribute(attrName,attrValue);
		}else if (force){
			// Override previous value
			jdl->delAttribute(attrName);
			jdl->setAttribute(attrName,attrValue);
		}
	}
}
// STATIC METHOD: set missing BOOL value from a conf Ad
void setMissingBool(glite::jdl::Ad* jdl,const string& attrName, glite::jdl::Ad& confAd, bool force=false){
	if (confAd.hasAttribute(attrName)){
		bool attrValue=confAd.getBool(attrName);
		confAd.delAttribute(attrName);
		if(   (!jdl->hasAttribute(attrName)) &&  attrValue ){
			// Set Default Attribute ONLY when TRUE
			jdl->setAttribute(attrName,attrValue);
		}else if (force){
			// Override previous value
			jdl->delAttribute(attrName);
			jdl->setAttribute(attrName,attrValue);
		}
	}
}
/******************
* JDL is still an AD (no type switched)
*******************/
void AdUtils::setDefaultValuesAd(glite::jdl::Ad* jdl,
	glite::jdl::Ad* conf,
	const std::string& pathOpt){
		
	if (!conf){return;}
	try{
		if (conf->hasAttribute(JDL_DEFAULT_ATTRIBUTES)){
			/* JDL ATTRIBUTE CUSTOM (NEW) APPROACH: */
			Ad confAd(conf->getAd(JDL_DEFAULT_ATTRIBUTES));
			// Default String Attrs: MYPROXYSERVER, VIRTUAL ORGANISATION, JOB_PROVENANCE
			setMissingString(jdl,JDL::MYPROXY,confAd);
			setMissingString(jdl,JDL::JOB_PROVENANCE,confAd);
			setMissingString(jdl,JDL::LB_ADDRESS,confAd);
			setMissingString(jdl,JDL::VIRTUAL_ORGANISATION,confAd,true);
			// Default Boolean Attributes:  ALLOW_ZIPPED_ISB ,PU_FILE_ENABLE
			setMissingBool(jdl,JDL::ALLOW_ZIPPED_ISB,confAd);
			setMissingBool(jdl,JDL::PU_FILE_ENABLE,confAd);
			// Other possible user values
			std::vector< std::string > attrs= confAd.attributes ();
			for (unsigned int i = 0 ; i<attrs.size(); i++){
				string attrName =attrs[i] ;
				// Exclude special attributes (will be parsed furtherly)
				// rank-req-retryC-shallowRC
				if( 	glite_wms_client_toLower(attrName)!=glite_wms_client_toLower(JDL::RANK) &&
					glite_wms_client_toLower(attrName)!=glite_wms_client_toLower(JDL::REQUIREMENTS) &&
					glite_wms_client_toLower(attrName)!=glite_wms_client_toLower(JDL::RETRYCOUNT) &&
					glite_wms_client_toLower(attrName)!=glite_wms_client_toLower(JDL::SHALLOWRETRYCOUNT)){
					if (!jdl->hasAttribute(attrs[i])){
						jdl->setAttributeExpr (attrs[i],confAd.lookUp(attrs[i])->Copy());
					}
				}
			}
			// Special [SHALLOW]RETRYCOUNT check:
			// This is the last check:
			// (avoid further checks: might be changing boolean type evaluation)
			if(jdl->hasAttribute(JDL::TYPE   , JDL_TYPE_COLLECTION) ||
			   jdl->hasAttribute(JDL::TYPE   , JDL_TYPE_DAG)){
				// COLLECTIONS and DAGS case:
				// (append special Resubmission private attributes)
				if(confAd.hasAttribute(JDL::RETRYCOUNT) && (!jdl->hasAttribute(JDLPrivate::DEFAULT_NODE_RETRYCOUNT))){
					jdl->setAttribute(JDLPrivate::DEFAULT_NODE_RETRYCOUNT,
						confAd.getInt(JDL::RETRYCOUNT));
				}
				if(confAd.hasAttribute(JDL::SHALLOWRETRYCOUNT) && (!jdl->hasAttribute(JDLPrivate::DEFAULT_NODE_SHALLOWRETRYCOUNT))){
					jdl->setAttribute(JDLPrivate::DEFAULT_NODE_SHALLOWRETRYCOUNT,
						confAd.getInt(JDL::SHALLOWRETRYCOUNT));
				}
			}else{
				// NORMAL JOBS: normal approach
				setMissingInt(jdl,JDL::RETRYCOUNT,confAd);
				setMissingInt(jdl,JDL::SHALLOWRETRYCOUNT,confAd);
			}
		}
	
	}catch(RequestAdException &exc){
		// Some classAd exception occurred
		throw WmsClientException(__FILE__,__LINE__,
			"AdUtils::setDefaultValuesAd",DEFAULT_ERR_CODE,
			"wrong conf attribute", exc.what());
	}
	// FURTHER CONF FILE specified by COMMAND-LINE
	try{
		if (!pathOpt.empty()){
			// JDL default attributes:
			Ad confPathAd;
			confPathAd.fromFile(pathOpt);
			jdl->merge(confPathAd);
		}
	}catch(RequestAdException &exc){
		// Some classAd exception occurred
		throw WmsClientException(__FILE__,__LINE__,
			"AdUtils::setDefaultValuesAd",DEFAULT_ERR_CODE,
			"Error while merging configuration file", exc.what());
	}
}



/******************
* JDL is a JobAd
*******************/
void AdUtils::setDefaultValues(glite::jdl::JobAd* jdl,
	glite::jdl::Ad* conf){
	if (!conf){return;}
	if (conf->hasAttribute(JDL_DEFAULT_ATTRIBUTES)){
		/* JDL ATTRIBUTE CUSTOM (NEW) APPROACH: */
		Ad confAd(conf->getAd(JDL_DEFAULT_ATTRIBUTES));
		// RANK
		if(confAd.hasAttribute(JDL::RANK)){
			jdl->setDefaultRank(confAd.lookUp(JDL::RANK));
		}
		// REQUIREMENTS
		if(confAd.hasAttribute(JDL::REQUIREMENTS)){
			jdl->setDefaultReq(confAd.lookUp(JDL::REQUIREMENTS));
		}
	}
}
/******************
* JDL is a Dag
*******************/
void AdUtils::setDefaultValues(glite::jdl::ExpDagAd* jdl,
	glite::jdl::Ad* conf){
	if (!conf){return;}
	if (conf->hasAttribute(JDL_DEFAULT_ATTRIBUTES)){
		/* JDL ATTRIBUTE CUSTOM (NEW) APPROACH: */
		Ad confAd(conf->getAd(JDL_DEFAULT_ATTRIBUTES));
		// RANK
		if(confAd.hasAttribute(JDL::RANK)){
			jdl->setDefaultRank(confAd.lookUp(JDL::RANK));
		}
		// REQUIREMENTS
		if(confAd.hasAttribute(JDL::REQUIREMENTS)){
			jdl->setDefaultReq(confAd.lookUp(JDL::REQUIREMENTS));
		}
	}
}
/******************
* JDL is a CollectionAd
*******************/
void AdUtils::setDefaultValues(glite::jdl::CollectionAd* jdl,
	glite::jdl::Ad* conf){
	if (!conf){return;}
	if (conf->hasAttribute(JDL_DEFAULT_ATTRIBUTES)){
		/* JDL ATTRIBUTE CUSTOM (NEW) APPROACH: */
		Ad confAd(conf->getAd(JDL_DEFAULT_ATTRIBUTES));
		// RANK
		if(confAd.hasAttribute(JDL::RANK)){
			jdl->setDefaultRank(confAd.lookUp(JDL::RANK));
		}
		// REQUIREMENTS
		if(confAd.hasAttribute(JDL::REQUIREMENTS)){
			jdl->setDefaultReq(confAd.lookUp(JDL::REQUIREMENTS));
		}
	}
}

/******************
* SOAP Timeout
*******************/
int  AdUtils::getSoapTimeout(const std::string& timeoutName, 
			     glite::jdl::Ad* p_conf)
{
	// Initialise the SOAP Timeout (0 == no timeout)
	int soap_timeout = 0;

	// Check if the configuration is valid
	if (NULL == p_conf) {
		// Throw the exception
		throw WmsClientException(__FILE__,__LINE__,"AdUtils::getSoapTimeout",DEFAULT_ERR_CODE,
			"Missing configuration","No configuration set by the caller");
	}

	// Check if the SOAP Class AD has been read
	if (p_conf->hasAttribute(JDL_SOAP_TIMEOUTS)) {

		// Initialise the searched Timeout 
		std::string searchedTimeout = "";

		/* Retrieve the SOAP Timeouts Class AD */
		Ad soapAd(p_conf->getAd(JDL_SOAP_TIMEOUTS));
		
		// Check if exists a global SOAP timeout
		if(soapAd.hasAttribute(SOAP_GLOBAL_TIMEOUT)) {
 			// Set the searched timeout
			searchedTimeout = SOAP_GLOBAL_TIMEOUT;
		}
		else {
		
			// Check if the needed Timeout has been set
			if(soapAd.hasAttribute(timeoutName)) {
				// Set the passed SOAP Timeout name as the searched one
				searchedTimeout = timeoutName;
			}
		}
		
		// Check if is present a SOAP timeout
		if(!searchedTimeout.empty()) {
			// Check if the attribute is of the correcy type
			if(soapAd.getType(searchedTimeout) != Ad::TYPE_INTEGER) {
			 	// Throw the exception
				throw WmsClientException(__FILE__,__LINE__,"AdUtils::getSoapTimeout",DEFAULT_ERR_CODE,
					"Invalid SOAP Timeout", "An invalid SOAP Timeout passed for " + searchedTimeout);
			}
		
			// Read the SOAP Timeout		
			soap_timeout = soapAd.getInt(searchedTimeout);
		}
		
	}
	
	// Return the SOAP Timeout
	return 	soap_timeout;
}

/***********************
*  JobId - Node mapping
************************/
std::map< std::string, std::string > AdUtils::getJobIdMap(const string& jdl){
	try{
		return ExpDagAd(jdl).getJobIdMap();
	}catch(RequestAdException &exc){
		return std::map< std::string, std::string >();
	}
}
std::string AdUtils::JobId2Node (const std::map< std::string, std::string > &map,
	glite::jobid::JobId jobid){
	if (map.size()){
		std::map<std::string,std::string >::const_iterator it = map.find(jobid.toString());
		if (it !=map.end()){
			return (*it).second;
		}
	}
	// If this point is reached, no mapping found. Simply return job unique string
	return jobid.unique();
}
} // glite
} // wms
} // client
} // utilities
