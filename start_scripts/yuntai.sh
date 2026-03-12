#!/bin/bash
# cd ~/lan_planner
# BASEDIR=$(dirname $(realpath $0))

# source $BASEDIR/devel/setup.bash

cd /home/orangepi/lan_planner/
source /home/orangepi/lan_planner/devel/setup.bash

roslaunch --wait px4_ego yuntai.launch
