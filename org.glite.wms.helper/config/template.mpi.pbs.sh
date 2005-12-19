#!/bin/sh

globus_url_retry_copy()
{
  count=0
  succeded=0
  while [ $count -le ${__file_tx_retry_count} -a $succeded -eq 0 ];
  do
    sleep_time=`expr \( $count \) \* \( $count \)`
    sleep "$sleep_time"
    count=`expr $count + 1`
    globus-url-copy $1 $2
    succeded=$?
  done
  return $succeded
}

doExit() {
  stat=$1

  echo "jw exit status = ${stat}"
  echo "jw exit status = ${stat}" >> "${maradona}"

  globus_url_retry_copy "file://${workdir}/${maradona}" "${__maradonaprotocol}"

  cd ..
  rm -rf ${newdir}

  exit $stat
}

doDSUploadTmp()
{
  filename="$__dsupload"
  echo "#" >> $filename.tmp
  echo "# Autogenerated by JobWrapper!" >> $filename.tmp
  echo "#" >> $filename.tmp
  echo "# The file contains the results of the upload and registration" >> $filename.tmp
  echo "# process in the following format:" >> $filename.tmp
  echo "# <outputfile> <lfn|guid|Error>" >> $filename.tmp
  echo "" >> $filename.tmp
}

doDSUpload()
{
  filename="$__dsupload"
  mv -fv $filename.tmp $filename
}

doCheckReplicaFile()
{
  sourcefile=$1
  filename="$__dsupload"
  exit_status=0
  if [ ! -f "${workdir}/$sourcefile" ]; then
    echo "$sourcefile    Error: File $sourcefile has not been found on the WN $host" >> $filename.tmp
    exit_status=1
  fi
  echo "" >> $filename.tmp
  return $exit_status
}

doReplicaFile()
{
  sourcefile=$1
  filename=$__dsupload
  exit_status=0

  local=`$GLITE_WMS_LOCATION/bin/edg-rm --vo=${__vo} copyAndRegisterFile file://${workdir}/$sourcefile 2>&1`
  result=$?
  if [ $result -eq 0 ]; then
    echo "$sourcefile    $local" >> $filename.tmp
  else
    echo "$sourcefile    Error: $local" >> $filename.tmp
    exit_status=1
  fi
  
  echo "" >> $filename.tmp
  return $exit_status
}

doReplicaFilewithLFN()
{
  sourcefile=$1
  lfn=$2
  filename=$__dsupload
  exit_status=0
  
  local=`$GLITE_WMS_LOCATION/bin/edg-rm --vo=${__vo} copyAndRegisterFile file://${workdir}/$sourcefile -l $lfn 2>&1`
  result=$?
  if [ $result -eq 0 ]; then
    echo "$sourcefile    $lfn" >> $filename.tmp
  else
    localnew=`$GLITE_WMS_LOCATION/bin/edg-rm --vo=${__vo} copyAndRegisterFile file://${workdir}/$sourcefile 2>&1`
    result=$?
    if [ $result -eq 0 ]; then
      echo "$sourcefile    $localnew" >> $filename.tmp
    else
      echo "$sourcefile    Error: $local; $localnew" >> $filename.tmp
      exit_status=1
    fi
  fi
  
  echo "" >> $filename.tmp
  return $exit_status
}

doReplicaFilewithSE()
{
  sourcefile=$1
  se=$2
  filename=$__dsupload
  exit_status=0

  local=`$GLITE_WMS_LOCATION/bin/edg-rm --vo=${__vo} copyAndRegisterFile file://${workdir}/$sourcefile -d $se 2>&1`
  result=$?
  if [ $result -eq 0 ]; then
    echo "$sourcefile    $local" >> $filename.tmp
  else
    localnew=`$GLITE_WMS_LOCATION/bin/edg-rm --vo=${__vo} copyAndRegisterFile file://${workdir}/$sourcefile 2>&1`
    result=$?
    if [ $result -eq 0 ]; then
      echo "$sourcefile    $localnew" >> $filename.tmp
    else
      echo "$sourcefile    Error: $local; $localnew" >> $filename.tmp
      exit_status=1
    fi
  fi

  echo "" >> $filename.tmp
  return $exit_status
}

doReplicaFilewithLFNAndSE()
{

  sourcefile=$1
  lfn=$2
  se=$3
  filename=$__dsupload
  exit_status=0

  local=`$GLITE_WMS_LOCATION/bin/edg-rm --vo=${__vo} copyAndRegisterFile file://${workdir}/$sourcefile -l $lfn -d $se 2>&1`
  result=$?
  if [ $result -eq 0 ]; then
    echo "$sourcefile    $lfn" >> $filename.tmp
  else
    localse=`$GLITE_WMS_LOCATION/bin/edg-rm --vo=${__vo} copyAndRegisterFile file://${workdir}/$sourcefile -d $se 2>&1`
    result=$?
    if [ $result -eq 0 ]; then
      echo "$sourcefile    $localse" >> $filename.tmp
    else
      locallfn=`$GLITE_WMS_LOCATION/bin/edg-rm --vo=${__vo} copyAndRegisterFile file://${workdir}/$sourcefile -l $lfn 2>&1`
      result=$?
      if [ $result -eq 0 ]; then 
        echo "$sourcefile    $locallfn" >> $filename.tmp
      else
        localnew=`$GLITE_WMS_LOCATION/bin/edg-rm --vo=${__vo} copyAndRegisterFile file://${workdir}/$sourcefile 2>&1`
        result=$?
        if [ $result -eq 0 ]; then
          echo "$sourcefile    $localnew" >> $filename.tmp
        else
          echo "$sourcefile    Error: $local; $localse; $locallfn; $localnew" >> $filename.tmp
          exit_status=1
        fi    
      fi
    fi
  fi
	  
  echo "" >> $filename.tmp
  return $exit_status
}

