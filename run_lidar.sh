pkill gzclient
sleep 1
pkill gzserver
sleep 1
BASEDIR=$(dirname $(realpath $0))

source $BASEDIR/devel/setup.bash
source ~/PX4-Autopilot/Tools/setup_gazebo.bash ~/PX4-Autopilot ~/PX4-Autopilot/build/px4_sitl_default
export ROS_PACKAGE_PATH=$ROS_PACKAGE_PATH:~/PX4-Autopilot
export ROS_PACKAGE_PATH=$ROS_PACKAGE_PATH:~/PX4-Autopilot/Tools/sitl_gazebo
export GAZEBO_PLUGIN_PATH=$GAZEBO_PLUGIN_PATH:/usr/lib/x86_64-linux-gnu/gazebo-11/plugins
export GAZEBO_MODEL_PATH=$GAZEBO_MODEL_PATH:$BASEDIR/src/px4_ego/models

export PX4_SIM_SPEED_FACTOR=1.0

roslaunch px4_ego gazebo_lidar_px4_ego_planner.launch
