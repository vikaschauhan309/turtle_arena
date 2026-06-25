#include "turtle_arena/robot_entity.hpp"

#include <cmath>

namespace turtle_arena
{

RobotEntity::RobotEntity(
  rclcpp::Node & node, ArenaWorld & arena_world, std::string name,
  float x, float y, float theta)
: node_(node), arena_world_(arena_world), name_(std::move(name))
{
  body_ = arena_world_.SpawnRobotBody(x, y, theta);

  cmd_vel_sub_ = node_.create_subscription<geometry_msgs::msg::Twist>(
    name_ + "/cmd_vel", 10,
    [this](const geometry_msgs::msg::Twist::SharedPtr msg) {OnCmdVel(msg);});
  odom_pub_ = node_.create_publisher<nav_msgs::msg::Odometry>(name_ + "/odom", 10);
  pose_pub_ = node_.create_publisher<turtlesim::msg::Pose>(name_ + "/pose", 10);
  scan_pub_ = node_.create_publisher<sensor_msgs::msg::LaserScan>(name_ + "/scan", 10);
}

RobotEntity::~RobotEntity()
{
  arena_world_.DestroyRobotBody(body_);
}

void RobotEntity::OnCmdVel(const geometry_msgs::msg::Twist::SharedPtr msg)
{
  DriveDirect(static_cast<float>(msg->linear.x), static_cast<float>(msg->angular.z));
}

void RobotEntity::DriveDirect(float linear_x, float angular_z)
{
  const float angle = body_->GetAngle();
  body_->SetLinearVelocity(b2Vec2(linear_x * std::cos(angle), linear_x * std::sin(angle)));
  body_->SetAngularVelocity(angular_z);
}

void RobotEntity::TeleportTo(float x, float y, float theta)
{
  body_->SetTransform(b2Vec2(x, y), theta);
  body_->SetLinearVelocity(b2Vec2(0.0f, 0.0f));
  body_->SetAngularVelocity(0.0f);
}

void RobotEntity::PublishState(const rclcpp::Time & stamp)
{
  const b2Vec2 position = body_->GetPosition();
  const float angle = body_->GetAngle();
  const b2Vec2 velocity = body_->GetLinearVelocity();
  const float forward_speed = velocity.x * std::cos(angle) + velocity.y * std::sin(angle);
  const float angular_speed = body_->GetAngularVelocity();

  nav_msgs::msg::Odometry odom;
  odom.header.stamp = stamp;
  odom.header.frame_id = "arena";
  odom.child_frame_id = name_;
  odom.pose.pose.position.x = position.x;
  odom.pose.pose.position.y = position.y;
  odom.pose.pose.orientation.z = std::sin(angle / 2.0f);
  odom.pose.pose.orientation.w = std::cos(angle / 2.0f);
  odom.twist.twist.linear.x = forward_speed;
  odom.twist.twist.angular.z = angular_speed;
  odom_pub_->publish(odom);

  turtlesim::msg::Pose pose;
  pose.x = position.x;
  pose.y = position.y;
  pose.theta = angle;
  pose.linear_velocity = forward_speed;
  pose.angular_velocity = angular_speed;
  pose_pub_->publish(pose);

  scan_pub_->publish(
    lidar_.Scan(arena_world_.world(), body_, position.x, position.y, angle, stamp, name_));
}

}  // namespace turtle_arena