function send_partial_file {
  # Use local variables to avoid conflicts with main program
  local TRIGGERFILE=$1
  local DESTURL=$2
  local POLL_INTERVAL=$3
  local FILESUFFIX=partialtrf
  local GLOBUS_RETURN_CODE
  local SLICENAME
  local LISTFILE=`pwd`/listfile.$$
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
    # Retrieve the list of files to be monitored
    if [ "${TRIGGERFILE:0:9}" == "gsiftp://" ]; then
      globus-url-copy ${TRIGGERFILE} file://${LISTFILE}
    elif [ "${TRIGGERFILE:0:8}" == "https://" ]; then
      htcp ${TRIGGERFILE} file://${LISTFILE}
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
        MD5=`expr "$(echo $SRCFILE | md5sum)" : '\([^ ]*\).*'`
        OLDSIZE="OLDSIZE_$MD5"
        COUNTER="COUNTER_$MD5"
        # Initialize variables if referenced for the first time
        if [ -z "${!OLDSIZE}" ]; then eval local $OLDSIZE=0; fi
        if [ -z "${!COUNTER}" ]; then eval local $COUNTER=1; fi
        # Make a snapshot of the current file
        cp $SRCFILE ${SRCFILE}.${FILESUFFIX}
        NEWSIZE=`stat -c %s ${SRCFILE}.${FILESUFFIX}`
        if [ "${NEWSIZE}" -gt "${!OLDSIZE}" ] ; then
          let "DIFFSIZE = NEWSIZE - $OLDSIZE"
          SLICENAME=$SRCFILE.`date +%Y%m%d%H%M%S`_${!COUNTER}
          tail -c $DIFFSIZE ${SRCFILE}.${FILESUFFIX} > $SLICENAME
          if [ "${DESTURL:0:9}" == "gsiftp://" ]; then
            globus-url-copy file://$SLICENAME ${DESTURL}/`basename $SLICENAME`
          elif [ "${DESTURL:0:8}" == "https://" ]; then
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
  export GLITE_WMS_LOCATION="${GLITE_LOCATION:-/opt/glite}"
fi

if [ -z "${EDG_WL_LOCATION}" ]; then
  export EDG_WL_LOCATION="${EDG_LOCATION:-/opt/edg}"
fi

LB_LOGEVENT=${GLITE_WMS_LOCATION}/bin/glite-lb-logevent
if [ ! -f "$LB_LOGEVENT" ]; then
  LB_LOGEVENT="${EDG_WL_LOCATION}/bin/edg-wl-logev"
fi

if [ ${__create_subdir} -eq 1 ]; then
  newdir="${__jobid_to_filename}"
  mkdir -p .mpi/${newdir}
  if [ $? != 0 ]; then
    echo "Cannot create .mpi/${newdir} directory"

    GLITE_WMS_SEQUENCE_CODE=`$GLITE_WMS_LOCATION/bin/glite-lb-logevent \
      --jobid="$GLITE_WMS_JOBID" \
      --source=LRMS \
      --sequence="$GLITE_WMS_SEQUENCE_CODE"\
      --event="Done"\
      --reason="Cannot create .mpi/${newdir} directory"\
      --status_code=FAILED\
      --exit_code=0\
      || echo $GLITE_WMS_SEQUENCE_CODE`
    export GLITE_WMS_SEQUENCE_CODE

    exit 1
  fi
fi
cd .mpi/${newdir}

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
workdir="`pwd`"

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
    globus_url_retry_copy "${__input_base_url}${f}" "file://${workdir}/${f}"
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
    globus_url_retry_copy "${f}" "file://${workdir}/${file}"
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

if [ -e "${__job}" ]; then
  chmod +x "${__job}" 2> /dev/null
else
  echo "${__job} not found or unreadable"
  echo "${__job} not found or unreadable" >> "${maradona}"

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

if [ $__token_support -eq 1 ]; then
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
fi

HOSTFILE=${PBS_NODEFILE}
for i in `cat $HOSTFILE`; do
  ssh ${i} mkdir -p `pwd`
  /usr/bin/scp -rp ./* "${i}:`pwd`"
  ssh ${i} chmod 755 `pwd`/${__job}
done

(
  cmd_line="mpirun -np ${__nodes} -machinefile ${HOSTFILE} ${__job} ${__arguments} $*"
  if [ -n "${__standard_input}" ]; then
    cmd_line="$cmd_line < ${__standard_input}"
  fi
  if [ -n "${__standard_output}" ]; then
    cmd_line="$cmd_line > ${__standard_output}"
  else
    cmd_line="$cmd_line > /dev/null"
  fi
  if [ -n "${__standard_error}" ]; then
    if [ -n "${__standard_output}" ]; then
      if [ "${__standard_error}" = "${__standard_output}" ]; then
        cmd_line="$cmd_line 2>&1"
      else
        cmd_line="$cmd_line 2>${__standard_error}"
      fi
    fi
  else
    cmd_line="$cmd_line 2 > /dev/null"
  fi

  perl -e '
    unless (defined($ENV{"EDG_WL_NOSETPGRP"})) {
      $SIG{"TTIN"} = "IGNORE";
      $SIG{"TTOU"} = "IGNORE";
      setpgrp(0, 0);
    }
    exec(@ARGV);
    warn "could not exec $ARGV[0]: $!\n";
    exit(127);
  ' "$cmd_line" &

  user_job=$!

  exec 2> /dev/null

  perl -e '
    while (1) {
      $time_left = `grid-proxy-info -timeleft 2> /dev/null` || 0;
      last if ($time_left <= 0);
      sleep($time_left);
    }
    kill(defined($ENV{"EDG_WL_NOSETPGRP"}) ? 9 : -9, '"$user_job"');
    exit(1);
    ' &

  watchdog=$!

  wait $user_job
  status=$?

  kill -9 $watchdog $user_job -$user_job

  exit $status
)

status=$?

if [ $__perusal_support -eq 1 ]; then
  kill -USR2 $send_pid
  wait $send_pid 
fi

if [ ${__output_data} -eq 1 ]; then
  return_value=0
  if [ $status -eq 0 ]; then
    local=`doDSUploadTmp`
    status=$?
    return_value=$status
    local_cnt=0
    for outputfile in ${__output_file[@]}
    do
      local=`doCheckReplicaFile ${__output_file}`
      status=$?
      if [ $status -ne 0 ]; then
        return_value=1
      else
        if [ -z "${__output_lfn}" -a -z "${__output_se}"] ; then
	       local=`doReplicaFile $outputfile`
        elif [ -n "${__output_lfn}" -a -z "${__output_se}"] ; then
	       local=`doReplicaFilewithLFN $outputfile ${__output_lfn[local_cnt]}`
        elif [ -z "${__output_lfn}" -a -n "${__output_se}"] ; then
          local=`doReplicaFilewithSE $outputfile ${__output_se[local_cnt]}`
        else
	       local=`doReplicaFilewithLFNAndSE $outputfile ${__output_lfn[local_cnt]} ${__output_se[local_cnt]}`
        fi
        status=$?
      fi
      local_cnt=`expr $local_cnt+1`
    done
    local=`doDSUpload`
    status=$?
  fi
fi

echo "job exit status = ${status}"
echo "job exit status = ${status}" >> "${maradona}"

error=0
if [ $__wmp_support -eq 0 ]; then
  file_size_acc=0
  for f in ${__output_file[@]} 
  do
    if [ -r "${f}" ]; then
      output=`dirname $f`
      if [ "x${output}" = "x." ]; then
        ff=$f
      else
       ff=${f##*/}
      fi
      if [ ${__max_osb_size} -ge 0 ]; then
        file_size=`stat -t $f | awk '{print $2}'`
        file_size_acc=`expr $file_size_acc + $file_size`
        if [ $file_size_acc -le ${__max_osb_size} ]; then
          globus_url_retry_copy "file://${workdir}/${f}" "${__output_base_url}${ff}"
        else
          echo "OSB quota exceeded for file://${workdir}/${f}"
        fi
      else
        globus_url_retry_copy "file://${workdir}/${f}" "${__output_base_url}${ff}"
      fi
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
  file_size_acc=0
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
      if [ ${__max_osb_size} -ge 0 ]; then
        file_size=`stat -t $f | awk '{print $2}'`
        file_size_acc=`expr $file_size_acc + $file_size`
        if [ $file_size_acc -le ${__max_osb_size} ]; then
          globus_url_retry_copy "file://${workdir}/${file}" "${f}"
        else
          echo "OSB quota exceeded for file://${workdir}/${f}"
        fi
      else
          globus_url_retry_copy "file://${workdir}/${file}" "${f}"
      fi
      if [ $? != 0 ]; then
        echo "Cannot upload ${file} into ${f}"
        echo "Cannot upload ${file} into ${f}" >> "${maradona}"

        export GLITE_WMS_SEQUENCE_CODE=`$GLITE_WMS_LOCATION/bin/glite-lb-logevent \
         --jobid="$GLITE_WMS_JOBID" \
         --source=LRMS \
         --sequence="$GLITE_WMS_SEQUENCE_CODE" \
         --event="Done" \
         --reason="Cannot upload ${file} into ${f}" \
         --status_code=FAILED \
         --exit_code=0 \
         || echo $GLITE_WMS_SEQUENCE_CODE`
        doExit 1
      fi
    fi
  done
fi

export GLITE_WMS_SEQUENCE_CODE=`$LB_LOGEVENT \
 --jobid="$GLITE_WMS_JOBID" \
 --source=LRMS \
 --sequence="$GLITE_WMS_SEQUENCE_CODE" \
 --event="Done" \
 --status_code=OK \
 --exit_code=$status \
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
