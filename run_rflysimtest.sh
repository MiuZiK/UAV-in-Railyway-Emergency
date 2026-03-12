BASEDIR=$(dirname $(realpath $0))

source $BASEDIR/devel/setup.bash

export PX4_SIM_SPEED_FACTOR=1.0

roslaunch px4_ego rflysim_lidar_px4_ego_planner.launch