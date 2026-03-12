#!/bin/bash
cd ~/ros_ws
BASEDIR=$(dirname $(realpath $0))

source $BASEDIR/devel/setup.bash
sleep 1
roslaunch orbbec_camera dabai_dcw.launch
