#pragma once

#include <memory>
#include <string>

#include <box2d/box2d.h>
#include <geometry_msgs/msg/twist.hpp>
#include <nav_msgs/msg/odometry.hpp>
#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/laser_scan.hpp>
#include <turtlesim/msg/pose.hpp>

#include "turtle_arena/arena_world.hpp"
#include "turtle_arena/lidar_sensor.hpp"

namespace turtle_arena
{

// One spawned robot: a Box2D body plus the ROS2 interfaces turtlesim users
// already expect -- <name>/cmd_vel in, <name>/odom and <name>/pose out.
// A fresh Twist sets the body's velocity immediately and it persists across
// ticks until the next Twist arrives, matching turtlesim's "keep moving
// until told otherwise" behavior.
class RobotEntity
{
public:
  RobotEntity(
    rclcpp::Node & node, ArenaWorld & arena_world, std::string name,
    float x, float y, float theta);
  ~RobotEntity();

  RobotEntity(const RobotEntity &) = delete;
  RobotEntity & operator=(const RobotEntity &) = delete;

  // Directly sets the body's velocity, bypassing /cmd_vel -- used for the
  // keyboard fallback/debug control path.
  void DriveDirect(float linear_x, float angular_z);

  // Snaps the body to a pose and zeroes velocity -- used by /world/reset.
  void TeleportTo(float x, float y, float theta);

  void PublishState(const rclcpp::Time & stamp);

  const std::string & name() const {return name_;}
  b2Body * body() const {return body_;}
  float x() const {return body_->GetPosition().x;}
  float y() const {return body_->GetPosition().y;}
  float theta() const {return body_->GetAngle();}

private:
  void OnCmdVel(const geometry_msgs::msg::Twist::SharedPtr msg);

  rclcpp::Node & node_;
  ArenaWorld & arena_world_;
  std::string name_;
  b2Body * body_;

  LidarSensor lidar_;

  rclcpp::Subscription<geometry_msgs::msg::Twist>::SharedPtr cmd_vel_sub_;
  rclcpp::Publisher<nav_msgs::msg::Odometry>::SharedPtr odom_pub_;
  rclcpp::Publisher<turtlesim::msg::Pose>::SharedPtr pose_pub_;
  rclcpp::Publisher<sensor_msgs::msg::LaserScan>::SharedPtr scan_pub_;
};

}  // namespace turtle_arena
