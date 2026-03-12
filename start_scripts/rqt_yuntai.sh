#!/bin/bash
cd ~/yt_ws
BASEDIR=$(dirname $(realpath $0))

source $BASEDIR/devel/setup.bash

rosrun rqt_yuntai rqt_yuntai
