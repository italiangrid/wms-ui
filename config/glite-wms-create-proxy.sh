# Copyright (c) Members of the EGEE Collaboration. 2004.
# See http://project.eu-egee.org/partners/ for details on the
# copyright holders.

# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
# either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

#!/bin/bash

create_proxy()
{
   proxy=`"${GLOBUS_LOCATION}/bin/grid-proxy-init" -q \
     -cert $GLITE_HOST_CERT \
     -key  $GLITE_HOST_KEY \
     -hours 24 \
     -out  "$1.$$" 2>&1`
   
   [ $? -eq 0 ] || echo $proxy " Fail " >> $2

   move=`chmod 400 "$1.$$" &&
     chown "${GLITE_WMS_USER}" "$1.$$" &&
     mv -f "$1.$$" "$1" 2>&1`

   [ $? -eq 0 ] || echo $move " Fail " >> $2
}

create_proxy `${GLITE_WMS_LOCATION}/bin/glite-wms-get-configuration Common.HostProxyFile` | /usr/bin/logger
