// File: signal_handling.h
// Author: Francesco Giacomini <Francesco.Giacomini@cnaf.infn.it>
// Copyright (c) 2002 EU DataGrid.
// For license conditions see http://www.eu-datagrid.org/license.html

// $Id$

#ifndef GLITE_WMS_MANAGER_SERVER_SIGNAL_HANDLING_H
#define GLITE_WMS_MANAGER_SERVER_SIGNAL_HANDLING_H

namespace glite {
namespace wms {
namespace manager {
namespace server {

void signal_handling_init();
bool received_quit_signal();

} // server
} // manager
} // wms
} // glite

#endif

// Local Variables:
// mode: c++
// End:
