catkin init
catkin config --merge-devel
catkin config --cmake-args -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_STANDARD=14 -DCMAKE_CXX_FLAGS=-Wall
catkin build
# -DGPU=1 use GPU, -DGPU=0 use CPU
# -DSHARED_MEMORY=1 on 3588, -DSHARED_MEMORY=0 on PC
