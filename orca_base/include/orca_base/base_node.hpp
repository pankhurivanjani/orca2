#ifndef ORCA_BASE_BASE_NODE_HPP
#define ORCA_BASE_BASE_NODE_HPP

#include "rclcpp_action/rclcpp_action.hpp"

#include "fiducial_vlam_msgs/msg/map.hpp"
#include "sensor_msgs/msg/joy.hpp"
#include "visualization_msgs/msg/marker_array.hpp"

#include "orca_msgs/action/mission.hpp"
#include "orca_msgs/msg/barometer.hpp"
#include "orca_msgs/msg/battery.hpp"
#include "orca_msgs/msg/control.hpp"
#include "orca_msgs/msg/leak.hpp"

#include "orca_shared/monotonic.hpp"

#include "orca_base/base_context.hpp"
#include "orca_base/map.hpp"
#include "orca_base/mission.hpp"
#include "orca_base/joystick.hpp"

using namespace std::chrono_literals;

namespace orca_base
{

  //=============================================================================
  // Utils
  //=============================================================================

  constexpr bool is_disarmed_mode(uint8_t mode)
  {
    using orca_msgs::msg::Control;
    return mode == Control::DISARMED;
  }

  constexpr bool is_hold_pressure_mode(uint8_t mode)
  {
    using orca_msgs::msg::Control;
    return mode == Control::ROV_HOLD_PRESSURE;
  }

  constexpr bool is_rov_mode(uint8_t mode)
  {
    using orca_msgs::msg::Control;
    return mode == Control::ROV || mode == Control::ROV_HOLD_PRESSURE;
  }

  constexpr bool is_auv_mode(uint8_t mode)
  {
    using orca_msgs::msg::Control;
    return mode >= Control::AUV_KEEP_STATION;
  }

  //=============================================================================
  // Thrusters
  //=============================================================================

  struct Thruster
  {
    std::string frame_id;   // URDF link frame id
    bool ccw;               // True if counterclockwise
    double forward_factor;
    double strafe_factor;
    double yaw_factor;
    double vertical_factor;
  };

  //=============================================================================
  // BaseNode
  //=============================================================================

  class BaseNode : public rclcpp::Node
  {
  private:
    // Constants
    const rclcpp::Duration JOY_TIMEOUT{RCL_S_TO_NS(1)};   // ROV: disarm if we lose communication
    const rclcpp::Duration ODOM_TIMEOUT{RCL_S_TO_NS(1)};  // AUV: disarm if we lose odometry
    const rclcpp::Duration BARO_TIMEOUT{RCL_S_TO_NS(1)};  // Holding z: disarm if we lose barometer
    const std::chrono::milliseconds SPIN_PERIOD{100ms};   // Check timeouts at 10Hz

    // Thrusters, order must match the order of the <thruster> tags in the URDF
    const std::vector<Thruster> THRUSTERS = {
      {"t200_link_front_right",    false, 1.0, 1.0,  1.0,  0.0},
      {"t200_link_front_left",     false, 1.0, -1.0, -1.0, 0.0},
      {"t200_link_rear_right",     true,  1.0, -1.0, 1.0,  0.0},
      {"t200_link_rear_left",      true,  1.0, 1.0,  -1.0, 0.0},
      {"t200_link_vertical_right", false, 0.0, 0.0,  0.0,  1.0},
      {"t200_link_vertical_left",  true,  0.0, 0.0,  0.0,  -1.0},
    };

    // Joystick assignments
    const int joy_axis_yaw_ = JOY_AXIS_LEFT_LR;
    const int joy_axis_forward_ = JOY_AXIS_LEFT_FB;
    const int joy_axis_strafe_ = JOY_AXIS_RIGHT_LR;
    const int joy_axis_vertical_ = JOY_AXIS_RIGHT_FB;
    const int joy_axis_z_trim_ = JOY_AXIS_TRIM_FB;

    const int joy_button_disarm_ = JOY_BUTTON_VIEW;
    const int joy_button_arm_ = JOY_BUTTON_MENU;
    const int joy_button_rov_ = JOY_BUTTON_A;
    const int joy_button_rov_hold_pressure_ = JOY_BUTTON_B;
    const int joy_button_auv_keep_station_ = JOY_BUTTON_X;
    const int joy_button_auv_keep_origin_ = JOY_BUTTON_Y;
    const int joy_button_auv_random_ = JOY_BUTTON_LOGO;

    const int joy_button_tilt_down_ = JOY_BUTTON_LEFT_BUMPER;
    const int joy_button_tilt_up_ = JOY_BUTTON_RIGHT_BUMPER;
    const int joy_button_bright_ = JOY_BUTTON_LEFT_STICK;
    const int joy_button_dim_ = JOY_BUTTON_RIGHT_STICK;

    // Parameters and dynamics model
    BaseContext cxt_;

    // Mode
    uint8_t mode_{orca_msgs::msg::Control::DISARMED};

    // Barometer state
    double pressure_{};

    // Joystick state
    sensor_msgs::msg::Joy joy_msg_;               // Most recent message

    // Odometry state
    nav_msgs::msg::Odometry filtered_odom_;       // Estimated odometry
    orca::PoseStamped filtered_pose_;                   // Estimated pose TODO remove
    double stability_{1.0};                       // Roll and pitch stability, 1.0 (flat) to 0.0 (>90 degree tilt)

