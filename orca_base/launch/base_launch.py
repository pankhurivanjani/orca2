"""Launch orca_base"""

from launch import LaunchDescription
from launch_ros.actions import Node


def generate_launch_description():
    return LaunchDescription([
        Node(package='joy', node_executable='joy_node', output='screen'),
        Node(package='orca_base', node_executable='orca_base', output='screen')
    ])
