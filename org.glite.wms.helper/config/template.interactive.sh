#!/bin/sh

doExit() {
  stat=$1

  echo "jw exit status = ${stat}"
  echo "jw exit status = ${stat}" >> "${maradona}"

  globus-url-copy "file://${workdir}/${maradona}" "${__maradonaprotocol}"

  cd ..
  rm -rf ${newdir}

  exit $stat
}

function send_partial_file {
  # Use local variables to avoid conflicts with main program
  local TRIGGERFILE=$1
  local DESTURL=$2
  local POLL_INTERVAL=$3
  local FILESUFFIX=partialtrf
  local GLOBUS_RETURN_CODE
  local SLICENAME
  local LISTFILE=`pwd`/listfile.$!
  local LAST_CYCLE=""
  local SLEEP_PID
  local MD5
  local OLDSIZE
  local NEWSIZE
  local COUNTER
  # Loop forever (need to be killed by main program)
  while [ -z "$LAST_CYCLE" ] ; do
    # Go to sleep, but be ready to wake up when the user job finishes
    sleep $POLL_INTERVAL & SLEEP_PID=$!
    trap 'LAST_CYCLE="y"; kill -ALRM $SLEEP_PID >/dev/null 2>&1' USR2
    wait $SLEEP_PID >/dev/null 2>&1
    # Retrive the list of files to be monitored
    tmpdemo=`echo $TRIGGERFILE | awk -F "://" '{print $1}'`
    if [ "$tmpdemo" == "gsiftp" ]; then
      globus-url-copy ${TRIGGERFILE} file://${LISTFILE}
    elif [ "$tmpdemo" == "https" ]; then
      htcp ${TRIGGERFILE} file://${file}
    fi
    # Skip iteration if unable to get the list
    # (can be used to switch off monitoring)
    if [ "$?" -ne "0" ] ; then
      continue
    fi
    for SRCFILE in `cat $LISTFILE` ; do
      # SRCFILE must contain the full path
      if [ "$SRCFILE" == "`basename $SRCFILE`" ] ; then
        SRCFILE=`pwd`/$SRCFILE
      fi
      if [ -f $SRCFILE ] ; then
        # Point to the "state" variables of the current file
        # (we will use indirect reference)
        MD5=`echo $SRCFILE | md5sum | awk '{ print $1 }'`
        OLDSIZE="OLDSIZE_$MD5"
        COUNTER="COUNTER_$MD5"
        # Initialize variables if referenced for the first time
        if [ -z "${!OLDSIZE}" ]; then eval local $OLDSIZE=0; fi
        if [ -z "${!COUNTER}" ]; then eval local $COUNTER=1; fi
        # Make a snapshot of the current file
        cp $SRCFILE ${SRCFILE}.${FILESUFFIX}
        let "NEWSIZE = `stat -t ${SRCFILE}.${FILESUFFIX} | awk '{ print $2 }'`"
        if [ "${NEWSIZE}" -gt "${!OLDSIZE}" ] ; then
          let "DIFFSIZE = NEWSIZE - $OLDSIZE"
          SLICENAME=$SRCFILE.`date +%Y%m%d%H%m%S`_${!COUNTER}
          tail -c $DIFFSIZE ${SRCFILE}.${FILESUFFIX} > $SLICENAME
          tmpdemo=`echo $DESTURL | awk -F "://" '{print $1}'`
          if [ "$tmpdemo" == "gsiftp" ]; then
            globus-url-copy file://$SLICENAME ${DESTURL}/`basename $SLICENAME`
          elif [ "$tmpdemo" == "https" ]; then
            htcp file://$SLICENAME ${DESTURL}/`basename $SLICENAME`
          fi
          GLOBUS_RETURN_CODE=$?
          rm ${SRCFILE}.${FILESUFFIX} $SLICENAME
          if [ "$GLOBUS_RETURN_CODE" -eq "0" ] ; then
            let "$OLDSIZE = NEWSIZE"
            let "$COUNTER += 1"
          fi # else we will send this chunk toghether with the next one
        fi # else the file size didn't increase
      fi
    done
  done
  # Do some cleanup
  if [ -f "$LISTFILE" ] ; then rm $LISTFILE ; fi
}

if [ "${__input_base_url}:-1" != "/" ]; then
  __input_base_url="${__input_base_url}/"
fi

