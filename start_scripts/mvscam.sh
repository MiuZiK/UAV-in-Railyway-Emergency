#!/bin/bash
export ROSDISTRO_INDEX_URL=https://mirrors.tuna.tsinghua.edu.cn/rosdistro/index-v4.yaml
source /opt/ros/noetic/setup.bash
export MVCAM_SDK_PATH=/opt/MVS
export MVCAM_COMMON_RUNENV=/opt/MVS/lib
export MVCAM_GENICAM_CLPROTOCOL=/opt/MVS/lib/CLProtocol
export ALLUSERSPROFILE=/opt/MVS/MVFG
export LD_LIBRARY_PATH=/opt/MVS/lib/aarch64:$LD_LIBRARY_PATH
cd /home/orangepi/ws_hikrobot_camera
BASEDIR=$(dirname $(realpath $0))

source $BASEDIR/devel/setup.bash
roslaunch hikrobot_camera hikrobot_camera.launch
