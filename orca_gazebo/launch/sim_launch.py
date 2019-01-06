"""Launch a simulation, with all the bells and whistles"""

# TODO(crystal): run xacro

import os

from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch_ros.actions import Node
from launch.actions import ExecuteProcess


def generate_launch_description():
    urdf = os.path.join(get_package_share_directory('orca_description'), 'urdf', 'orca.urdf')
    world = os.path.join(get_package_share_directory('orca_gazebo'), 'worlds', 'orca.world')
    return LaunchDescription([
        ExecuteProcess(cmd=['gazebo', '--verbose', world], output='screen'),
        ExecuteProcess(cmd=['rviz2'], output='screen'),
        Node(package='robot_state_publisher', node_executable='robot_state_publisher', output='screen', arguments=[urdf]),
        Node(package='joy', node_executable='joy_node', output='screen'),
        Node(package='orca_base', node_executable='orca_base', output='screen'),
    ])