if [ "${__output_base_url}:-1" != "/" ]; then
  __output_base_url="${__output_base_url}/"
fi

if [ -n "${__gatekeeper_hostname}" ]; then
  export GLITE_WMS_LOG_DESTINATION="${__gatekeeper_hostname}"
fi

if [ -n "${__jobid}" ]; then
  export GLITE_WMS_JOBID="${__jobid}"
fi

GLITE_WMS_SEQUENCE_CODE="$1"
shift

if [ -z "${GLITE_WMS_LOCATION}" ]; then
  export GLITE_WMS_LOCATION="${__edg_location}"
fi

if [ $__create_subdir -eq 1 ]; then
  newdir="${__jobid_to_filename}"
  mkdir ${newdir}
  cd ${newdir}
fi

if [ ! -w . ]; then
  echo "Working directory not writable"

 export GLITE_WMS_SEQUENCE_CODE=`$GLITE_WMS_LOCATION/bin/glite-lb-logevent \
  --jobid="$GLITE_WMS_JOBID" \
  --source=LRMS \
  --sequence="$GLITE_WMS_SEQUENCE_CODE"\
  --event="Done"\
  --reason="Working directory not writable!"\
  --status_code=FAILED\
  --exit_code=0\
  || echo $GLITE_WMS_SEQUENCE_CODE`

  exit 1
fi

if [ -n "${__brokerinfo}" ]; then
  export GLITE_WMS_RB_BROKERINFO="`pwd`/${__brokerinfo}"
fi

maradona="${__jobid_to_filename}.output"
touch "${maradona}"

if [ -z "${GLOBUS_LOCATION}" ]; then
  echo "GLOBUS_LOCATION undefined"
  echo "GLOBUS_LOCATION undefined" >> "${maradona}"

  export GLITE_WMS_SEQUENCE_CODE=`$GLITE_WMS_LOCATION/bin/glite-lb-logevent \
   --jobid="$GLITE_WMS_JOBID" \
   --source=LRMS \
   --sequence="$GLITE_WMS_SEQUENCE_CODE"\
   --event="Done"\
   --reason="GLOBUS_LOCATION undefined"\
   --status_code=FAILED\
   --exit_code=0\
   || echo $GLITE_WMS_SEQUENCE_CODE`

  doExit 1
elif [ -r "${GLOBUS_LOCATION}/etc/globus-user-env.sh" ]; then
  . ${GLOBUS_LOCATION}/etc/globus-user-env.sh
else
  echo "${GLOBUS_LOCATION}/etc/globus-user-env.sh not found or unreadable"
  echo "${GLOBUS_LOCATION}/etc/globus-user-env.sh not found or unreadable" >> "${maradona}"

  export GLITE_WMS_SEQUENCE_CODE=`$GLITE_WMS_LOCATION/bin/glite-lb-logevent \
   --jobid="$GLITE_WMS_JOBID" \
   --source=LRMS \
   --sequence="$GLITE_WMS_SEQUENCE_CODE"\
   --event="Done"\
   --reason="${GLOBUS_LOCATION}/etc/globus-user-env.sh not found or unreadable"\
   --status_code=FAILED\
   --exit_code=0\
   || echo $GLITE_WMS_SEQUENCE_CODE`

  doExit 1
fi

for env in ${__environment[@]}
do
  eval export $env
done

umask 022

if [ $__wmp_support -eq 0 ]; then
  for f in ${__input_file[@]}
  do
    globus-url-copy "${__input_base_url}${f}" "file://${workdir}/${f}"
    if [ $? != 0 ]; then
      echo "Cannot download ${f} from ${__input_base_url}"
      echo "Cannot download ${f} from ${__input_base_url}" >> "${maradona}"

      export GLITE_WMS_SEQUENCE_CODE=`$GLITE_WMS_LOCATION/bin/glite-lb-logevent \
       --jobid="$GLITE_WMS_JOBID" \
       --source=LRMS \
       --sequence="$GLITE_WMS_SEQUENCE_CODE"\
       --event="Done"\
       --reason="Cannot download ${f} from ${__input_base_url}"\
       --status_code=FAILED\
       --exit_code=0\
       || echo $GLITE_WMS_SEQUENCE_CODE`
      doExit 1
    fi
  done
