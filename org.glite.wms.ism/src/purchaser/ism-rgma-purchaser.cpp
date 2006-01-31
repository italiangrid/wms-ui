// File: ism-rgma-purchaser.cpp
// Author: Enzo Martelli <enzo.martelli@mi.infn.it>
// Copyright (c) 2002 EU DataGrid.
// For license conditions see http://www.eu-datagrid.org/license.html

#include <boost/mem_fn.hpp>
//#include <time.h>
#include "glite/wms/ism/purchaser/ism-rgma-purchaser.h"

#include "glite/wms/common/ldif2classad/exceptions.h"
#include "glite/wms/common/utilities/ii_attr_utils.h"
#include "glite/wms/common/utilities/classad_utils.h"

#include "glite/wms/common/logger/edglog.h"
#include "glite/wms/common/logger/manipulators.h"

//RGMA headers
#include "rgma/ConsumerFactory.h"
#include "impl/ConsumerFactoryImpl.h"
#include "rgma/Consumer.h"
#include "rgma/QueryProperties.h"
#include "rgma/ResultSet.h"
#include "rgma/TimeInterval.h"
#include "rgma/Units.h"
#include "rgma/RemoteException.h"
#include "rgma/UnknownResourceException.h"
#include "rgma/RGMAException.h"
#include "rgma/ResourceEndpointList.h"
#include "rgma/ResourceEndpoint.h"
#include "rgma/ResultSetMetaData.h"
#include "rgma/Types.h"
#include "rgma/Tuple.h"

#include <string>
#include <vector>

#define edglog(level) logger::threadsafe::edglog << logger::setlevel(logger::level)
#define edglog_fn(name) logger::StatePusher    pusher(logger::threadsafe::edglog, #name);

//#define MAXDELAY 100000000;                                                                                                     
using glite::rgma::ConsumerFactory;
using rgma::impl::ConsumerFactoryImpl;
using glite::rgma::Consumer;
using glite::rgma::QueryProperties;
using glite::rgma::ResultSet;
using glite::rgma::TimeInterval;
using glite::rgma::Units;
using glite::rgma::RemoteException;
using glite::rgma::UnknownResourceException;
using glite::rgma::RGMAException;
using glite::rgma::ResourceEndpointList;
using glite::rgma::ResourceEndpoint;
using glite::rgma::ResultSetMetaData;
using glite::rgma::Types;
using glite::rgma::Tuple;

using namespace classad;
using namespace std;


