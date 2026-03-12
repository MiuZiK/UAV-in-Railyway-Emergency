#!/bin/bash
cd ~/lan_planner
BASEDIR=$(dirname $(realpath $0))

source $BASEDIR/devel/setup.bash


roslaunch px4_ego odom_cvt.launch