else
  #WMP support
  for f in ${__wmp_input_base_file[@]}
  do
    file=`basename $f`
    globus-url-copy "${f}" "file://${workdir}/${file}"
    if [ $? != 0 ]; then
      echo "Cannot download ${file} from ${f}"
      echo "Cannot download ${file} from ${f}" >> "${maradona}"

      export GLITE_WMS_SEQUENCE_CODE=`$GLITE_WMS_LOCATION/bin/glite-lb-logevent \
       --jobid="$GLITE_WMS_JOBID" \
       --source=LRMS \
       --sequence="$GLITE_WMS_SEQUENCE_CODE"\
       --event="Done"\
       --reason="Cannot download ${file} from ${f}"\
       --status_code=FAILED\
       --exit_code=0\
       || echo $GLITE_WMS_SEQUENCE_CODE`
      doExit 1
    fi
  done
fi

if [ -e "${__jobid}" ]; then
  chmod +x "${__jobid}" 2> /dev/null
else
  echo "${__jobid} not found or unreadable"
  echo "${__jobid} not found or unreadable" >> "${maradona}"

  export GLITE_WMS_SEQUENCE_CODE=`$GLITE_WMS_LOCATION/bin/glite-lb-logevent \
   --jobid="$GLITE_WMS_JOBID" \
   --source=LRMS \
   --sequence="$GLITE_WMS_SEQUENCE_CODE"\
   --event="Done"\
   --reason="/bin/echo not found or unreadable!"\
   --status_code=FAILED\
   --exit_code=0\
   || echo $GLITE_WMS_SEQUENCE_CODE`

  doExit 1
fi

for f in  "glite-wms-pipe-input" "glite-wms-pipe-output" "glite-wms-job-agent" ; do
   globus-url-copy "${__input_base_url}opt/glite/bin/${f} file://${workdir}/${f}"
   chmod +x ${workdir}/${f}
done

globus-url-copy "{__input_base_url}opt/glite/lib/libglite-wms-grid-console-agent.so.0 file://${workdir}/libglite-wms-grid-console-agent.so.0"

host=`hostname -f`
export GLITE_WMS_SEQUENCE_CODE=`$GLITE_WMS_LOCATION/bin/glite-lb-logevent \
 --jobid="$GLITE_WMS_JOBID" \
 --source=LRMS \
 --sequence="$GLITE_WMS_SEQUENCE_CODE"\
 --event="Running"\
 --node=$host\
 || echo $GLITE_WMS_SEQUENCE_CODE`

if [ $__perusal_support -eq 1 ]; then
  send_partial_file ${__perusal_listfileuri} ${__perusal_filesdesturi} ${__perusal_timeinterval} & send_pid=$!
fi

value=`$GLITE_WMS_LOCATION/bin/glite-gridftp-rm $__token_file`
result=$?
if [ $result -eq 0 ]; then
  GLITE_WMS_SEQUENCE_CODE=`$GLITE_WMS_LOCATION/bin/glite-lb-logevent \
 --jobid="$GLITE_WMS_JOBID" \
 --source=LRMS \
 --sequence="$GLITE_WMS_SEQUENCE_CODE"\
 --event="ReallyRunning"\
 --status_code=\
 --exit_code=\
 || echo $GLITE_WMS_SEQUENCE_CODE`
export GLITE_WMS_SEQUENCE_CODE

  echo "Take token: ${GLITE_WMS_SEQUENCE_CODE}"
else
  echo "Cannot take token!"
  echo "Cannot take token!" >> "${maradona}"

  GLITE_WMS_SEQUENCE_CODE=`$GLITE_WMS_LOCATION/bin/glite-lb-logevent \
 --jobid="$GLITE_WMS_JOBID" \
 --source=LRMS \
 --sequence="$GLITE_WMS_SEQUENCE_CODE"\
 --event="Done"\
 --reason="Cannot take token!"\
 --status_code=FAILED\
 --exit_code=0\
 || echo $GLITE_WMS_SEQUENCE_CODE`
export GLITE_WMS_SEQUENCE_CODE

  doExit 1
fi

./glite-wms-job-agent "${BYPASS_SHADOW_HOST} ${BYPASS_SHADOW_PORT} ${__job} ${__arguments} $*"

status=$?

kill -USR2 $send_pid
wait $send_pid 

echo "job exit status = ${status}"
echo "job exit status = ${status}" >> "${maradona}"

