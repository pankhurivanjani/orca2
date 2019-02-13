# Orca2 #

Orca2 is a [ROS2](https://index.ros.org/doc/ros2/) port of [Orca](https://github.com/clydemcqueen/orca).

Orca is a ROS driver for the BlueROV2.

## Status

The overall goal is to port Orca to ROS2, then continue work on a BlueROV2-based AUV.

* `orca_msgs` is ported
* `orca_driver` is blocked on several packages
* `orca_base` is ported
* `orca_topside` the Rviz2 plugins are building but are not loading correctly
* `orca_gazebo` is ported. The URDF file must be manually converted to SDF
* `orca_vision` is blocked on pcl and possibly other packages

## Requirements

[Install ROS2 Crystal](https://index.ros.org/doc/ros2/Installation/)
with the `ros-crystal-desktop` option.

If you install binaries, be sure to also install the development tools and ROS tools from the
[source installation instructions](https://index.ros.org/doc/ros2/Installation/Linux-Development-Setup/).

Requires a joystick compatible with ROS2.

Install Gazebo v9:

~~~
sudo apt install gazebo9 libgazebo9 libgazebo9-dev
~~~

Install these additional ROS packages:
~~~
sudo apt install ros-crystal-gazebo-ros-pkgs ros-crystal-cv-bridge
~~~

## Install and build

~~~
mkdir -p ~/ros2/orca2_ws/src
cd ~/ros2/orca2_ws/src
git clone https://github.com/clydemcqueen/orca2.git
git clone https://github.com/ptrmu/flock_vlam.git
cd ~/ros2/orca2_ws
source /opt/ros/crystal/setup.bash
colcon build --event-handlers console_direct+
# Ignore the build warnings from orca_topside; this package hasn't been ported yet
~~~

## Run simulation

Run Orca2 in Gazebo:

~~~
cd ~/ros2/orca2_ws
source /opt/ros/crystal/setup.bash
source install/setup.bash
export GAZEBO_MODEL_PATH=~/ros2/orca2_ws/install/orca_gazebo/share/orca_gazebo/models
ros2 launch orca_gazebo sim_launch.py
# See also orca_gazebo/README.md for a possible dynamic linking problem and workaround
~~~

See the [Orca](https://github.com/clydemcqueen/orca) README for more information.