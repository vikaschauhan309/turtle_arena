#pragma once

#include <string>

#include <box2d/box2d.h>
#include <rclcpp/time.hpp>
#include <sensor_msgs/msg/laser_scan.hpp>

namespace turtle_arena
{

struct LidarConfig
{
  int num_rays = 360;
  float range_min = 0.05f;
  float range_max = 13.0f;  // > arena diagonal (10x7.5m) so walls are always reachable
};

// Casts a full-circle ring of Box2D rays from a robot's pose against the
// arena's geometry and packages the result as a standard LaserScan, so
// challenges can be solved using only /scan the way a real lidar-equipped
// robot would.
class LidarSensor
{
public:
  explicit LidarSensor(LidarConfig config = LidarConfig{});

  sensor_msgs::msg::LaserScan Scan(
    const b2World & world, const b2Body * self_body,
    float x, float y, float theta,
    const rclcpp::Time & stamp, const std::string & frame_id) const;

private:
  LidarConfig config_;
};

}  // namespace turtle_arena
