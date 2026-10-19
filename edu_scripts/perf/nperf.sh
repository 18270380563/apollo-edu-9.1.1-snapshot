#!/bin/bash

day=`date '+%Y%m%d_%H%M%S'`
log="/apollo_workspace/data/log/nperf.log.${day}"
touch ${log}
ln -snf nperf.log.${day} /apollo_workspace/data/log/nperf.INFO

while true
do
    # common processes
    pids=`ps -ef | grep -Ew "(mainboard|dreamview|cyber_recorder|cyber_monitor)" | grep -Ev "(cyber_launch|grep|nohup)" | awk '{print $2}' | tr "\n" "," | sed 's/.$//g'`
    echo >>${log}
    date '+%m-%d %H:%M:%S' >>${log}
    top -p ${pids} -bc -w 512 -n 1 >>${log}
    # chrome processes
    echo >>${log}
    ps aux | grep -Ew "chrome" | grep -Ev "grep" | awk '{if($3 > 0.0) {print $0}}' >> ${log}

    echo >>${log}
    if [[ -e /usr/bin/tegrastats ]];then
        timeout 2 /usr/bin/tegrastats --readall >> ${log}
    fi
    if [[ -e /usr/bin/nvidia-smi ]];then
        /usr/bin/nvidia-smi >> ${log}
    fi
    sleep 1
done
