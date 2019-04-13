#ifndef ORCA_BASE_GEOMETRY_HPP
#define ORCA_BASE_GEOMETRY_HPP

#include "orca_base/model.hpp"
#include "orca_base/util.hpp"

#include "geometry_msgs/msg/pose_stamped.hpp"
#include "nav_msgs/msg/odometry.hpp"
#include "nav_msgs/msg/path.hpp"
#include "tf2_geometry_msgs/tf2_geometry_msgs.h"

// 4 DoF geometry and motion structs

namespace orca_base {

//=====================================================================================
// Pose
//=====================================================================================

struct Pose
{
  double x;
  double y;
  double z;
  double yaw;

  constexpr Pose(): x{0}, y{0}, z{0}, yaw{0} {}

  void to_msg(geometry_msgs::msg::Pose &msg) const
  {
    msg.position.x = x;
    msg.position.y = y;
    msg.position.z = z;

    // Yaw to quaternion
    tf2::Quaternion q;
    q.setRPY(0, 0, yaw);
    msg.orientation = tf2::toMsg(q);
  }

  void from_msg(const geometry_msgs::msg::Pose &msg)
  {
    x = msg.position.x;
    y = msg.position.y;
    z = msg.position.z;
    yaw = get_yaw(msg.orientation);
  }

  // Distance between 2 poses on the xy plane
  double distance_xy(const Pose &that) const
  {
    return std::hypot(x - that.x, y - that.y);
  }

  // Z distance between 2 poses
  double distance_z(const Pose &that) const
  {
    return std::abs(z - that.z);
  }

  // Yaw distance between 2 poses
  double distance_yaw(const Pose &that) const
  {
    return std::abs(norm_angle(yaw - that.yaw));
  }
};

//=====================================================================================
// PoseStamped
//=====================================================================================

struct PoseStamped
{
  rclcpp::Time t;
  int64_t nanoseconds; // TODO debug
  Pose pose;

  void to_msg(geometry_msgs::msg::PoseStamped &msg) const
  {
    msg.header.stamp = t;
    // msg.header.frame_id TODO for round trip
    pose.to_msg(msg.pose);
  }

  void from_msg(const geometry_msgs::msg::PoseStamped &msg)
  {
    t = msg.header.stamp;
    nanoseconds = t.nanoseconds();
    pose.from_msg(msg.pose);
  }

  void from_msg(nav_msgs::msg::Odometry &msg)
  {
    t = msg.header.stamp;
    pose.from_msg(msg.pose.pose);
  }

  void add_to_path(nav_msgs::msg::Path &path)
  {
    geometry_msgs::msg::PoseStamped msg;
    to_msg(msg);
    msg.header.frame_id = path.header.frame_id;
    path.poses.push_back(msg);
  }
};

//=====================================================================================
// Twist
//=====================================================================================

struct Twist
{
  double x;
  double y;
  double z;
  double yaw;

  constexpr Twist(): x{0}, y{0}, z{0}, yaw{0} {}

  void from_msg(const geometry_msgs::msg::Twist &msg)
  {
    x = msg.linear.x;
    y = msg.linear.y;
    z = msg.linear.z;
    yaw = msg.angular.z;
  }
};

//=====================================================================================
// Acceleration
//=====================================================================================

struct Acceleration
{
  double x;
  double y;
  double z;
  double yaw;

  constexpr Acceleration(): x{0}, y{0}, z{0}, yaw{0} {}
};

//=====================================================================================
// Thruster efforts from joystick or pid controllers, in the body frame
// Ranges from 1.0 for forward to -1.0 for reverse
//=====================================================================================

class Efforts
{
  double forward_;
  double strafe_;
  double vertical_;
  double yaw_;

public:

  Efforts(): forward_{0}, strafe_{0}, vertical_{0}, yaw_{0} {}

  double forward() { return forward_; }

  double strafe() { return strafe_; }

  double vertical() { return vertical_; }

  double yaw() { return yaw_; }

  void set_forward(double forward) { forward_ = clamp(forward, -1.0, 1.0); }

  void set_strafe(double strafe) { strafe_ = clamp(strafe, -1.0, 1.0); }

  void set_vertical(double vertical) { vertical_ = clamp(vertical, -1.0, 1.0); }

  void set_yaw(double yaw) { yaw_ = clamp(yaw, -1.0, 1.0); }

  void all_stop()
  {
    forward_ = 0;
    strafe_ = 0;
    vertical_ = 0;
    yaw_ = 0;
  }

  void from_acceleration(const Acceleration &u_bar, const double current_yaw)
  {
    // Convert from world frame to body frame
    double x_effort = accel_to_effort_xy(u_bar.x);
    double y_effort = accel_to_effort_xy(u_bar.y);
    double forward, strafe;
    rotate_frame(x_effort, y_effort, current_yaw, forward, strafe);

    set_forward(forward);
    set_strafe(strafe);
    set_vertical(accel_to_effort_z(u_bar.z));
    set_yaw(accel_to_effort_yaw(u_bar.yaw));
  }
};

} // namespace orca_base

#endif //ORCA_BASE_GEOMETRY_HPP