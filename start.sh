#!/bin/bash

arch=`uname -m`
if [[ ${arch} = "x86_64" ]]; then
    cd ~/socketcan && bash start.sh
elif [[ ${arch} = "aarch64" ]]; then
    sudo ip link set can0 type can bitrate 500000
    sudo ip link set can1 type can bitrate 500000
    sudo ip link set up can0
    sudo ip link set up can1
    
    # 相机触发（以天准为例）
    tztek-jetson-tool-internal-trigger-camera /dev/ttyTHS1 10 1000 >/dev/null 2>&1 &
    
    sudo jetson_clocks
    
    sudo chmod 777 /dev/ttyACM0
    sudo chmod 777 /dev/ttysWK1
    sudo chmod 777 /dev/ttysWK3
    sudo chmod 777 /var/run/docker.sock
    
    sudo echo "1" | sudo tee /proc/sys/kernel/core_uses_pid
    sudo echo "/apollo/data/core/%e.core.%p" | sudo tee /proc/sys/kernel/core_pattern

else
    echo "unsupported arch: ${arch}"
    exit 1
fi

