#!/bin/bash

# 定义摄像头参数数组（名称、频道后缀、帧率、设备号）
declare -a CAMERAS=(
    "Narrow,camera_narrow,15,0"        # 12mm，窄视角，较高帧率
    "Wide,camera_wide,10,1"            # 2.3mm，广角，基础帧率
    "obstacle,camera_obstacle,20,2"    # 6mm，障碍物检测，高帧率
    "traffic,camera_traffic,5,3"       # 25mm，交通标志识别，低帧率
    "rightforward,camera_rightforward,10,4"
    "rightbackward,camera_rightbackward,15,5"
    "backward,camera_backward,20,6"
    "leftbackward,camera_leftbackward,15,7"
    "leftforward,camera_leftforward,10,8"  # 修正拼写为 leftforward
)

for cam in "${CAMERAS[@]}"; do
    IFS=',' read -r name channel_suffix frame_rate device_num <<< "$cam"
    
    # 生成文件名（如 Narrow.pb.txt）
    output_file="${name}.pb.txt"
    
    # 替换模板参数
    sed -e "s/{CAMERA_FRAME_ID}/$name/g" \
        -e "s/{CHANNEL_SUFFIX}/$channel_suffix/g" \
        -e "s/{FRAME_RATE}/$frame_rate/g" \
        -e "s/{DEVICE_NUM}/$device_num/g" \
        camera_template.pb.txt > "$output_file"
done

echo "生成9个摄像头配置文件:"
ls -1 *.pb.txt
