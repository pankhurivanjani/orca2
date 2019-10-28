#ifndef ORCA_BASE_MISSION_HPP
#define ORCA_BASE_MISSION_HPP

#include "fiducial_vlam_msgs/msg/map.hpp"

#include "orca_base/planner.hpp"
#include "orca_base/segment.hpp"

namespace orca_base
{

  class Mission
  {
    rclcpp::Logger logger_;                               // ROS logger
    std::shared_ptr<BasePlanner> planner_;                // Path planner
    int segment_idx_;                                     // Current segment

  public:

    Mission(rclcpp::Logger logger, std::shared_ptr<BasePlanner> planner, const BaseContext &cxt,
            const fiducial_vlam_msgs::msg::Map &map, const PoseStamped &start);

    // Advance the plan, return true to continue
    bool advance(double dt, Pose &plan, Acceleration &ff);
  };

} // namespace orca_base

#endif //ORCA_BASE_MISSION_HPP
