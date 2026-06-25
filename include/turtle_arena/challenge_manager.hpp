#pragma once

#include <map>
#include <string>

#include <rclcpp/rclcpp.hpp>
#include <std_msgs/msg/string.hpp>

#include "turtle_arena/world_loader.hpp"

namespace turtle_arena
{

// Tracks, per robot, whether it has reached the goal zone, how long it's
// taken, and how many collisions it's racked up since spawn/last reset --
// and publishes that as <name>/challenge_status.
class ChallengeManager
{
public:
  ChallengeManager(rclcpp::Node & node, ZoneSpec start_zone, ZoneSpec goal_zone);

  void Update(
    const std::string & name, float x, float y, int collision_count,
    const rclcpp::Time & now);

  void Reset(const std::string & name, const rclcpp::Time & now);
  void Remove(const std::string & name);

  const ZoneSpec & start_zone() const {return start_zone_;}
  const ZoneSpec & goal_zone() const {return goal_zone_;}

private:
  struct State
  {
    rclcpp::Time start_time;
    bool succeeded = false;
  };

  rclcpp::Publisher<std_msgs::msg::String>::SharedPtr StatusPublisherFor(const std::string & name);

  rclcpp::Node & node_;
  ZoneSpec start_zone_;
  ZoneSpec goal_zone_;
  std::map<std::string, State> states_;
  std::map<std::string, rclcpp::Publisher<std_msgs::msg::String>::SharedPtr> status_pubs_;
};

}  // namespace turtle_arena
