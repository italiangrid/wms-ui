#include "JobControllerFactory.h"
#include "JobControllerClient.h"
#include "JobControllerClientImpl.h"

JOBCONTROL_NAMESPACE_BEGIN {

namespace controller {

JobControllerClient::JobControllerClient( void ) : jcc_impl( JobControllerFactory::instance()->create_client() )
{}

JobControllerClient::~JobControllerClient( void )
{ delete this->jcc_impl; }

void JobControllerClient::release_request( void )
{
  this->jcc_impl->release_request();
}

void JobControllerClient::extract_next_request( void )
{
  this->jcc_impl->extract_next_request();
}

const Request *JobControllerClient::get_current_request( void )
{
  return this->jcc_impl->get_current_request();
}

}; // Namespace controller

} JOBCONTROL_NAMESPACE_END;
