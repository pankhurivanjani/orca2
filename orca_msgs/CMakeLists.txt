cmake_minimum_required(VERSION 3.5)
project(orca_msgs)

# Default to C++14
if (NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 14)
endif ()

# Emulate colcon in CLion
if ($ENV{CLION_IDE})
  message("Running inside CLion")
  find_package(fastrtps_cmake_module REQUIRED)
  set(FastRTPS_INCLUDE_DIR "/opt/ros/eloquent/include")
  set(FastRTPS_LIBRARY_RELEASE "/opt/ros/eloquent/lib/libfastrtps.so")
endif ()

# Debugging: set _dump_all_variables to true
set(_dump_all_variables false)
if (_dump_all_variables)
  get_cmake_property(_variable_names VARIABLES)
  list(SORT _variable_names)
  foreach (_variable_name ${_variable_names})
    message(STATUS "${_variable_name}=${${_variable_name}}")
  endforeach ()
endif ()

# Find packages
find_package(ament_cmake REQUIRED)
find_package(rosidl_default_generators REQUIRED)
find_package(std_msgs REQUIRED)

# Generate ROS messages
rosidl_generate_interfaces(
  orca_msgs
  action/Mission.action
  msg/Barometer.msg
  msg/Battery.msg
  msg/Control.msg
  msg/Depth.msg
  msg/Efforts.msg
  msg/Leak.msg
  msg/Proc.msg
  msg/Pose.msg
  msg/PoseStamped.msg
  DEPENDENCIES std_msgs
)

ament_export_dependencies(rosidl_default_runtime)

ament_package()