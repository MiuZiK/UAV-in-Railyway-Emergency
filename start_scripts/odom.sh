#!/bin/bash
# cd ~/odom_ws
# BASEDIR=$(dirname $(realpath $0))

# source $BASEDIR/devel/setup.bash
cd /home/orangepi/odom_ws/
source /home/orangepi/odom_ws/devel/setup.bash

roslaunch fast_lio mapping_mid360.launch
