#!/bin/bash
rosbag record --lz4 /livox/imu /livox/lidar /tf /hikrobot_camera/rgb -o test.bag
