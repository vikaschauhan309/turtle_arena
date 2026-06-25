#pragma once

#include <map>
#include <memory>
#include <string>

#include <rclcpp/rclcpp.hpp>
#include <std_srvs/srv/trigger.hpp>
#include <turtlesim/srv/kill.hpp>
#include <turtlesim/srv/spawn.hpp>

#include "turtle_arena/arena_world.hpp"
#include "turtle_arena/challenge_manager.hpp"
#include "turtle_arena/robot_entity.hpp"
#include "turtle_arena/world_loader.hpp"

namespace turtle_arena
{

// Top-level ROS2 node: owns the physics world, the loaded world geometry,
// every spawned robot, and the challenge manager. Exposes turtlesim's
// /spawn and /kill services so existing turtlesim tooling (teleop, spawn
// scripts) works against this sim unmodified, plus /world/reset for
// challenge worlds. Spawns "turtle1" at the world's start zone on startup.
class ArenaNode : public rclcpp::Node
{
public:
  ArenaNode();

  // Advances physics by dt and publishes every robot's new state. Called
  // once per render frame from main()'s loop.
  void Tick(float dt);

  // Keyboard fallback/debug control -- bypasses /cmd_vel for the named
  // robot. No-op if the robot doesn't exist (e.g. it was killed).
  void DriveDirect(const std::string & name, float linear_x, float angular_z);

  ArenaWorld & arena_world() {return arena_world_;}
  const WorldSpec & world_spec() const {return world_spec_;}
  const std::map<std::string, std::unique_ptr<RobotEntity>> & robots() const {return robots_;}

  // True if launched with -p headless:=true -- skips the SFML window
  // entirely so the node can run under launch_testing with no display.
  bool headless() const {return headless_;}

private:
  void SpawnRobot(const std::string & name, float x, float y, float theta);

  void OnSpawn(
    const std::shared_ptr<turtlesim::srv::Spawn::Request> request,
    std::shared_ptr<turtlesim::srv::Spawn::Response> response);
  void OnKill(
    const std::shared_ptr<turtlesim::srv::Kill::Request> request,
    std::shared_ptr<turtlesim::srv::Kill::Response> response);
  void OnWorldReset(
    const std::shared_ptr<std_srvs::srv::Trigger::Request> request,
    std::shared_ptr<std_srvs::srv::Trigger::Response> response);

  bool headless_ = false;
  WorldSpec world_spec_;
  ArenaWorld arena_world_;
  std::unique_ptr<ChallengeManager> challenge_manager_;
  std::map<std::string, std::unique_ptr<RobotEntity>> robots_;
  int next_auto_name_id_ = 2;  // "turtle1" is the default spawn, like stock turtlesim

  rclcpp::Service<turtlesim::srv::Spawn>::SharedPtr spawn_service_;
  rclcpp::Service<turtlesim::srv::Kill>::SharedPtr kill_service_;
  rclcpp::Service<std_srvs::srv::Trigger>::SharedPtr world_reset_service_;
};

}  // namespace turtle_arena
