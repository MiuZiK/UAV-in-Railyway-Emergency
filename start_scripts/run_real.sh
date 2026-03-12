#!/bin/bash
# cd ~/lan_planner
# BASEDIR=$(dirname $(realpath $0))

# source $BASEDIR/devel/setup.bash

cd /home/orangepi/lan_planner/
source /home/orangepi/lan_planner/devel/setup.bash

roslaunch px4_ego real_fly_with_mid360.launch
