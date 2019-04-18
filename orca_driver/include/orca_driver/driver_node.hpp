#ifndef ORCA_DRIVER_H
#define ORCA_DRIVER_H

#include <string>
#include <vector>

#include "rclcpp/rclcpp.hpp"

#include "orca_driver/maestro.hpp"
#include "orca_msgs/msg/battery.hpp"
#include "orca_msgs/msg/control.hpp"
#include "orca_msgs/msg/leak.hpp"

namespace orca_driver {

struct Thruster
{
  int channel_;
  bool reverse_;
};

// DriverNode provides the interface between the Orca hardware and ROS.

class DriverNode: public rclcpp::Node
{
private:

  // Parameters
  int num_thrusters_;
  std::vector<Thruster> thrusters_;
  int lights_channel_;
  int tilt_channel_;
  int voltage_channel_;
  int leak_channel_;
  std::string maestro_port_;
  double voltage_multiplier_;
  double voltage_min_;

  // State
  maestro::Maestro maestro_;
  orca_msgs::msg::Battery battery_msg_;
  orca_msgs::msg::Leak leak_msg_;
  
  // Subscriptions
  rclcpp::Subscription<orca_msgs::msg::Control>::SharedPtr control_sub_;
  
  // Callbacks
  void controlCallback(const orca_msgs::msg::Control::SharedPtr msg);
  
  // Publications
  rclcpp::Publisher<orca_msgs::msg::Battery>::SharedPtr battery_pub_;
  rclcpp::Publisher<orca_msgs::msg::Leak>::SharedPtr leak_pub_;
  
  bool readBattery();
  bool readLeak();
  bool preDive();

public:
  explicit DriverNode();
  ~DriverNode() {}; // Suppress default copy and move constructors

  bool connect();
  void spinOnce();
  void disconnect();
};

} // namespace orca_driver

#endif // ORCA_DRIVER_H