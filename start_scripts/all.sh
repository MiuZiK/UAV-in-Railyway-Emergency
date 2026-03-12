#!/bin/bash
cd /home/orangepi/lan_planner/start_scripts
gnome-terminal -- bash -c "sleep 10; ./odom.sh; exec bash"
gnome-terminal -- bash -c "sleep 6; ./run_real.sh; exec bash"
gnome-terminal -- bash -c "sleep 1; ./offboard.sh; exec bash"

while true; do
    echo "running  ..."
    sleep 10
done
