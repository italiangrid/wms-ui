// File: JCDeliveryPolicy.cpp
// Author: Francesco Giacomini <Francesco.Giacomini@cnaf.infn.it>
// Copyright (c) 2002 EU DataGrid.
// For license conditions see http://www.eu-datagrid.org/license.html

// $Id$

#include "edg/workload/planning/manager/JCDeliveryPolicy.h"

#include <string>
#include <classad_distribution.h>

#include "dispatching_utils.h"
#include "../common/lb_utils.h"

#include "glite/wms/common/logger/logger_utils.h"
#include "glite/wms/jobcontrol/controller/JobController.h"

#include "glite/wmsutils/jobid/JobId.h"

#include "glite/wms/jdl/JobAdManipulation.h"

namespace glite {
namespace wms {
namespace manager {
namespace server {

JCDeliveryPolicy::~JCDeliveryPolicy()
{
}

void
JCDeliveryPolicy::Deliver(classad::ClassAd const& ad)
{
  wmsutils::jobid::JobId id(wms::jdl::get_edg_jobid(ad));

  boost::mutex::scoped_lock l(submit_cancel_mutex());
  ContextPtr context_ptr = get_context(id);
  if (unregister_context(id)) {
    Debug("delivering job " << id);
    edg_wll_Context context = *context_ptr;
    jobcontrol::controller::JobController(&context).submit(&ad);
  }
}

} // server
} // manager
} // wms
} // glite

