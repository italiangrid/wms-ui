/*
Copyright (c) Members of the EGEE Collaboration. 2004.
See http://www.eu-egee.org/partners for details on the
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

// File: common.cpp
// Author: Salvatore Monforte <Salvatore.Monforte@ct.infn.it>
// Copyright (c) 2002 EU DataGrid.

// $Id: common.cpp,v 1.8.2.4.6.2.4.3 2012/06/22 11:51:31 mcecchi Exp $

#include <boost/tokenizer.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/trim.hpp>

#include "glite/wms/ism/ism.h"
#include "glite/wms/ism/purchaser/common.h"
#include "glite/wms/common/logger/logger_utils.h"
#include "glite/wmsutils/classads/classad_utils.h"
#include "glite/wms/common/configuration/Configuration.h"
#include "glite/wms/common/configuration/WMConfiguration.h"
#include "glite/wms/common/configuration/NSConfiguration.h"

using namespace std;
namespace utils = glite::wmsutils::classads;

namespace glite {
namespace wms {
namespace ism {
namespace purchaser {

void apply_skip_predicate(
  glue_info_container_type& glue_info_container,
  vector<glue_info_iterator>& glue_info_container_updated_entries,
  skip_predicate_type skip,
  std::string const& purchasedby
)
{
  glue_info_iterator it = glue_info_container.begin();
  glue_info_iterator const glue_info_container_end(
    glue_info_container.end()
  );

  for ( ; it != glue_info_container_end; ++it) {
    if (!skip(it->first)) {
      it->second->InsertAttr("PurchasedBy", purchasedby);
      glue_info_container_updated_entries.push_back(it);
    } else {
      Debug("Skipping " << it->first << " due to skip predicate settings");
    }
  }
}

void tokenize_ldap_dn(std::string const& s, std::vector<std::string> &v)
{
  boost::escaped_list_separator<char> ldap_dn_sep("",",","");
  boost::tokenizer<boost::escaped_list_separator<char> >
    ldap_dn_tok(s,ldap_dn_sep);

  boost::tokenizer< boost::escaped_list_separator<char> >::iterator
    ldap_dn_tok_it(
      ldap_dn_tok.begin()
    );
  boost::tokenizer< boost::escaped_list_separator<char> >::iterator const
    ldap_dn_tok_end(
       ldap_dn_tok.end()
    );

  for( ; ldap_dn_tok_it != ldap_dn_tok_end; ++ldap_dn_tok_it)
    v.push_back(
      boost::algorithm::trim_copy(*ldap_dn_tok_it)
    );

}

namespace {

   std::string const gangmatch_storage_ad_str(
    "["
    "  storage =  ["
    "     VO = parent.other.VirtualOrganisation;"
    "     CloseSEs = retrieveCloseSEsInfo( VO );"
    "  ];"
    "]"
  );

  boost::scoped_ptr<classad::ClassAd> gangmatch_storage_ad;
}

bool expand_information_service_info(glue_info_type& glue_info)
{
  string isURL;
  bool result = false;
  try {

    isURL = static_cast<std::string>(
      utils::evaluate_attribute(*glue_info, "GlueInformationServiceURL")
    );
    static boost::regex  expression_gisu("\\S.*://(.*):([0-9]+)/(.*)");
    boost::smatch        pieces_gisu;

    if (boost::regex_match(isURL, pieces_gisu, expression_gisu)) {

      string ishost(pieces_gisu[1].first, pieces_gisu[1].second);
      string isport(pieces_gisu[2].first, pieces_gisu[2].second);
      string isbasedn(pieces_gisu[3].first, pieces_gisu[3].second);

      glue_info->InsertAttr("InformationServiceDN", isbasedn);
      glue_info->InsertAttr("InformationServiceHost", ishost);
      glue_info->InsertAttr("InformationServicePort", boost::lexical_cast<int>(isport));
      result = true;
    }
  } catch (utils::InvalidValue& e) {
    Error("Cannot evaluate GlueInformationServiceURL...");
    result = false;
  }
  return result;
}

bool insert_gangmatch_storage_ad(glue_info_type& glue_info)
{
 try {
    if(!gangmatch_storage_ad) {
      gangmatch_storage_ad.reset( 
        utils::parse_classad(gangmatch_storage_ad_str)
      ); 
    }
    glue_info->Update(*gangmatch_storage_ad);
  } catch(...) {
    assert(false);
  }
  return true;
}

bool expand_glueid_info(glue_info_type& glue_info)
{
  string ce_str;
  ce_str.assign(utils::evaluate_attribute(*glue_info, "GlueCEUniqueID"));
  static boost::regex  expression_ceid("(.+/[^\\-]+-([^\\-]+))-(.+)");
  boost::smatch  pieces_ceid;
  string gcrs, type, name;
  
  if (boost::regex_match(ce_str, pieces_ceid, expression_ceid)) {
    
    gcrs.assign(pieces_ceid[1].first, pieces_ceid[1].second);
    try {
      type.assign(utils::evaluate_attribute(*glue_info, "GlueCEInfoLRMSType"));
    } 
    catch(utils::InvalidValue& e) {
      // Try to fall softly in case the attribute is missing...
      type.assign(pieces_ceid[2].first, pieces_ceid[2].second);
      Warning("Cannot evaluate GlueCEInfoLRMSType using value from contact string: " << type);
    }
    // ... or in case the attribute is empty.
    if (type.length() == 0) type.assign(pieces_ceid[2].first, pieces_ceid[2].second);
    name.assign(pieces_ceid[3].first, pieces_ceid[3].second);
  } else { 
    Warning("Cannot parse CEid=" << ce_str);
    return false;
  }

  glue_info -> InsertAttr("GlobusResourceContactString", gcrs);
  glue_info -> InsertAttr("LRMSType", type);
  glue_info -> InsertAttr("QueueName", name);
  glue_info -> InsertAttr("CEid", ce_str);

  return true;
}

bool split_information_service_url(classad::ClassAd const& ad, boost::tuple<std::string, int, std::string>& i)
{
 try {
  std::string ldap_dn;
  std::string ldap_host;
  std::string ldap_url;

  ldap_url.assign( utils::evaluate_attribute(ad, "GlueInformationServiceURL") );
  static boost::regex expression_gisu( "\\S.*://(.*):([0-9]+)/(.*)" );
  boost::smatch pieces_gisu;
  std::string port;

  if (boost::regex_match(ldap_url, pieces_gisu, expression_gisu)) {

    ldap_host.assign (pieces_gisu[1].first, pieces_gisu[1].second);
    port.assign      (pieces_gisu[2].first, pieces_gisu[2].second);
    ldap_dn.assign   (pieces_gisu[3].first, pieces_gisu[3].second);

    i = boost::make_tuple(ldap_host, std::atoi(port.c_str()), ldap_dn);
  }
  else {
  return false;
  }
 }
 catch (utils::InvalidValue& e) {
   return false;
 }
 return true;
}

}}}}
