#!/bin/bash
cd ~/unilidar_sdk/unitree_lidar_ros
BASEDIR=$(dirname $(realpath $0))

source $BASEDIR/devel/setup.bash


roslaunch point_lio_unilidar mapping_unilidar_l1.launch
