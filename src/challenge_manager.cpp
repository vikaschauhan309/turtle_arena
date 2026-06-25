#include "turtle_arena/challenge_manager.hpp"

#include <cmath>

namespace turtle_arena
{

ChallengeManager::ChallengeManager(rclcpp::Node & node, ZoneSpec start_zone, ZoneSpec goal_zone)
: node_(node), start_zone_(start_zone), goal_zone_(goal_zone)
{
}

rclcpp::Publisher<std_msgs::msg::String>::SharedPtr ChallengeManager::StatusPublisherFor(
  const std::string & name)
{
  const auto it = status_pubs_.find(name);
  if (it != status_pubs_.end()) {
    return it->second;
  }
  auto pub = node_.create_publisher<std_msgs::msg::String>(name + "/challenge_status", 10);
  status_pubs_[name] = pub;
  return pub;
}

void ChallengeManager::Update(
  const std::string & name, float x, float y, int collision_count, const rclcpp::Time & now)
{
  auto & state = states_[name];
  if (state.start_time.nanoseconds() == 0) {
    state.start_time = now;
  }

  const float dx = x - goal_zone_.x;
  const float dy = y - goal_zone_.y;
  if (std::sqrt(dx * dx + dy * dy) <= goal_zone_.radius) {
    state.succeeded = true;
  }

  const double elapsed = (now - state.start_time).seconds();

  std_msgs::msg::String status;
  const std::string status_word = state.succeeded ? "status=success" : "status=in_progress";
  status.data = status_word + " elapsed=" + std::to_string(elapsed) +
    " collisions=" + std::to_string(collision_count);
  StatusPublisherFor(name)->publish(status);
}

void ChallengeManager::Reset(const std::string & name, const rclcpp::Time & now)
{
  states_[name] = State{now, false};
}

void ChallengeManager::Remove(const std::string & name)
{
  states_.erase(name);
  status_pubs_.erase(name);
}

}  // namespace turtle_arena
