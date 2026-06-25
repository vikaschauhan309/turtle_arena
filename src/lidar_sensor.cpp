#include "turtle_arena/lidar_sensor.hpp"

#include <cmath>
#include <limits>

namespace turtle_arena
{

namespace
{

// Finds the closest fixture hit, ignoring the casting robot's own body so
// it doesn't detect itself.
class ClosestRayCastCallback : public b2RayCastCallback
{
public:
  explicit ClosestRayCastCallback(const b2Body * ignore_body)
  : ignore_body_(ignore_body)
  {
  }

  float ReportFixture(
    b2Fixture * fixture, const b2Vec2 &, const b2Vec2 &, float fraction) override
  {
    if (fixture->GetBody() == ignore_body_) {
      return -1.0f;  // keep casting past this fixture
    }
    hit = true;
    hit_fraction = fraction;
    return fraction;  // clip the ray to the closest hit so far
  }

  bool hit = false;
  float hit_fraction = 1.0f;

private:
  const b2Body * ignore_body_;
};

}  // namespace

LidarSensor::LidarSensor(LidarConfig config)
: config_(config)
{
}

sensor_msgs::msg::LaserScan LidarSensor::Scan(
  const b2World & world, const b2Body * self_body,
  float x, float y, float theta,
  const rclcpp::Time & stamp, const std::string & frame_id) const
{
  sensor_msgs::msg::LaserScan scan;
  scan.header.stamp = stamp;
  scan.header.frame_id = frame_id;
  scan.angle_min = -M_PI;
  scan.angle_max = M_PI;
  scan.angle_increment = static_cast<float>(2.0 * M_PI / config_.num_rays);
  scan.time_increment = 0.0f;
  scan.scan_time = 0.0f;
  scan.range_min = config_.range_min;
  scan.range_max = config_.range_max;
  scan.ranges.resize(config_.num_rays);

  const b2Vec2 origin(x, y);
  for (int i = 0; i < config_.num_rays; ++i) {
    const float bearing = scan.angle_min + i * scan.angle_increment;
    const float world_angle = theta + bearing;
    const b2Vec2 end(
      x + config_.range_max * std::cos(world_angle),
      y + config_.range_max * std::sin(world_angle));

    ClosestRayCastCallback callback(self_body);
    world.RayCast(&callback, origin, end);

    scan.ranges[i] = callback.hit ?
      callback.hit_fraction * config_.range_max :
      std::numeric_limits<float>::infinity();
  }

  return scan;
}

}  // namespace turtle_arena
