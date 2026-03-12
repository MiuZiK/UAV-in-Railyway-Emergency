#!/bin/bash
output_dir="bag"
mkdir -p "$output_dir"
timestamp=$(date + "%Y%m%d_%H%M%S")
bag_name="${timestamp}.bag"

echo "begin to record rosbag:$bag_name"
rosbag record --lz4  -o "$bag_name" /livox/imu /livox/lidar /tf /rosout /rosout_agg /mavros/state /mavros/sys_status /cloud_registered 