    // ROV operation
    std::shared_ptr<pid::Controller> pressure_hold_pid_;

    // AUV operation
    std::shared_ptr<Mission> mission_;            // The mission we're running
    Map map_;                                     // Map of fiducial markers
    nav_msgs::msg::Path filtered_path_;           // Estimate of the actual path (from filtered_pose_)

    // Outputs
    int tilt_{};                                  // Camera tilt
    int brightness_{};                            // Lights

    // Subscriptions
    rclcpp::Subscription<orca_msgs::msg::Barometer>::SharedPtr baro_sub_;
    rclcpp::Subscription<orca_msgs::msg::Battery>::SharedPtr battery_sub_;
    rclcpp::Subscription<geometry_msgs::msg::PoseStamped>::SharedPtr goal_sub_;
    rclcpp::Subscription<sensor_msgs::msg::Joy>::SharedPtr joy_sub_;
    rclcpp::Subscription<orca_msgs::msg::Leak>::SharedPtr leak_sub_;
    rclcpp::Subscription<fiducial_vlam_msgs::msg::Map>::SharedPtr map_sub_;
    rclcpp::Subscription<nav_msgs::msg::Odometry>::SharedPtr odom_sub_;

    // Timer
    rclcpp::TimerBase::SharedPtr spin_timer_;

    // Validate parameters
    void validate_parameters();

    // Subscription callbacks
    void baro_callback(orca_msgs::msg::Barometer::SharedPtr msg, bool first);

    void battery_callback(orca_msgs::msg::Battery::SharedPtr msg);

    void goal_callback(geometry_msgs::msg::PoseStamped::SharedPtr msg);

    void joy_callback(sensor_msgs::msg::Joy::SharedPtr msg, bool first);

    void leak_callback(orca_msgs::msg::Leak::SharedPtr msg);

    void map_callback(fiducial_vlam_msgs::msg::Map::SharedPtr msg);

    void odom_callback(nav_msgs::msg::Odometry::SharedPtr msg, bool first);

    // Callback wrappers
    monotonic::Monotonic<BaseNode *, const orca_msgs::msg::Barometer::SharedPtr> baro_cb_{this, &BaseNode::baro_callback};
    monotonic::Monotonic<BaseNode *, sensor_msgs::msg::Joy::SharedPtr> joy_cb_{this, &BaseNode::joy_callback};
    monotonic::Valid<BaseNode *, fiducial_vlam_msgs::msg::Map::SharedPtr> map_cb_{this, &BaseNode::map_callback};
    monotonic::Monotonic<BaseNode *, nav_msgs::msg::Odometry::SharedPtr> odom_cb_{this, &BaseNode::odom_callback};

    // Publications
    rclcpp::Publisher<orca_msgs::msg::Control>::SharedPtr control_pub_;
    rclcpp::Publisher<visualization_msgs::msg::MarkerArray>::SharedPtr thrust_marker_pub_;
    rclcpp::Publisher<nav_msgs::msg::Path>::SharedPtr planned_path_pub_;
    rclcpp::Publisher<nav_msgs::msg::Path>::SharedPtr filtered_path_pub_;

    // Mission server
    rclcpp_action::Server<orca_msgs::action::Mission>::SharedPtr mission_server_;

    // Mission callbacks
    rclcpp_action::GoalResponse
    mission_goal(const rclcpp_action::GoalUUID &uuid, std::shared_ptr<const orca_msgs::action::Mission::Goal> goal);

    rclcpp_action::CancelResponse
    mission_cancel(std::shared_ptr<rclcpp_action::ServerGoalHandle<orca_msgs::action::Mission>> goal_handle);

    void mission_accepted(std::shared_ptr<rclcpp_action::ServerGoalHandle<orca_msgs::action::Mission>> goal_handle);

    void rov_advance(const rclcpp::Time &stamp);

    void auv_advance(double dt);

    void all_stop(const rclcpp::Time &msg_time);

    void publish_control(const rclcpp::Time &msg_time, const orca::Pose &error, const orca::Efforts &efforts);

    void disarm(const rclcpp::Time &msg_time);

    void set_mode(const rclcpp::Time &msg_time, uint8_t new_mode, const orca::Pose &goal = {},
                  const std::shared_ptr<rclcpp_action::ServerGoalHandle<orca_msgs::action::Mission>> &goal_handle = nullptr);

    bool disarmed()
    { return is_disarmed_mode(mode_); }

    bool holding_pressure()
    { return is_hold_pressure_mode(mode_); }

    bool rov_mode()
    { return is_rov_mode(mode_); }

    bool auv_mode()
    { return is_auv_mode(mode_); }

    bool baro_ok(const rclcpp::Time &t)
    { return baro_cb_.receiving() && t - baro_cb_.prev() < BARO_TIMEOUT; }

    bool joy_ok(const rclcpp::Time &t)
    { return joy_cb_.receiving() && t - joy_cb_.prev() < JOY_TIMEOUT; }

    bool odom_ok(const rclcpp::Time &t)
    { return odom_cb_.receiving() && t - odom_cb_.prev() < ODOM_TIMEOUT; }

  public:
    explicit BaseNode();

    ~BaseNode() override = default;

    void spin_once();
  };

} // namespace orca_base

#endif // ORCA_BASE_BASE_NODE_HPP
