// JDL
#include "glite/jdl/Ad.h"
#include "glite/jdl/JobAd.h"
#include "glite/jdl/ExpDagAd.h"
#include "glite/jdl/JDLAttributes.h"
#include "glite/jdl/PrivateAttributes.h"  // RETRYCOUNT Default attributes
#include "glite/jdl/jdl_attributes.h"
#include "glite/jdl/JdlAttributeList.h"
#include "glite/jdl/RequestAdExceptions.h"
#include "glite/jdl/collectionad.h"
#include "glite/wms/common/configuration/WMCConfiguration.h" // Configuration
// HEADER
#include "adutils.h"
#include "excman.h"
#include "utils.h"
using namespace std ;
using namespace glite::jdl ;

namespace glite {
namespace wms{
namespace client {
namespace utilities {
const string DEFAULT_UI_CLIENTCONFILE	=	"glite_wmsclient.conf";  //Used in utils as well
const string DEFAULT_UI_CONFILE			=	"glite_wms.conf";  // kept for compatibility purpose with older versions
const string JDL_WMS_CLIENT		=	"WmsClient";
const string JDL_WMPROXY_ENDPOINT	=	"WmProxyEndPoints";
const string JDL_LB_ENDPOINT		=	"LBEndPoints";
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

}
/******************************
*Default destructor
******************************/
AdUtils::~AdUtils( ){ }

/******************************
*  AdUtils class methods:
*******************************/
string AdUtils::generateVoPath(string& voName){
	//new approach
	string conf = string(getenv("HOME"))+"/.glite/"+glite_wms_client_toLower(voName)+"/"+DEFAULT_UI_CLIENTCONFILE;
	if (Utils::isFile(conf))
		return conf;
	else
	//checking for old configuration file
		return string(getenv("HOME"))+"/.glite/"+glite_wms_client_toLower(voName)+"/"+DEFAULT_UI_CONFILE;
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
		cout <<"AdUtils::parseVo dbg00 Checking VO: " << endl ;
		voName=ad.getString(JDL::VIRTUAL_ORGANISATION);

	}else if ( (glite_wms_client_toLower(voName))
		!=glite_wms_client_toLower(ad.getString(JDL::VIRTUAL_ORGANISATION)) ){
		// VO Mismatch problem
		if (vbLevel==WMSLOG_DEBUG){
			errMsg(WMS_WARNING,"VirtualOrganisation value will be overriden by ",voName,true);
		}

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
	return false;
}
/******************************
*  General Static Methods
*******************************/
classad::ClassAd* AdUtils::loadConfiguration(const std::string& pathUser ,
	const std::string& pathDefault, const std::string& pathGeneral,
	const std::string& voName){
	glite::jdl::Ad adUser, adDefault, adGeneral;
	// Load ad from file (if necessary)
	if (pathGeneral!=""){
		if (!checkConfigurationAd(adGeneral,pathGeneral)){
			if (vbLevel==WMSLOG_DEBUG){errMsg(WMS_DEBUG, "Loaded generic configuration file:\n",pathGeneral,true);}
		}
	}	
	if (pathDefault!=""){
		if(!checkConfigurationAd(adDefault,pathDefault)){
			if (vbLevel==WMSLOG_DEBUG){errMsg(WMS_DEBUG, "Loaded Vo specific configuration file:\n",pathDefault,true);}
		}
	}
	if (pathUser!=""){
		if (!checkConfigurationAd(adUser,pathUser)){
			if (vbLevel==WMSLOG_DEBUG){errMsg(WMS_DEBUG, "Loaded user configuration file:\n",pathUser,true);}
		}
	}

	// Merge all configuration file found
	adGeneral.merge(adDefault);
	adGeneral.merge(adUser);
	// VO overriding
	if(voName!=""){
		if (adGeneral.hasAttribute(JDL::VIRTUAL_ORGANISATION)){
			adGeneral.delAttribute(JDL::VIRTUAL_ORGANISATION);
		}
		adGeneral.setAttribute(JDL::VIRTUAL_ORGANISATION,voName);
	}
	if (!adGeneral.isSet()){
		if (vbLevel==WMSLOG_DEBUG){errMsg(WMS_WARNING, "Unable to load any configuration file properly","",true);}
	}else if (vbLevel==WMSLOG_DEBUG){
		errMsg(WMS_DEBUG, "Loaded Configuration values:",adGeneral.toLines(),true);
	}
	return adGeneral.ad();
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

// TBD: Utilise template class for similar methods
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
	glite::wms::common::configuration::WMCConfiguration* conf,
	std::string* pathOpt){
	if (!conf){return;}
	try{
		if (conf->jdl_default_attributes()){
			/* JDL ATTRIBUTE CUSTOM (NEW) APPROACH: */
			Ad confAd(*(conf->jdl_default_attributes()));
			// Default String Attrs: HLRLOCATION, MYPROXYSERVER, VIRTUAL ORGANISATION, JOB_PROVENANCE
			setMissingString(jdl,JDL::MYPROXY,confAd);
			setMissingString(jdl,JDL::HLR_LOCATION,confAd);
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
				if(confAd.hasAttribute(JDL::RETRYCOUNT)){
					jdl->setAttribute(JDLPrivate::DEFAULT_NODE_RETRYCOUNT,
						confAd.getInt(JDL::RETRYCOUNT));
				}
				if(confAd.hasAttribute(JDL::SHALLOWRETRYCOUNT)){
					jdl->setAttribute(JDLPrivate::DEFAULT_NODE_SHALLOWRETRYCOUNT,
						confAd.getInt(JDL::SHALLOWRETRYCOUNT));
				}
			}else{
				// NORMAL JOBS: normal approach
				setMissingInt(jdl,JDL::RETRYCOUNT,confAd);
				setMissingInt(jdl,JDL::SHALLOWRETRYCOUNT,confAd);
			}
		}else{
			/* ALL ATTRIBUTES MIXED CONF FILE (OLD) APPROACH: */
			// Strings attributes:
			// HLRLOCATION, MYPROXYSERVER, VIRTUAL ORGANISATION, JOB_PROVENANCE
			setMissing(jdl,JDL::MYPROXY,conf->my_proxy_server());
			setMissing(jdl,JDL::HLR_LOCATION,conf->hlrlocation());
			setMissing(jdl,JDL::VIRTUAL_ORGANISATION,conf->virtual_organisation(),true);
			setMissing(jdl,JDL::JOB_PROVENANCE,conf->job_provenance());
			setMissing(jdl,JDL::LB_ADDRESS,conf->lbaddress());
			// Boolean Attributes:
			// ALLOW_ZIPPED_ISB ,PU_FILE_ENABLE
			setMissing(jdl,JDL::ALLOW_ZIPPED_ISB,conf->allow_zipped_isb());
			setMissing(jdl,JDL::PU_FILE_ENABLE,conf->perusal_file_enable());
			// INT attributes special [SHALLOW]RETRYCOUNT
			if(jdl->hasAttribute(JDL::TYPE   , JDL_TYPE_COLLECTION) ||
			   jdl->hasAttribute(JDL::TYPE   , JDL_TYPE_DAG)){
				// COLLECTIONS AND DAGS case:
				// (append special Resubmission private attributes)
				if((!jdl->hasAttribute(JDLPrivate::DEFAULT_NODE_RETRYCOUNT))){
					jdl->setAttribute(JDLPrivate::DEFAULT_NODE_RETRYCOUNT,conf->retry_count());
				}
				if((!jdl->hasAttribute(JDLPrivate::DEFAULT_NODE_SHALLOWRETRYCOUNT))){
					jdl->setAttribute(JDLPrivate::DEFAULT_NODE_SHALLOWRETRYCOUNT,conf->shallow_retry_count());
				}
			}else{
				// NORMAL JOBS: normal approach
				setMissing(jdl, JDL::RETRYCOUNT, conf->retry_count());
				setMissing(jdl, JDL::SHALLOWRETRYCOUNT, conf->shallow_retry_count());
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
		if (pathOpt){
			// JDL default attributes:
			Ad confPathAd;
			confPathAd.fromFile(*pathOpt);
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
	glite::wms::common::configuration::WMCConfiguration* conf){
	if (!conf){return;}
	if (conf->jdl_default_attributes()){
		/* JDL ATTRIBUTE CUSTOM (NEW) APPROACH: */
		Ad confAd(*(conf->jdl_default_attributes()));
		// RANK
		if(confAd.hasAttribute(JDL::RANK)){
			jdl->setDefaultRank(confAd.lookUp(JDL::RANK));
		}
		// REQUIREMENTS
		if(confAd.hasAttribute(JDL::REQUIREMENTS)){
			jdl->setDefaultReq(confAd.lookUp(JDL::REQUIREMENTS));
		}
	}else{
		/* ALL ATTRIBUTES MIXED CONF FILE (OLD) APPROACH: */
		// RANK
		if(conf->rank()!=NULL){ jdl->setDefaultRank(conf->rank());}
		// REQUIREMENTS
		if(conf->requirements()!=NULL){ jdl->setDefaultReq(conf->requirements());}
	}
}
/******************
* JDL is a Dag
*******************/
void AdUtils::setDefaultValues(glite::jdl::ExpDagAd* jdl,
	glite::wms::common::configuration::WMCConfiguration* conf){
	if (!conf){return;}
	if (conf->jdl_default_attributes()){
		/* JDL ATTRIBUTE CUSTOM (NEW) APPROACH: */
		Ad confAd(*(conf->jdl_default_attributes()));
		// RANK
		if(confAd.hasAttribute(JDL::RANK)){
			jdl->setDefaultRank(confAd.lookUp(JDL::RANK));
		}
		// REQUIREMENTS
		if(confAd.hasAttribute(JDL::REQUIREMENTS)){
			jdl->setDefaultReq(confAd.lookUp(JDL::REQUIREMENTS));
		}
	}else{
		/* ALL ATTRIBUTES MIXED CONF FILE (OLD) APPROACH: */
		// RANK
		if(conf->rank()!=NULL){ jdl->setDefaultRank(conf->rank()); }
		// REQUIREMENTS
		if(conf->requirements()!=NULL){ jdl->setDefaultReq(conf->requirements()); }
	}
}
/******************
* JDL is a CollectionAd
*******************/
void AdUtils::setDefaultValues(glite::jdl::CollectionAd* jdl,
	glite::wms::common::configuration::WMCConfiguration* conf){
	if (!conf){return;}
	if (conf->jdl_default_attributes()){
		/* JDL ATTRIBUTE CUSTOM (NEW) APPROACH: */
		Ad confAd(*(conf->jdl_default_attributes()));
		// RANK
		if(confAd.hasAttribute(JDL::RANK)){
			jdl->setDefaultRank(confAd.lookUp(JDL::RANK));
		}
		// REQUIREMENTS
		if(confAd.hasAttribute(JDL::REQUIREMENTS)){
			jdl->setDefaultReq(confAd.lookUp(JDL::REQUIREMENTS));
		}
	}else{
		/* ALL ATTRIBUTES MIXED CONF FILE (OLD) APPROACH: */
		// RANK
		if(conf->rank()!=NULL){ jdl->setDefaultRank(conf->rank());}
		// REQUIREMENTS
		if(conf->requirements()!=NULL){ jdl->setDefaultReq(conf->requirements());}
	}
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
	glite::wmsutils::jobid::JobId jobid){
	if (map.size()){
		std::map<std::string,std::string >::const_iterator it = map.find(jobid.toString());
		if (it !=map.end()){
			return (*it).second;
		}
	}
	// If this point is reached, no mapping found. Simply return job unique string
	return jobid.getUnique();
}
} // glite
} // wms
} // client
} // utilities