namespace glite {
namespace wms {

namespace ldif2classad	= common::ldif2classad;
namespace logger        = common::logger;
namespace utilities     = common::utilities;

namespace ism {
namespace purchaser {

namespace {
boost::condition f_rgma_purchasing_cycle_run_condition;
boost::mutex     f_rgma_purchasing_cycle_run_mutex;
}


gluece_query* gluece_query::m_query = NULL;

gluece_query* gluece_query::get_query_instance()
{
   if (m_query == NULL) {
      m_query = new gluece_query();
   }
   return m_query;
}

gluece_query::~gluece_query()
{ 
   if (m_consumer != NULL) {
      try {
         if ( m_query_status ) m_consumer->close();
// workaround that avoid "delete" call when an exception is catched
// it would cause an abort. 
//         delete m_consumer;
      }
      catch  (RemoteException e) {
         Error("Failed to contact Consumer service...");
         Error(e.getMessage());
      }
      catch (UnknownResourceException ure) {
         Error("Failed to contact Consumer service...");
         Error(ure.getMessage());
      }
      catch (RGMAException rgmae) {
         Error("R-GMA application error in Consumer...");
         Error(rgmae.getMessage());
      }
      delete m_consumer;
   }
}


bool gluece_query::refresh_consumer ( int rgma_consumer_ttl )
{
   if ( m_consumer != NULL ) {
      try {
         if ( m_query_status ) m_consumer->close();
      }
      catch  (RemoteException e) {
         Error("Failed to contact Consumer service...");
         Error(e.getMessage());
         m_query_status = false; 
      }
      catch (UnknownResourceException ure) {
         Error("Failed to contact Consumer service...");
         Error(ure.getMessage());
         m_query_status = false;
      }
      catch (RGMAException rgmae) {
         Error("R-GMA application error in Consumer...");
         Error(rgmae.getMessage());
         m_query_status = false;
      }
      delete m_consumer;

      m_consumer = NULL;
      m_query_status = false;
   }
   boost::scoped_ptr<ConsumerFactory> factory(new ConsumerFactoryImpl());
   TimeInterval terminationInterval( rgma_consumer_ttl, Units::SECONDS);
   try {
      m_consumer = factory->createConsumer( terminationInterval,
                                                   "SELECT * FROM GlueCE",
                                                   QueryProperties::LATEST );
      m_query_status = true;
      return true;
   }
   catch (RemoteException e) {
      Error("Failed to contact Consumer service...");
      Error(e.getMessage());
      m_consumer = NULL;
      m_query_status = false;
      return false;
   }
   catch (UnknownResourceException ure) {
      Error("Failed to contact Consumer service...");
      Error(ure.getMessage());
      m_consumer = NULL;
      m_query_status = false;
      return false;
   }
   catch (RGMAException rgmae) {
      Error("R-GMA application error in Consumer...");
      Error(rgmae.getMessage());
      m_consumer = NULL;
      m_query_status = false;
      return false;
   }
}

bool gluece_query::refresh_query ( int rgma_query_timeout )
{
//   struct timespec delay; delay.tv_sec = 0; delay.tv_nsec = MAXDELAY;   
   try {
      if ( (m_consumer == NULL) || (m_query_status == false) ) {
         Warning("Consumer not created or corrupted");
         m_consumer = false;
         return false;
      }
      if ( m_consumer->isExecuting() ) m_consumer->abort();
         
      m_consumer->start( TimeInterval(rgma_query_timeout, Units::SECONDS) );
      //while(m_consumer->isExecuting()){
      //   nanosleep(&delay,NULL);
      //}
      m_query_status = true; 
      return true;
   }
   catch (RemoteException e) {
      Error("Failed to contact Consumer service...");
      Error(e.getMessage());
      m_query_status = false;
      return false;
   }
   catch (UnknownResourceException ure) {
      Error("Failed to contact Consumer service...");
      Error(ure.getMessage());
      m_query_status = false;
      return false;
   }
   catch (RGMAException rgmae) {
      Error("R-GMA application error in Consumer...");
      Error(rgmae.getMessage());
      m_query_status = false;
      return false;
   }
}

bool gluece_query::pop_tuples ( glite::rgma::ResultSet & out, int maxTupleNumber)
{
   try {
      if ( m_query_status ) {
         m_consumer->pop(out, maxTupleNumber);
         return true;
      }
      else {
         Error("Cannot pop tuples: Query failed or not started");
         return false;
      }
   }
   catch (RemoteException e) {
      Error("Failed to contact Consumer service...");
      Error(e.getMessage());
      m_query_status = false;
      return false;
   }
   catch (UnknownResourceException ure) {
      Error("Failed to contact Consumer service...");
      Error(ure.getMessage());
      m_query_status = false;
      return false;
   }
   catch (RGMAException rgmae) {
      Error("R-GMA application error in Consumer...");
      Error(rgmae.getMessage());
      m_query_status = false;
      return false;
   }
}



AccessControlBaseRule_query* AccessControlBaseRule_query::m_query = NULL;

AccessControlBaseRule_query* AccessControlBaseRule_query::get_query_instance()
{
   if (m_query == NULL) {
      m_query = new AccessControlBaseRule_query();
   }
   return m_query;
}

AccessControlBaseRule_query::~AccessControlBaseRule_query()
{
   if (m_consumer != NULL) {
      try {
         if ( m_query_status ) m_consumer->close();
// workaround that avoid "delete" call when an exception is catched
// it would cause an abort. 
//         delete m_consumer;
      }
      catch  (RemoteException e) {
         Error("Failed to contact Consumer service...");
         Error(e.getMessage());
      }
      catch (UnknownResourceException ure) {
         Error("Failed to contact Consumer service...");
         Error(ure.getMessage());
      }
      catch (RGMAException rgmae) {
         Error("R-GMA application error in Consumer...");
         Error(rgmae.getMessage());
      }
      delete m_consumer;
   }
}


bool AccessControlBaseRule_query::refresh_consumer ( int rgma_consumer_ttl )
{
   if ( m_consumer != NULL ) {
      try {
         if ( m_query_status ) m_consumer->close();
      }
      catch  (RemoteException e) {
         Error("Failed to contact Consumer service...");
         Error(e.getMessage());
         m_query_status = false;
      }
      catch (UnknownResourceException ure) {
         Error("Failed to contact Consumer service...");
         Error(ure.getMessage());
         m_query_status = false;
      }
      catch (RGMAException rgmae) {
         Error("R-GMA application error in Consumer...");
         Error(rgmae.getMessage());
         m_query_status = false;
      }
      delete m_consumer;

      m_consumer = NULL;
      m_query_status = false;
   }

   boost::scoped_ptr<ConsumerFactory> factory(new ConsumerFactoryImpl());
   TimeInterval terminationInterval( rgma_consumer_ttl, Units::SECONDS);
   try {
      m_consumer = factory->createConsumer( terminationInterval,
                                                   "SELECT * FROM GlueCEAccessControlBaseRule",
                                                   QueryProperties::LATEST );
      m_query_status = true;
      return true;
   }
   catch (RemoteException e) {
      Error("Failed to contact Consumer service...");
      Error(e.getMessage());
      m_consumer = NULL;
      m_query_status = false;
      return false;
   }
   catch (UnknownResourceException ure) {
      Error("Failed to contact Consumer service...");
      Error(ure.getMessage());
      m_consumer = NULL;
      m_query_status = false;
      return false;
   }
   catch (RGMAException rgmae) {
      Error("R-GMA application error in Consumer...");
      Error(rgmae.getMessage());
      m_consumer = NULL;
      m_query_status = false;
      return false;
   }
}


bool AccessControlBaseRule_query::refresh_query ( int rgma_query_timeout )
{
//   struct timespec delay; delay.tv_sec = 0; delay.tv_nsec = MAXDELAY;   
   try {
      if ( (m_consumer == NULL) || (m_query_status == false) ) {
         Warning("Consumer not created or corrupted");
         m_consumer = false;
         return false;
      }

      if ( m_consumer->isExecuting() )
         m_consumer->abort();

      m_consumer->start( TimeInterval(rgma_query_timeout, Units::SECONDS) );
      //while(m_consumer->isExecuting()){
      //   nanosleep(&delay,NULL);
      //}
      m_query_status = true;
      return true;
   }
   catch (RemoteException e) {
      Error("Failed to contact Consumer service...");
      Error(e.getMessage());
      m_query_status = false;
      return false;
   }
   catch (UnknownResourceException ure) {
      Error("Failed to contact Consumer service...");
      Error(ure.getMessage());
      m_query_status = false;
      return false;
   }
   catch (RGMAException rgmae) {
      Error("R-GMA application error in Consumer...");
      Error(rgmae.getMessage());
      m_query_status = false;
      return false;
   }
}

bool AccessControlBaseRule_query::pop_tuples ( glite::rgma::ResultSet & out, int maxTupleNumber)
{
   try {
      if ( m_query_status ) {
         m_consumer->pop(out, maxTupleNumber);
         return true;
      }
      else {
         Error("Cannot pop tuples: Query failed or not started");
         return false;
      }
   }
   catch (RemoteException e) {
      Error("Failed to contact Consumer service...");
      Error(e.getMessage());
      m_query_status = false;
      return false;
   }
   catch (UnknownResourceException ure) {
      Error("Failed to contact Consumer service...");
      Error(ure.getMessage());
      m_query_status = false;
      return false;
   }
   catch (RGMAException rgmae) {
      Error("R-GMA application error in Consumer...");
      Error(rgmae.getMessage());
      m_query_status = false;
      return false;
   }
}



SubCluster_query* SubCluster_query::m_query = NULL;

SubCluster_query* SubCluster_query::get_query_instance()
{
   if (m_query == NULL) {
      m_query = new SubCluster_query();
   }
   return m_query;
}

SubCluster_query::~SubCluster_query()
{
   if (m_consumer != NULL) {
      try {
         if ( m_query_status ) m_consumer->close();
// workaround that avoid "delete" call when an exception is catched
// it would cause an abort. 
//         delete m_consumer;
      }
      catch  (RemoteException e) {
         Error("Failed to contact Consumer service...");
         Error(e.getMessage());
      }
      catch (UnknownResourceException ure) {
         Error("Failed to contact Consumer service...");
         Error(ure.getMessage());
      }
      catch (RGMAException rgmae) {
         Error("R-GMA application error in Consumer...");
         Error(rgmae.getMessage());
      }
      delete m_consumer;
   }
}


bool SubCluster_query::refresh_consumer ( int rgma_consumer_ttl )
{
   if ( m_consumer != NULL ) {
      try {
         if ( m_query_status ) m_consumer->close();
      }
      catch  (RemoteException e) {
         Error("Failed to contact Consumer service...");
         Error(e.getMessage());
         m_query_status = false;
      }
      catch (UnknownResourceException ure) {
         Error("Failed to contact Consumer service...");
         Error(ure.getMessage());
         m_query_status = false;
      }
      catch (RGMAException rgmae) {
         Error("R-GMA application error in Consumer...");
         Error(rgmae.getMessage());
         m_query_status = false;
      }
      delete m_consumer;

      m_consumer = NULL;
      m_query_status = false;
   }

   boost::scoped_ptr<ConsumerFactory> factory(new ConsumerFactoryImpl());
   TimeInterval terminationInterval( rgma_consumer_ttl, Units::SECONDS);
   try {
      m_consumer = factory->createConsumer( terminationInterval,
                                                   "SELECT * FROM GlueSubCluster",
                                                   QueryProperties::LATEST );
      m_query_status = true;
      return true;
   }
   catch (RemoteException e) {
      Error("Failed to contact Consumer service...");
      Error(e.getMessage());
      m_consumer = NULL;
      m_query_status = false;
      return false;
   }
   catch (UnknownResourceException ure) {
      Error("Failed to contact Consumer service...");
      Error(ure.getMessage());
      m_consumer = NULL;
      m_query_status = false;
      return false;
   }
   catch (RGMAException rgmae) {
      Error("R-GMA application error in Consumer...");
      Error(rgmae.getMessage());
      m_consumer = NULL;
      m_query_status = false;
      return false;
   }
}


bool SubCluster_query::refresh_query ( int rgma_query_timeout )
{
//   struct timespec delay; delay.tv_sec = 0; delay.tv_nsec = MAXDELAY;   
   try {
      if ( (m_consumer == NULL) || (m_query_status == false) ) {
         Warning("Consumer not created or corrupted");
         m_consumer = false;
         return false;
      }
      if ( m_consumer->isExecuting() )
         m_consumer->abort();

      m_consumer->start( TimeInterval(rgma_query_timeout, Units::SECONDS) );
      //while(m_consumer->isExecuting()){
      //   nanosleep(&delay,NULL);
      //}
      m_query_status = true;
      return true;
   }
   catch (RemoteException e) {
      Error("Failed to contact Consumer service...");
      Error(e.getMessage());
      m_query_status = false;
      return false;
   }
   catch (UnknownResourceException ure) {
      Error("Failed to contact Consumer service...");
      Error(ure.getMessage());
      m_query_status = false;
      return false;
   }
   catch (RGMAException rgmae) {
      Error("R-GMA application error in Consumer...");
      Error(rgmae.getMessage());
      m_query_status = false;
      return false;
   }
}

bool SubCluster_query::pop_tuples ( glite::rgma::ResultSet & out, int maxTupleNumber)
{
   try {
      if ( m_query_status ) {
         m_consumer->pop(out, maxTupleNumber);
         return true;
      }
      else {
         Error("Cannot pop tuples: Query failed or not started");
         return false;
      }
   }
   catch (RemoteException e) {
      Error("Failed to contact Consumer service...");
      Error(e.getMessage());
      m_query_status = false;
      return false;
   }
   catch (UnknownResourceException ure) {
      Error("Failed to contact Consumer service...");
      Error(ure.getMessage());
      m_query_status = false;
      return false;
   }
   catch (RGMAException rgmae) {
      Error("R-GMA application error in Consumer...");
      Error(rgmae.getMessage());
      m_query_status = false;
      return false;
   }
}




SoftwareRunTimeEnvironment_query* SoftwareRunTimeEnvironment_query::m_query = NULL;

SoftwareRunTimeEnvironment_query* SoftwareRunTimeEnvironment_query::get_query_instance()
{
   if (m_query == NULL) {
      m_query = new SoftwareRunTimeEnvironment_query();
   }
   return m_query;
}

SoftwareRunTimeEnvironment_query::~SoftwareRunTimeEnvironment_query()
{
   if (m_consumer != NULL) {
      try {
         if ( m_query_status ) m_consumer->close();
// workaround that avoid "delete" call when an exception is catched
// it would cause an abort. 
//         delete m_consumer;
      }
      catch  (RemoteException e) {
         Error("Failed to contact Consumer service...");
         Error(e.getMessage());
      }
      catch (UnknownResourceException ure) {
         Error("Failed to contact Consumer service...");
         Error(ure.getMessage());
      }
      catch (RGMAException rgmae) {
         Error("R-GMA application error in Consumer...");
         Error(rgmae.getMessage());
      }
      delete m_consumer;
   }
}


bool SoftwareRunTimeEnvironment_query::refresh_consumer ( int rgma_consumer_ttl )
{
   if ( m_consumer != NULL ) {
      try {
         if ( m_query_status ) m_consumer->close();
      }
      catch  (RemoteException e) {
         Error("Failed to contact Consumer service...");
         Error(e.getMessage());
         m_query_status = false;
      }
      catch (UnknownResourceException ure) {
         Error("Failed to contact Consumer service...");
         Error(ure.getMessage());
         m_query_status = false;
      }
      catch (RGMAException rgmae) {
         Error("R-GMA application error in Consumer...");
         Error(rgmae.getMessage());
         m_query_status = false;
      }
      delete m_consumer;

      m_consumer = NULL;
      m_query_status = false;
   }

   boost::scoped_ptr<ConsumerFactory> factory(new ConsumerFactoryImpl());
   TimeInterval terminationInterval( rgma_consumer_ttl, Units::SECONDS);
   try {
      m_consumer = factory->createConsumer( terminationInterval,
                                                   "SELECT * FROM GlueSubClusterSoftwareRunTimeEnvironment",
                                                   QueryProperties::LATEST );
      m_query_status = true;
      return true;
   }
   catch (RemoteException e) {
      Error("Failed to contact Consumer service...");
      Error(e.getMessage());
      m_consumer = NULL;
      m_query_status = false;
      return false;
   }
   catch (UnknownResourceException ure) {
      Error("Failed to contact Consumer service...");
      Error(ure.getMessage());
      m_consumer = NULL;
      m_query_status = false;
      return false;
   }
   catch (RGMAException rgmae) {
      Error("R-GMA application error in Consumer...");
      Error(rgmae.getMessage());
      m_consumer = NULL;
      m_query_status = false;
      return false;
   }
}


bool SoftwareRunTimeEnvironment_query::refresh_query ( int rgma_query_timeout )
{
//   struct timespec delay; delay.tv_sec = 0; delay.tv_nsec = MAXDELAY;   
   try {
      if ( (m_consumer == NULL) || (m_query_status == false) ) {
         Warning("Consumer not created or corrupted");
         m_consumer = false;
         return false;
      }

      if ( m_consumer->isExecuting() )
         m_consumer->abort();

      m_consumer->start( TimeInterval(rgma_query_timeout, Units::SECONDS) );
      //while(m_consumer->isExecuting()){
      //   nanosleep(&delay,NULL);
      //}
      m_query_status = true;
      return true;

   }
   catch (RemoteException e) {
      Error("Failed to contact Consumer service...");
      Error(e.getMessage());
      m_query_status = false;
      return false;
   }
   catch (UnknownResourceException ure) {
      Error("Failed to contact Consumer service...");
      Error(ure.getMessage());
      m_query_status = false;
      return false;
   }
   catch (RGMAException rgmae) {
      Error("R-GMA application error in Consumer...");
      Error(rgmae.getMessage());
      m_query_status = false;
      return false;
   }
}

bool SoftwareRunTimeEnvironment_query::pop_tuples ( glite::rgma::ResultSet & out, int maxTupleNumber)
{
   try {
      if ( m_query_status ) {
         m_consumer->pop(out, maxTupleNumber);
         return true;
      }
      else {
         Error("Cannot pop tuples: Query failed or not started");
         return false;
      }
   }
   catch (RemoteException e) {
      Error("Failed to contact Consumer service...");
      Error(e.getMessage());
      m_query_status = false;
      return false;
   }
   catch (UnknownResourceException ure) {
      Error("Failed to contact Consumer service...");
      Error(ure.getMessage());
      m_query_status = false;
      return false;
   }
   catch (RGMAException rgmae) {
      Error("R-GMA application error in Consumer...");
      Error(rgmae.getMessage());
      m_query_status = false;
      return false;
   }
}




CESEBind_query* CESEBind_query::m_query = NULL;

CESEBind_query* CESEBind_query::get_query_instance()
{
   if (m_query == NULL) {
      m_query = new CESEBind_query();
   }
   return m_query;
}

CESEBind_query::~CESEBind_query()
{
   if (m_consumer != NULL) {
      try {
         if ( m_query_status ) m_consumer->close();
// workaround that avoid "delete" call when an exception is catched
// it would cause an abort. 
//         delete m_consumer;
      }
      catch  (RemoteException e) {
         Error("Failed to contact Consumer service...");
         Error(e.getMessage());
      }
      catch (UnknownResourceException ure) {
         Error("Failed to contact Consumer service...");
         Error(ure.getMessage());
      }
      catch (RGMAException rgmae) {
         Error("R-GMA application error in Consumer...");
         Error(rgmae.getMessage());
      }
      delete m_consumer;
   }
}


bool CESEBind_query::refresh_consumer ( int rgma_consumer_ttl )
{
   if ( m_consumer != NULL ) {
      try {
         if ( m_query_status ) m_consumer->close();
      }
      catch  (RemoteException e) {
         Error("Failed to contact Consumer service...");
         Error(e.getMessage());
         m_query_status = false;
      }
      catch (UnknownResourceException ure) {
         Error("Failed to contact Consumer service...");
         Error(ure.getMessage());
         m_query_status = false;
      }
      catch (RGMAException rgmae) {
         Error("R-GMA application error in Consumer...");
         Error(rgmae.getMessage());
         m_query_status = false;
      }
      delete m_consumer;

      m_consumer = NULL;
      m_query_status = false;
   }

   boost::scoped_ptr<ConsumerFactory> factory(new ConsumerFactoryImpl());
   TimeInterval terminationInterval( rgma_consumer_ttl, Units::SECONDS);
   try {
      m_consumer = factory->createConsumer( terminationInterval,
                                                   "SELECT * FROM GlueCESEBind",
                                                   QueryProperties::LATEST );
      m_query_status = true;
      return true;
   }
   catch (RemoteException e) {
      Error("Failed to contact Consumer service...");
      Error(e.getMessage());
      m_consumer = NULL;
      m_query_status = false;
      return false;
   }
   catch (UnknownResourceException ure) {
      Error("Failed to contact Consumer service...");
      Error(ure.getMessage());
      m_consumer = NULL;
      m_query_status = false;
      return false;
   }
   catch (RGMAException rgmae) {
      Error("R-GMA application error in Consumer...");
      Error(rgmae.getMessage());
      m_consumer = NULL;
      m_query_status = false;
      return false;
   }
}


bool CESEBind_query::refresh_query ( int rgma_query_timeout )
{
//   struct timespec delay; delay.tv_sec = 0; delay.tv_nsec = MAXDELAY;   
   try {
      if ( (m_consumer == NULL) || (m_query_status == false) ) {
         Warning("Consumer not created or corrupted");
         m_consumer = false;
         return false;
      }

      if ( m_consumer->isExecuting() )
         m_consumer->abort();

      m_consumer->start( TimeInterval(rgma_query_timeout, Units::SECONDS) );
      //while(m_consumer->isExecuting()){
      //   nanosleep(&delay,NULL);
      //}
      m_query_status = true;
      return true;

   }
   catch (RemoteException e) {
      Error("Failed to contact Consumer service...");
      Error(e.getMessage());
      m_query_status = false;
      return false;
   }
   catch (UnknownResourceException ure) {
      Error("Failed to contact Consumer service...");
      Error(ure.getMessage());
      m_query_status = false;
      return false;
   }
   catch (RGMAException rgmae) {
      Error("R-GMA application error in Consumer...");
      Error(rgmae.getMessage());
      m_query_status = false;
      return false;
   }
}

bool CESEBind_query::pop_tuples ( glite::rgma::ResultSet & out, int maxTupleNumber)
{
   try {
      if ( m_query_status ) {
         m_consumer->pop(out, maxTupleNumber);
         return true;
      }
      else {
         Error("Cannot pop tuples: Query failed or not started");
         return false;
      }
   }
   catch (RemoteException e) {
      Error("Failed to contact Consumer service...");
      Error(e.getMessage());
      m_query_status = false;
      return false;
   }
   catch (UnknownResourceException ure) {
      Error("Failed to contact Consumer service...");
      Error(ure.getMessage());
      m_query_status = false;
      return false;
   }
   catch (RGMAException rgmae) {
      Error("R-GMA application error in Consumer...");
      Error(rgmae.getMessage());
      m_query_status = false;
      return false;
   }
}

namespace{

bool ParseValue(const string& v, utilities::edgstrstream& s)
{
   if ( v.size() ) {

      bool is_digit = true;
      utilities::edgstrstream vstream;
      string value;

      if ( v.size() == 1 ) {
         if ( !isdigit(v[0]) ) is_digit = false;
      }
      else {

         for (int i =0; i< v.size(); i++){
            if ( !isdigit(v[i]) && 
                  (v[i] != '.') &&
                  (v[i] != 'B') &&
                  (v[i] != 'K') &&
                  (v[i] != 'M') &&
                  (v[i] != 'G') &&                               
                  (v[i] != 'T')  ) { is_digit = false; break;}
         }
      }
         
      vstream << v;
      vstream >> value;

      if(is_digit) { //...if numeric value...
         s << value;
      }
      else {
         // Change everything back into upper case, but store the
         // result in a different string
         string  lower_v(v);
         transform (lower_v.begin(), lower_v.end(), lower_v.begin(), ::tolower);
                                                                                                                             
         if( lower_v == "true" || lower_v == "false" || lower_v == "undefined" ) {
            s << lower_v;
         }
         // RGMA pubblish boolean values as "T" and "F" but ClassAD wouldn't recognize them.
         else if ( lower_v == "t")  s << "true";
         else if ( lower_v == "f")  s << "false";
         else {
            // Quotes the value for the attribute if alphanumeric...
            s << "\"" << v << "\"";
         }
      }
   }
   else{
      Warning("trying to parse an empty string to be put in the classAd");
      return false;
   }

   return true;

}

bool ParseMultiValue(const RGMAMultiValue& v, utilities::edgstrstream& s)
{
   if ( !v.size() ) {
      Warning("trying to parse an empty multi-value field to be put in the classAd");
      return false;
   }
   s << "{";
   for(RGMAMultiValue::const_iterator it = v.begin(); it != v.end(); ) {
      if ( ParseValue(*it, s) )
         s << ((++it != v.end()) ? "," : "}" );
   }
   return true;
}
                                                                                                                             

bool ExportClassAd( ClassAd* ad, Tuple& tuple )
{
   ClassAdParser parser;
   if ( ! ad ) { Error("Empty ClassAd pointer passed"); return false; }

   ResultSetMetaData* row = NULL;
   if ( !( row = tuple.getMetaData() ) ) { Error("Wrong tuple passed"); return false; }

   if ( row->begin() == row->end() ) {
      Warning("trying to create a classAd from an empty tuple");
      return false;
   }

   for (ResultSetMetaData::iterator rowIt = row->begin(); rowIt < row->end(); rowIt++ )
   {
      utilities::edgstrstream exprstream;
      string name = rowIt->getColumnName();
                                                                                                              
      if ( rowIt->getColumnType()  ==  Types::VARCHAR ){
         try{
            if ( tuple.isNull(name) ) { 
               classad::Value undefined_value;
               undefined_value.SetUndefinedValue();
               ad->Insert(name, dynamic_cast<ExprTree*>(classad::Literal::MakeLiteral(undefined_value)));
               continue;
            }
            else if ( ! ParseValue( tuple.getString(name),  exprstream) ){
               Warning("Failure in parsing "<<name
                       <<" value while trying to convert a tuple in a ClassAd");
               //return false;
               continue;
            }
         }
         catch (RGMAException rgmae) {
            //      edglog(error) <<"Evaluating "<<name<<" attribute...FAILED"<< endl;
            //      edglog(error) <<rgmae.getMessage() << endl;
            Error("Evaluating "<<name<<" attribute...FAILED");
            Error(rgmae.getMessage());
            //return false;
            continue;
         }
      }                                                        
      else {
         //   RGMA fills tables only with string. Should it forsee multi-value fields or
         //   different types ones, we should provide some other function like this
                                                                                                           
         //edglog(error) << "Unknown type found in RGMA table: " << name << endl;
         Error("Unknown type found in RGMA table: " << name);
         continue;
      }
                                                                                                              
      exprstream << ends;
                                                                                                              
      ExprTree* exptree = 0;

      parser.ParseExpression(exprstream.str(), exptree);
      if(!exptree) {
         Error("trying to processing "<< name
               <<" attribute while converting a tuple in a classAd...FAILED");
         return false;
      }
      if ( !ad->Insert(name, exptree) ) {
         Error("trying to insert "<<name
               <<" attribute in a classAd...FAILED");
         return false;
     }
                                                                                                              
   } //for

   return true;
}

//unused
bool extractMultiValueFromResultSet( ResultSet & set,
                                     RGMAMultiValue& v,
                                     const string & nameID,
                                     const string & id,
                                     const string & name)
{
   bool match = false;
   if ( set.begin() == set.end() ){
      Error("parsing a ResultSet for finding every \""<<name
            <<"\" that correspond to \""<<nameID<<":  "<<id<<"\"... FAILED"<<endl);
      Error("EMPTY RESULTSET");
      return false;
   }

   for (ResultSet::iterator tableIt = set.begin() ; tableIt <  set.end() ; tableIt ++ ) {
      try {
         if ( tableIt->getString(nameID) == id ) {
            v.push_back( tableIt->getString(name) );
            match = true;
         }
      }
      catch (RGMAException rgmae) {
//      edglog(error) <<"parsing a ResultSet for finding every "<<name
//                    <<" that correspond to "<<id<<": FAILED"<<endl;
//      edglog(error) <<rgmae.getMessage() << endl;
         Error("parsing a ResultSet for finding every "<<name
                  <<" that correspond to "<<nameID<<":  "<<id<<"... FAILED"<<endl);
         Error(rgmae.getMessage());
         return false;
      }

   }
   return match;

}

bool addMultivalueAttributeToClassAd( ClassAd* ad, const RGMAMultiValue & v, const string & valueName )
{
   ClassAdParser parser;
   utilities::edgstrstream exprstream;

   if ( ! ParseMultiValue( v, exprstream ) ) return false;

   exprstream << ends;

   ExprTree* exptree = 0;

   parser.ParseExpression(exprstream.str(), exptree);
   if(!exptree) {
      Error("trying to processing "<<valueName
            <<" multi-value attribute to be added to a classAd...FAILED");
      return false;
   }
   if ( !ad->Insert(valueName, exptree) ) {
      Error("trying to insert "<<valueName
            <<" multi-value attribute in a classAd...FAILED");
      return false;
   }

   return true;
}

bool changeAttributeName( ClassAd* ad, const string & oldName, const string & newName)
{
   ExprTree* exptree = ad->Remove(oldName.c_str());

   if ( exptree ) {
      if ( ad->Insert(newName, exptree) ) return true;
      else  return false;
   }
   else return false;
   
   //never reached
   return false;
}   


void checkSubCluster( ClassAd* subClusterAd ){

//   Debug("mapping RGMA attribute names of GlueSubCluster table to BDII names");

   //change UniqueID with GlueSubClusterUniqueID
   if ( !changeAttributeName( subClusterAd, "UniqueID", "GlueSubClusterUniqueID") )
      Warning("changing ClassAd's attributeName: UniqueID -> GlueSubClusterUniqueID... FAILED");
   //change Name with GlueSubClusterName
   if ( !changeAttributeName( subClusterAd, "Name", "GlueSubClusterName") )
      Warning("changing ClassAd's attributeName: Name -> GlueSubClusterName... FAILED");
   //change SMPSize with GlueHostArchitectureSMPSize
   if ( !changeAttributeName( subClusterAd, "SMPSize", "GlueHostArchitectureSMPSize") )
      Warning("changing ClassAd's attributeName: SMPSize -> GlueHostArchitectureSMPSize... FAILED");
   //change BenchmarkSF00 with GlueHostBenchmarkSF00
   if ( !changeAttributeName( subClusterAd, "BenchmarkSF00", "GlueHostBenchmarkSF00") )
      Warning("changing ClassAd's attributeName: BenchmarkSF00 -> GlueHostBenchmarkSF00... FAILED");
   //change BenchmarkSI00 with GlueHostBenchmarkSI00
   if ( !changeAttributeName( subClusterAd, "BenchmarkSI00", "GlueHostBenchmarkSI00") )
      Warning("changing ClassAd's attributeName: BenchmarkSI00 -> GlueHostBenchmarkSI00... FAILED");
   //change RAMSize with GlueHostMainMemoryRAMSize
   if ( !changeAttributeName( subClusterAd, "RAMSize", "GlueHostMainMemoryRAMSize") )
      Warning("changing ClassAd's attributeName: RAMSize -> GlueHostMainMemoryRAMSize... FAILED");
   //change VirtualSize with GlueHostMainMemoryVirtualSize
   if ( !changeAttributeName( subClusterAd, "VirtualSize", "GlueHostMainMemoryVirtualSize") )
      Warning("changing ClassAd's attributeName: VirtualSize -> GlueHostMainMemoryVirtualSize... FAILED");
   //change InboundIP with GlueHostNetworkAdapterInboundIP
   if ( !changeAttributeName( subClusterAd, "InboundIP", "GlueHostNetworkAdapterInboundIP") )
      Warning("changing ClassAd's attributeName: InboundIP -> GlueHostNetworkAdapterInboundIP... FAILED");
   //change OutboundIP with GlueHostNetworkAdapterOutboundIP
   if ( !changeAttributeName( subClusterAd, "OutboundIP", "GlueHostNetworkAdapterOutboundIP") )
      Warning("changing ClassAd's attributeName: OutboundIP -> GlueHostNetworkAdapterOutboundIP... FAILED");
   //change OSName with GlueHostOperatingSystemName
   if ( !changeAttributeName( subClusterAd, "OSName", "GlueHostOperatingSystemName") )
      Warning("changing ClassAd's attributeName: OSName -> GlueHostOperatingSystemName... FAILED");
   //change OSRelease with GlueHostOperatingSystemRelease
   if ( !changeAttributeName( subClusterAd, "OSRelease", "GlueHostOperatingSystemRelease") )
      Warning("changing ClassAd's attributeName: OSRelease -> GlueHostOperatingSystemRelease... FAILED");
   //change OSVersion with GlueHostOperatingSystemVersion
   if ( !changeAttributeName( subClusterAd, "OSVersion", "GlueHostOperatingSystemVersion") )
      Warning("changing ClassAd's attributeName: OSVersion -> GlueHostOperatingSystemVersion... FAILED");
   //change ClockSpeed with GlueHostProcessorClockSpeed
   if ( !changeAttributeName( subClusterAd, "ClockSpeed", "GlueHostProcessorClockSpeed") )
      Warning("changing ClassAd's attributeName: ClockSpeed -> GlueHostProcessorClockSpeed... FAILED");
   //change Model with GlueHostProcessorModel
   if ( !changeAttributeName( subClusterAd, "Model", "GlueHostProcessorModel") )
      Warning("changing ClassAd's attributeName: Model -> GlueHostProcessorModel... FAILED");
   //change Vendor with GlueHostProcessorVendor
   if ( !changeAttributeName( subClusterAd, "Vendor", "GlueHostProcessorVendor") )
      Warning("changing ClassAd's attributeName: Vendor -> GlueHostProcessorVendor... FAILED");
   //change InformationServiceURL with GlueInformationServiceURL
   if ( !changeAttributeName( subClusterAd, "InformationServiceURL", "GlueInformationServiceURL") )      
      Warning("changing ClassAd's attributeName: InformationServiceURL -> GlueInformationServiceURL... FAILED");
   //change RAMAvailable with GlueHostMainMemoryRAMAvailable
   if ( !changeAttributeName( subClusterAd, "RAMAvailable", "GlueHostMainMemoryRAMAvailable") )
      Warning("changing ClassAd's attributeName: RAMAvailable -> GlueHostMainMemoryRAMAvailable... FAILED");
   //change VirtualAvailable with GlueHostMainMemoryVirtualAvailable
   if ( !changeAttributeName( subClusterAd, "VirtualAvailable", "GlueHostMainMemoryVirtualAvailable") )
      Warning("changing ClassAd's attributeName: VirtualAvailable -> GlueHostMainMemoryVirtualAvailable... FAILED");
   //change PlatformType with GlueHostArchitecturePlatformType
   if ( !changeAttributeName( subClusterAd, "PlatformType", "GlueHostArchitecturePlatformType") )
      Warning("changing ClassAd's attributeName: PlatformType -> GlueHostArchitecturePlatformType... FAILED");
   //change Version with GlueHostProcessorVersion
   if ( !changeAttributeName( subClusterAd, "Version", "GlueHostProcessorVersion") )
      Warning("changing ClassAd's attributeName: Version -> GlueHostProcessorVersion... FAILED");
   //change InstructionSet with GlueHostProcessorInstructionSet
   if ( !changeAttributeName( subClusterAd, "InstructionSet", "GlueHostProcessorInstructionSet") )
      Warning("changing ClassAd's attributeName: InstructionSet -> GlueHostProcessorInstructionSet... FAILED");
   //change OtherProcessorDescription with GlueHostProcessorOtherProcessorDescription
   if ( !changeAttributeName( subClusterAd, "OtherProcessorDescription", "GlueHostProcessorOtherProcessorDescription") )
      Warning("changing ClassAd's attributeName: OtherProcessorDescription -> GlueHostProcessorOtherProcessorDescription... FAILED");
   //change CacheL1 with GlueHostProcessorCacheL1
   if ( !changeAttributeName( subClusterAd, "CacheL1", "GlueHostProcessorCacheL1") )
      Warning("changing ClassAd's attributeName: CacheL1 -> GlueHostProcessorCacheL1... FAILED");
   //change CacheL1D with GlueHostProcessorCacheL1D
   if ( !changeAttributeName( subClusterAd, "CacheL1D", "GlueHostProcessorCacheL1D") )
      Warning("changing ClassAd's attributeName: CacheL1D -> GlueHostProcessorCacheL1D... FAILED");
   //change CacheL1I with GlueHostProcessorCacheL1I
   if ( !changeAttributeName( subClusterAd, "CacheL1I", "GlueHostProcessorCacheL1I") )
      Warning("changing ClassAd's attributeName: CacheL1I -> GlueHostProcessorCacheL1I... FAILED");
   //change CacheL2 with GlueHostProcessorCacheL2
   if ( !changeAttributeName( subClusterAd, "CacheL2", "GlueHostProcessorCacheL2") )
      Warning("changing ClassAd's attributeName: CacheL2 -> GlueHostProcessorCacheL2... FAILED");
   //change PhysicalCPUs with GlueSubClusterPhysicalCPUs
   if ( !changeAttributeName( subClusterAd, "PhysicalCPUs", "GlueSubClusterPhysicalCPUs") )
      Warning("changing ClassAd's attributeName: PhysicalCPUs -> GlueSubClusterPhysicalCPUs... FAILED");
   //change LogicalCPUs with GlueSubClusterLogicalCPUs
   if ( !changeAttributeName( subClusterAd, "LogicalCPUs", "GlueSubClusterLogicalCPUs") )
      Warning("changing ClassAd's attributeName: LogicalCPUs -> GlueSubClusterLogicalCPUs... FAILED");
   //change TmpDir with GlueSubClusterTmpDir
   if ( !changeAttributeName( subClusterAd, "TmpDir", "GlueSubClusterTmpDir") )
      Warning("changing ClassAd's attributeName: TmpDir -> GlueSubClusterTmpDir... FAILED");
   //change WNTmpDir with GlueSubClusterWNTmpDir
   if ( !changeAttributeName( subClusterAd, "WNTmpDir", "GlueSubClusterWNTmpDir") )
      Warning("changing ClassAd's attributeName: WNTmpDir -> GlueSubClusterWNTmpDir... FAILED");



}

void checkGlueCE( ClassAd* gluece_info ) {

//   Debug("mapping RGMA attribute names of GlueCE table to BDII names");

   //changing UniqueID with GlueCEUniqueID
   if ( !changeAttributeName( gluece_info, "UniqueID", "GlueCEUniqueID") )
      Warning("changing ClassAd's attributeName: UniqueID -> GlueCEUniqueID... FAILED");
   //changing Name with GlueCEName
   if ( !changeAttributeName( gluece_info, "Name", "GlueCEName") )
      Warning("changing ClassAd's attributeName: Name -> GlueCEName... FAILED");
   //changing  GatekeeperPort with GlueCEInfoGatekeeperPort
   if ( !changeAttributeName( gluece_info, "GatekeeperPort", "GlueCEInfoGatekeeperPort") )
      Warning("changing ClassAd's attributeName: GatekeeperPort -> GlueCEInfoGatekeeperPort... FAILED");
   //changing HostName with GlueCEInfoHostName
   if ( !changeAttributeName( gluece_info, "HostName", "GlueCEInfoHostName") )
      Warning("changing ClassAd's attributeName: HostName -> GlueCEInfoHostName:... FAILED");
   //changing LRMSType with GlueCEInfoLRMSType
   if ( !changeAttributeName( gluece_info, "LRMSType", "GlueCEInfoLRMSType") )
      Warning("changing ClassAd's attributeName: LRMSType -> GlueCEInfoLRMSType... FAILED");
   //changing LRMSVersion with GlueCEInfoLRMSVersion
   if ( !changeAttributeName( gluece_info, "LRMSVersion", "GlueCEInfoLRMSVersion") )
      Warning("changing ClassAd's attributeName: LRMSVersion -> GlueCEInfoLRMSVersion... FAILED");
   //changing GRAMVersion with GlueCEInfoGRAMVersion
   if ( !changeAttributeName( gluece_info, "GRAMVersion", "GlueCEInfoGRAMVersion") )
      Warning("changing ClassAd's attributeName: GRAMVersion -> GlueCEInfoGRAMVersion... FAILED");
   //changing TotalCPUs with GlueCEInfoTotalCPUs
   if ( !changeAttributeName( gluece_info, "TotalCPUs", "GlueCEInfoTotalCPUs") )
      Warning("changing ClassAd's attributeName: TotalCPUs -> GlueCEInfoTotalCPUs... FAILED");
   //changing EstimatedResponseTime with GlueCEStateEstimatedResponseTime
   if ( !changeAttributeName( gluece_info, "EstimatedResponseTime", "GlueCEStateEstimatedResponseTime") )
      Warning("changing ClassAd's attributeName: EstimatedResponseTime -> GlueCEStateEstimatedResponseTime... FAILED");
   //changing FreeCpus with GlueCEStateFreeCPUs
   if ( !changeAttributeName( gluece_info, "FreeCpus", "GlueCEStateFreeCPUs") )
      Warning("changing ClassAd's attributeName: FreeCpus -> GlueCEStateFreeCPUs... FAILED");
   //changing RunningJobs with GlueCEStateRunningJobs
   if ( !changeAttributeName( gluece_info, "RunningJobs", "GlueCEStateRunningJobs") )
      Warning("changing ClassAd's attributeName: RunningJobs -> GlueCEStateRunningJobs... FAILED");
   //changing Status with GlueCEStateStatus
   if ( !changeAttributeName( gluece_info, "Status", "GlueCEStateStatus") )
      Warning("changing ClassAd's attributeName: Status -> GlueCEStateStatus... FAILED");
   //changing TotalJobs with GlueCEStateTotalJobs
   if ( !changeAttributeName( gluece_info, "TotalJobs", "GlueCEStateTotalJobs") )
       Warning("changing ClassAd's attributeName: TotalJobs -> GlueCEStateTotalJobs... FAILED");
   //changing WaitingJobs with GlueCEStateWaitingJobs
   if ( !changeAttributeName( gluece_info, "WaitingJobs", "GlueCEStateWaitingJobs") )
      Warning("changing ClassAd's attributeName: WaitingJobs -> GlueCEStateWaitingJobs... FAILED");
   //changing WorstResponseTime with GlueCEStateWorstResponseTime
   if ( !changeAttributeName( gluece_info, "WorstResponseTime", "GlueCEStateWorstResponseTime") )
      Warning("changing ClassAd's attributeName: WorstResponseTime -> GlueCEStateWorstResponseTime... FAILED");
   //changing MaxCPUTime with GlueCEPolicyMaxCPUTime
   if ( !changeAttributeName( gluece_info, "MaxCPUTime", "GlueCEPolicyMaxCPUTime") )
      Warning("changing ClassAd's attributeName: MaxCPUTime -> GlueCEPolicyMaxCPUTime... FAILED");
   //changing MaxRunningJobs with GlueCEPolicyMaxRunningJobs
   if ( !changeAttributeName( gluece_info, "MaxRunningJobs", "GlueCEPolicyMaxRunningJobs") )
      Warning("changing ClassAd's attributeName: MaxRunningJobs -> GlueCEPolicyMaxRunningJobs... FAILED");
   //changing MaxTotalJobs with GlueCEPolicyMaxTotalJobs
   if ( !changeAttributeName( gluece_info, "MaxTotalJobs", "GlueCEPolicyMaxTotalJobs") )
      Warning("changing ClassAd's attributeName: MaxTotalJobs -> GlueCEPolicyMaxTotalJobs... FAILED");
   //changing MaxWallClockTime with GlueCEPolicyMaxWallClockTime
   if ( !changeAttributeName( gluece_info, "MaxWallClockTime", "GlueCEPolicyMaxWallClockTime") )
      Warning("changing ClassAd's attributeName: MaxWallClockTime -> GlueCEPolicyMaxWallClockTime... FAILED");
   //changing Priority with GlueCEPolicyPriority
   if ( !changeAttributeName( gluece_info, "Priority", "GlueCEPolicyPriority") )
      Warning("changing ClassAd's attributeName: Priority -> GlueCEPolicyPriority... FAILED");
   //changing InformationServiceURL with GlueInformationServiceURL
   if ( !changeAttributeName( gluece_info, "InformationServiceURL", "GlueInformationServiceURL") )
      Warning("changing ClassAd's attributeName: InformationServiceURL -> GlueInformationServiceURL... FAILED");
   //changing JobManager with GlueCEInfoJobManager
   if ( !changeAttributeName( gluece_info, "JobManager", "GlueCEInfoJobManager") )
      Warning("changing ClassAd's attributeName: JobManager -> GlueCEInfoJobManager... FAILED");
   //changing ApplicationDir with GlueCEInfoApplicationDir
   if ( !changeAttributeName( gluece_info, "ApplicationDir", "GlueCEInfoApplicationDir") )
      Warning("changing ClassAd's attributeName: ApplicationDir -> GlueCEInfoApplicationDir... FAILED");
   //changing DataDir with GlueCEInfoDataDir
   if ( !changeAttributeName( gluece_info, "DataDir", "GlueCEInfoDataDir") )
      Warning("changing ClassAd's attributeName: DataDir -> GlueCEInfoDataDir... FAILED");
   //changing DefaultSE with GlueCEInfoDefaultSE
   if ( !changeAttributeName( gluece_info, "DefaultSE", "GlueCEInfoDefaultSE") )
      Warning("changing ClassAd's attributeName: DefaultSE -> GlueCEInfoDefaultSE... FAILED");
   //changing FreeJobSlots with GlueCEStateFreeJobSlots
   if ( !changeAttributeName( gluece_info, "FreeJobSlots", "GlueCEStateFreeJobSlots") )
      Warning("changing ClassAd's attributeName: FreeJobSlots -> GlueCEStateFreeJobSlots... FAILED");
   //changing AssignedJobSlots with GlueCEPolicyAssignedJobSlots
   if ( !changeAttributeName( gluece_info, "AssignedJobSlots", "GlueCEPolicyAssignedJobSlots") )
      Warning("changing ClassAd's attributeName: AssignedJobSlots -> GlueCEPolicyAssignedJobSlots... FAILED");


}

bool checkMainValue( ClassAd* ad ) {
   if ( //!ad->Lookup("GlueCEAccessControlBaseRule")               ||
        !ad->Lookup("GlueSubClusterUniqueID") 
        //!ad->Lookup("GlueSubClusterSoftwareRunTimeEnvironment")  ||
        //!ad->Lookup("CloseStorageElements") 
      )
      return false;
   else return true;
}


bool checkListAttr( ClassAd* ad, const string& valueName, const string& listElem ) {
//   Debug("adding element to \""<<valueName<<"\" list-value");
                                                                                               
   vector<string> val;
   if ( ad->Lookup(valueName) ) {
      if ( ! utilities::EvaluateAttrList( *ad, valueName, val) ) {
         Debug("Cannot evaluate "<<valueName<<" list while trying to add "<<listElem<<".");
         return false;
      }
   }
   val.push_back( listElem );
   if ( ! addMultivalueAttributeToClassAd( ad, val, valueName) ) {
      Warning("Failure in inserting \""<<valueName<<"\" list-value after having added "
              <<listElem);
      return false;
   }
   return true;
}


bool checkClassAdListAttr( ClassAd* ad, const string& valueName, ClassAd* listElem )
{
   //a deep copy of listElem has to be passed  

//   Debug("adding element to \""<<valueName<<"\" list-value");

   vector<ExprTree*>               val;
   classad::ExprList*        expr_list;
    
   if ( ad->EvaluateAttrList( valueName, expr_list) ) {
      //expr_list->GetComponents(val);
      for ( ExprList::iterator it = expr_list->begin(); it < expr_list->end(); it++ )
         val.push_back((*it)->Copy());
   }
   val.push_back(listElem);
   if ( !ad->Insert(valueName, classad::ExprList::MakeExprList(val)) ){
      for ( vector<ExprTree*>::iterator it = val.begin(); it < val.end(); it++ )
         delete *it;      
      Warning("Failure in inserting classad::ExprList after having added another value to "
              <<valueName<<" list-value.");
      return false;
   }
   return true;
   
}


} //  anonymous namespace

void ism_rgma_purchaser::prefetchGlueCEinfo( gluece_info_container_type& gluece_info_container)
{
//   edglog_fn("ism_rgma_purchaser::prefetchGlueCEinfo");
   Debug("starting queries...");

   if ( ! gluece_query::get_query_instance()->refresh_query( m_rgma_query_timeout) ||
        ! AccessControlBaseRule_query::get_query_instance()->refresh_query( m_rgma_query_timeout) ||
        ! SubCluster_query::get_query_instance()->refresh_query( m_rgma_query_timeout) ||
        ! SoftwareRunTimeEnvironment_query::get_query_instance()->refresh_query( m_rgma_query_timeout) ||
        ! CESEBind_query::get_query_instance()->refresh_query( m_rgma_query_timeout)
      ) {
      Warning("RGMA queries FAILED.");
      return;
   }
                                                                                                                             
   Debug("Creating a ClassAd for each entry in GlueCE table");
   ResultSet resultSet;
   do {
      if ( !gluece_query::get_query_instance()->pop_tuples( resultSet, 1000) ) {
         Warning("failed popping tuples from GlueCe");
         return;
      }
      if ( resultSet.begin() != resultSet.end() )  {

         for ( ResultSet::iterator it=resultSet.begin(); it < resultSet.end(); it++ ) {

            boost::shared_ptr<classad::ClassAd> ceAd(new ClassAd());
            string unique_id;

            try {
               if ( ExportClassAd( ceAd.get(), *it ) ) {
                  checkGlueCE( ceAd.get() );
                  unique_id = it->getString("UniqueID");
                  gluece_info_container[unique_id] = ceAd;
               }
               else Warning("Failure in adding an entry for "<< unique_id
                         <<"to the ClassAd list to be put in the ISM");
            }
            catch(RGMAException rgmae) {
                  Error("Evaluating a GlueCE-UniqueID value...FAILED");
                  Error("Cannot add the related entry to the ClassAd list to be put in the ISM");
                  Error(rgmae.getMessage());
            }

         } // for

      }
   }  while( ! resultSet.endOfResults() );
}


bool ism_rgma_purchaser_entry_update::operator()(int a,boost::shared_ptr<classad::ClassAd>& ad)
{
   boost::mutex::scoped_lock l(f_rgma_purchasing_cycle_run_mutex);
   f_rgma_purchasing_cycle_run_condition.notify_one();
   return true;
}

  
ism_rgma_purchaser::ism_rgma_purchaser(
   int rgma_query_timeout,
   exec_mode_t mode,
   int rgma_consumer_ttl,
   int rgma_cons_life_cycles,
   size_t interval,
   exit_predicate_type exit_predicate,
   skip_predicate_type skip_predicate
) : ism_purchaser(mode, interval, exit_predicate, skip_predicate),
    m_rgma_query_timeout(rgma_query_timeout),
    m_rgma_consumer_ttl(rgma_consumer_ttl),
    m_rgma_cons_life_cycles(rgma_cons_life_cycles)
{
}

void ism_rgma_purchaser::operator()()
{
  ism_rgma_purchaser::do_purchase();
}

void ism_rgma_purchaser::do_purchase()
{
//  edglog_fn("ism_rgma_purchaser::do_purchase");
   unsigned int consLifeCycles = 0;
   do {
      bool consumers_are_alive = 
         gluece_query::get_query_instance()->get_query_status() && 
         AccessControlBaseRule_query::get_query_instance()->get_query_status() &&
         SubCluster_query::get_query_instance()->get_query_status() &&
         SoftwareRunTimeEnvironment_query::get_query_instance()->get_query_status() &&
         CESEBind_query::get_query_instance()->get_query_status();

                               
      if ( (consLifeCycles == 0) || (consLifeCycles == m_rgma_cons_life_cycles) || (! consumers_are_alive) ) {
         if ( ! gluece_query::get_query_instance()->refresh_consumer( m_rgma_consumer_ttl ) ||
              ! AccessControlBaseRule_query::get_query_instance()->refresh_consumer( m_rgma_consumer_ttl )  ||
              ! SubCluster_query::get_query_instance()->refresh_consumer( m_rgma_consumer_ttl) ||
              ! SoftwareRunTimeEnvironment_query::get_query_instance()->refresh_consumer( m_rgma_consumer_ttl ) ||
              ! CESEBind_query::get_query_instance()->refresh_consumer( m_rgma_consumer_ttl) ) {
            Warning("RGMA consumer creation failed");
            continue;
         }
         consLifeCycles = 0;
         Debug("CONSUMER REFRESHED");
      }
      consLifeCycles++;  
                                                                                   
      gluece_info_container_type gluece_info_container;
      vector<gluece_info_iterator> gluece_info_container_updated_entries;

      ism_rgma_purchaser::prefetchGlueCEinfo(gluece_info_container);

      bool AccContrBaseRuleIsEmpty = true;
      bool SubClusterIsEmpty = true;
      bool SoftwareRunTimeEnvironmentIsEmpty = true;
      bool CESEBindIsEmpty = true;
      if (AccessControlBaseRule_query::get_query_instance()->get_query_status() )
         AccContrBaseRuleIsEmpty = false;   
      if ( SubCluster_query::get_query_instance()->get_query_status() ) 
         SubClusterIsEmpty = false;
      if ( SoftwareRunTimeEnvironment_query::get_query_instance()->get_query_status() )
         SoftwareRunTimeEnvironmentIsEmpty = false;
      if ( CESEBind_query::get_query_instance()->get_query_status() )
         CESEBindIsEmpty = false;


      while ( ( !AccContrBaseRuleIsEmpty ) || ( !SubClusterIsEmpty ) || 
              ( !SoftwareRunTimeEnvironmentIsEmpty) || ( !CESEBindIsEmpty ) ) {

         if ( ! AccContrBaseRuleIsEmpty ) {
         ResultSet accSet;
            if(AccessControlBaseRule_query::get_query_instance()->pop_tuples( accSet, 1000)){
               if ( accSet.begin() != accSet.end() ) {

                  for ( ResultSet::iterator it=accSet.begin(); it < accSet.end() ; it++ ) {
                     try {
                        string val = it->getString("Value");
                        string GlueCEUniqueID = it->getString("GlueCEUniqueID");
                        gluece_info_iterator elem = gluece_info_container.find(GlueCEUniqueID);
                        if ( elem != gluece_info_container.end() ) {
                           checkListAttr( (elem->second).get(), "GlueCEAccessControlBaseRule", val);
                        }
                        else Warning(GlueCEUniqueID << " not found in GlueCE table, "<<
                                     "but it is present in GlueCEAccessControlBaseRule table");
                     }
                     catch(RGMAException rgmae) {
                        Error("Cannot evaluate tuple returned by GlueCEAccessControlBaseRule table");
                        Error(rgmae.getMessage());
                     }

                  } //for

               }

               if ( accSet.endOfResults() ) AccContrBaseRuleIsEmpty = true;
            }
            else {
               Warning("Failure in poping tuples from the query to AccessControlBaseRule");
               AccContrBaseRuleIsEmpty = true;
            }
         } // if ( ! AccContrBaseRuleIsEmpty )       

         if ( ! SubClusterIsEmpty ) {
         ResultSet subSet;
            if(SubCluster_query::get_query_instance()->pop_tuples( subSet, 1000)){
               if ( subSet.begin() != subSet.end() ) {
                  for ( ResultSet::iterator tuple = subSet.begin(); tuple < subSet.end(); tuple++) {

                     try {
                        string GlueSubClusterUniqueIDFromRgma = tuple->getString("UniqueID");
                        for (gluece_info_iterator it = gluece_info_container.begin(); it != gluece_info_container.end(); ++it) {
                           string GlueClusterUniqueID;
                           if ( ((it->second).get())->EvaluateAttrString("GlueClusterUniqueID", GlueClusterUniqueID) ){
                              if ( GlueClusterUniqueID == GlueSubClusterUniqueIDFromRgma ) {
                                 boost::scoped_ptr<ClassAd> subClusterAd ( new ClassAd() );
                                 if ( ExportClassAd( subClusterAd.get(), *tuple ) ){
                                    checkSubCluster( subClusterAd.get() );
                                    ((it->second).get())->Update( *subClusterAd.get() );
                                 }
                                 else Warning("Failure in updating SubCluster values for "
                                              <<GlueClusterUniqueID<<".");
                              }
                           }
                           else Warning("Cannot find GlueClusterUniqueID field in the ClassAd");
                        }
                     }
                     catch(RGMAException rgmae) {
                        Error("Cannot evaluate tuple returned by GlueCESubCluster table");
                        Error(rgmae.getMessage());
                     }

                  } //for

               }
               if ( subSet.endOfResults() ) SubClusterIsEmpty = true;

            }
            else {
               Warning("Failure in popping tuples from query to GlueSubCluster");
               SubClusterIsEmpty = true;
            }
 
         } // if ( ! SubClusterIsEmpty )

         if ( !SoftwareRunTimeEnvironmentIsEmpty ){
         ResultSet softSet;
            if(SoftwareRunTimeEnvironment_query::get_query_instance()->pop_tuples( softSet, 1000)){
               if ( softSet.begin() != softSet.end() ) {
                  for( ResultSet::iterator it=softSet.begin(); it < softSet.end(); it++) {

                     try {
                        string GlueSubClusterUniqueIDFromRgma = it->getString("GlueSubClusterUniqueID");
                        string val = it->getString("Value");
 
                        for (gluece_info_iterator it = gluece_info_container.begin(); it != gluece_info_container.end(); ++it) {
                           string GlueSubClusterUniqueID;
                           if ( ((it->second).get())->EvaluateAttrString("GlueSubClusterUniqueID", GlueSubClusterUniqueID) ){
   
                                 if ( GlueSubClusterUniqueID == GlueSubClusterUniqueIDFromRgma )  
                                     checkListAttr( (it->second).get(), "GlueHostApplicationSoftwareRunTimeEnvironment", val );
                           }
                           // Warning log misses since it could happen that GlueSubClusterUniqueID is not
                           // present in the ClassAd. An entry in GlueSubCluster table may not have the
                           // the corresponding one in GlueCEUniqueID table
                        }
                     }
                     catch(RGMAException rgmae) {
                        Error("Cannot evaluate tuple returned by GlueCESubClusterSoftwareRunTimeEnvironment table");
                        Error(rgmae.getMessage());
                     }

                  } //for

               }

               if ( softSet.endOfResults() ) SoftwareRunTimeEnvironmentIsEmpty = true;
            }
            else {
               Warning("Failure in popping tuples from query to GlueSubClusterSoftwareRunTimeEnvironment");
               SoftwareRunTimeEnvironmentIsEmpty = true;
            }
         } // if ( ! SoftwareRunTimeEnvironmentIsEmpty )

         if ( ! CESEBindIsEmpty ) {
         ResultSet bindSet;
            if(CESEBind_query::get_query_instance()->pop_tuples( bindSet, 1000)){
               if ( bindSet.begin() != bindSet.end() ) {
                  for( ResultSet::iterator it=bindSet.begin(); it < bindSet.end(); it++) {

                     boost::scoped_ptr<ClassAd> el(new ClassAd());
                     try {
                        string GlueCEUniqueID = it->getString("GlueCEUniqueID");
                        string GlueSEUniqueID = it->getString("GlueSEUniqueID") ;
                        gluece_info_iterator elem = gluece_info_container.find( GlueCEUniqueID );
                        if ( elem != gluece_info_container.end() ) {
                           //1 
                           if ( ! (elem->second)->Lookup("GlueCESEBindGroupCEUniqueID") )  {
                              (elem->second)->InsertAttr("GlueCESEBindGroupCEUniqueID", GlueCEUniqueID );
                           }
                           //2
                           checkListAttr( (elem->second).get(), "GlueCESEBindGroupSEUniqueID",
                                          GlueSEUniqueID );
                           //3 
                           if ( ( el->InsertAttr("name", GlueSEUniqueID ) )   &&
                                ( el->InsertAttr("mount", it->getString("Accesspoint")) ) ) {
                    
                              if (  !checkClassAdListAttr( (elem->second).get(), "CloseStorageElements",
                                                           static_cast<ClassAd*>(el->Copy()) )  ){
                                 Warning("Failure in adding element to CloseStorageElements list-value for \""
                                         <<GlueCEUniqueID<<"\".");
                              }
                           }
                        }
                        else 
                           Warning( GlueCEUniqueID << " not found in GlueCE table, "
                                    <<"but is present in GlueCESEBind table"); 
                     }
                     catch(RGMAException rgmae) {
                        Error("Cannot evaluate tuple returned by GlueCEAccessControlBaseRule table");
                        Error(rgmae.getMessage());
                     }

                  }//for

               }
               if ( bindSet.endOfResults() ) CESEBindIsEmpty = true;
               
            }
            else{
               Warning("Failure in popping tuples from the query to CESEBind");
               CESEBindIsEmpty = true;
            }

         }  // if ( ! CESEBindIsEmpty )


      } // while  (  (! AccContrBaseRuleIsEmpty ) || ( !SubClusterIsEmpty ) || ( !SoftwareRunTimeEnvironmentIsEmpty) || ( !CESEBindIsEmpty )  )  

      try{
   
         for (gluece_info_iterator it = gluece_info_container.begin();
              it != gluece_info_container.end(); ++it) {
   
            if (m_skip_predicate.empty() || !m_skip_predicate(it->first)) {
   
               bool purchasing_ok = checkMainValue((it->second).get())     && 
                                    expand_glueceid_info(it->second)       &&
                                    insert_aux_requirements(it->second);
   
               if (purchasing_ok) {
                  it->second->InsertAttr("PurchasedBy","ism_rgma_purchaser");
                  gluece_info_container_updated_entries.push_back(it);
   
                  string GlueCEUniqueID="NO";
                  string GlueCEAccessControlBaseRule = "NO";
                  string GlueHostApplicationSoftwareRunTimeEnvironment = "NO";
                  string CloseStorageElements = "NO";
                  if ( it->second->EvaluateAttrString("GlueCEUniqueID", GlueCEUniqueID) ) {
                     if( it->second->Lookup("GlueCEAccessControlBaseRule") ) GlueCEAccessControlBaseRule="";
                     if( it->second->Lookup("GlueHostApplicationSoftwareRunTimeEnvironment") ) 
                                                           GlueHostApplicationSoftwareRunTimeEnvironment="";
                     if( it->second->Lookup("CloseStorageElements") ) CloseStorageElements="";
                  }
                  Debug("Purchased \""<<GlueCEUniqueID<<"\" CE with all SubCluster values"<<std::endl<<
                        "           with "<<GlueCEAccessControlBaseRule<<" GlueCEAccessControlBaseRule attribute"<<std::endl<<
                        "           with "<<GlueHostApplicationSoftwareRunTimeEnvironment
                                      <<" GlueHostApplicationSoftwareRunTimeEnvironment attribute"<< std::endl<<
                        "           with "<<CloseStorageElements<<" CloseStorageElements attribute");
               }
            }
         }
   
         {
            ism_mutex_type::scoped_lock l(get_ism_mutex());	
            while(!gluece_info_container_updated_entries.empty()) {
   	  
               ism_type::value_type ism_entry = make_ism_entry(
                  gluece_info_container_updated_entries.back()->first, 
                  static_cast<int>(get_current_time().sec), 
                  gluece_info_container_updated_entries.back()->second, 
                  ism_rgma_purchaser_entry_update() );
   
   	       get_ism().insert(ism_entry);
   
                  gluece_info_container_updated_entries.pop_back();            
            } // while
         } // unlock the mutex
         if (m_mode) {
            boost::xtime xt;
            boost::xtime_get(&xt, boost::TIME_UTC);
            xt.sec += m_interval;
            boost::mutex::scoped_lock l(f_rgma_purchasing_cycle_run_mutex);
            f_rgma_purchasing_cycle_run_condition.timed_wait(l, xt);
         }
      }
      catch (...) { // TODO: Check which exception may arrive here... and remove catch all
         Warning("FAILED TO PURCHASE INFO FROM RGMA.");
      }

   } 
   while (m_mode && (m_exit_predicate.empty() || !m_exit_predicate()));

}

ism_rgma_purchaser::~ism_rgma_purchaser() {
   gluece_query::destroy_query_instance();
   AccessControlBaseRule_query::destroy_query_instance();
   SubCluster_query::destroy_query_instance();
   SoftwareRunTimeEnvironment_query::destroy_query_instance();
   CESEBind_query::destroy_query_instance();
}


// the class factories
extern "C" ism_rgma_purchaser* create_rgma_purchaser(
                   int rgma_query_timeout,
                   exec_mode_t mode,
                   int rgma_consumer_ttl,
                   int rgma_cons_life_cycles,
                   size_t interval,
                   exit_predicate_type exit_predicate,
                   skip_predicate_type skip_predicate
                   ) 
{
   return new ism_rgma_purchaser(rgma_query_timeout, mode, rgma_consumer_ttl,
                                 rgma_cons_life_cycles, interval, 
                                 exit_predicate, skip_predicate
                                );
}

extern "C" void destroy_rgma_purchaser(ism_rgma_purchaser* p) {
    delete p;
}

// the entry update function factory
extern "C" boost::function<bool(int&, ad_ptr)> create_rgma_entry_update_fn() 
{
  return ism_rgma_purchaser_entry_update();
}



} // namespace purchaser
} // namespace ism
} // namespace wms
} // namespace glite