error=0
if [ $__wmp_support -eq 0 ]; then
  for f in ${__output_file[@]} 
  do
    if [ -r "${f}" ]; then
      output=`dirname $f`
      if [ "x${output}" = "x." ]; then
        ff=$f
      else
       ff=${f##*/}
      fi
      globus-url-copy "file://${workdir}/${f}" "${__output_base_url}${ff}"
      if [ $? != 0 ]; then
        echo "Cannot upload ${f} into ${__output_base_url}"
        echo "Cannot upload ${f} into ${__output_base_url}" >> "${maradona}"

        export GLITE_WMS_SEQUENCE_CODE=`$GLITE_WMS_LOCATION/bin/glite-lb-logevent \
         --jobid="$GLITE_WMS_JOBID" \
         --source=LRMS \
         --sequence="$GLITE_WMS_SEQUENCE_CODE"\
         --event="Done"\
         --reason="Cannot upload ${f} into ${__output_base_url}"\
         --status_code=FAILED\
         --exit_code=0\
         || echo $GLITE_WMS_SEQUENCE_CODE`
        doExit 1
      fi
    fi
  done
else
  #WMP support
  for f in ${__wmp_output_dest_file[@]} 
  do
    file=`basename $f`
    if [ -r "${file}" ]; then
      output=`dirname $f`
      if [ "x${output}" = "x." ]; then
        ff=$f
      else
       ff=${f##*/}
      fi
      globus-url-copy "file://${workdir}/${file}" "${f}"
      if [ $? != 0 ]; then
        echo "Cannot upload ${file} into ${f}"
        echo "Cannot upload ${file} into ${f}" >> "${maradona}"

        export GLITE_WMS_SEQUENCE_CODE=`$GLITE_WMS_LOCATION/bin/glite-lb-logevent \
         --jobid="$GLITE_WMS_JOBID" \
         --source=LRMS \
         --sequence="$GLITE_WMS_SEQUENCE_CODE"\
         --event="Done"\
         --reason="Cannot upload ${file} into ${f}"\
         --status_code=FAILED\
         --exit_code=0\
         || echo $GLITE_WMS_SEQUENCE_CODE`
        doExit 1
      fi
    fi
  done
fi

export GLITE_WMS_SEQUENCE_CODE=`$GLITE_WMS_LOCATION/bin/glite-lb-logevent \
 --jobid="$GLITE_WMS_JOBID" \
 --source=LRMS \
 --sequence="$GLITE_WMS_SEQUENCE_CODE"\
 --event="Done"\
 --status_code=OK\
 --exit_code=$status\
 || echo $GLITE_WMS_SEQUENCE_CODE`

if [ -n "${LSB_JOBID}" ]; then
  cat "${X509_USER_PROXY}" | ${GLITE_WMS_LOCATION}/libexec/glite_dgas_ceServiceClient -s ${__gatekeeper_hostname}:56569: -L lsf_${LSB_JOBID} -G ${GLITE_WMS_JOBID} -C ${__globus_resource_contact_string} -H "$HLR_LOCATION"
  if [ $? != 0 ]; then
  echo "Error transferring gianduia with command: cat ${X509_USER_PROXY} | ${GLITE_WMS_LOCATION}/libexec/glite_dgas_ceServiceClient -s ${__gatekeeper_hostname}:56569: -L lsf_${LSB_JOBID} -G ${GLITE_WMS_JOBID} -C ${__globus_resource_contact_string} -H $HLR_LOCATION"
  fi
fi

if [ -n "${PBS_JOBID}" ]; then
  cat ${X509_USER_PROXY} | ${GLITE_WMS_LOCATION}/libexec/glite_dgas_ceServiceClient -s ${__gatekeeper_hostname}:56569: -L pbs_${PBS_JOBID} -G ${GLITE_WMS_JOBID} -C ${__globus_resource_contact_string} -H "$HLR_LOCATION"
  if [ $? != 0 ]; then
  echo "Error transferring gianduia with command: cat ${X509_USER_PROXY} | ${GLITE_WMS_LOCATION}/libexec/glite_dgas_ceServiceClient -s ${__gatekeeper_hostname}:56569: -L pbs_${PBS_JOBID} -G ${GLITE_WMS_JOBID} -C ${__globus_resource_contact_string} -H $HLR_LOCATION"
  fi
fi

doExit 0

