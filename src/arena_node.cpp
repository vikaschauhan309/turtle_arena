#include "turtle_arena/arena_node.hpp"

namespace turtle_arena
{

namespace
{
WorldSpec LoadConfiguredWorld(rclcpp::Node & node)
{
  node.declare_parameter<std::string>("world_file", "");
  return LoadWorld(node.get_parameter("world_file").as_string());
}
}  // namespace

ArenaNode::ArenaNode()
: rclcpp::Node("turtle_arena_node"),
  world_spec_(LoadConfiguredWorld(*this)),
  arena_world_(world_spec_.dimensions)
{
  headless_ = declare_parameter<bool>("headless", false);
  arena_world_.AddObstacleWalls(world_spec_.walls);
  challenge_manager_ = std::make_unique<ChallengeManager>(
    *this, world_spec_.start_zone, world_spec_.goal_zone);

  spawn_service_ = create_service<turtlesim::srv::Spawn>(
    "spawn",
    [this](
      const std::shared_ptr<turtlesim::srv::Spawn::Request> request,
      std::shared_ptr<turtlesim::srv::Spawn::Response> response) {
      OnSpawn(request, response);
    });
  kill_service_ = create_service<turtlesim::srv::Kill>(
    "kill",
    [this](
      const std::shared_ptr<turtlesim::srv::Kill::Request> request,
      std::shared_ptr<turtlesim::srv::Kill::Response> response) {
      OnKill(request, response);
    });
  world_reset_service_ = create_service<std_srvs::srv::Trigger>(
    "world/reset",
    [this](
      const std::shared_ptr<std_srvs::srv::Trigger::Request> request,
      std::shared_ptr<std_srvs::srv::Trigger::Response> response) {
      OnWorldReset(request, response);
    });

  SpawnRobot("turtle1", world_spec_.start_zone.x, world_spec_.start_zone.y, 0.0f);

  RCLCPP_INFO(
    get_logger(), "loaded world '%s' (%.1fm x %.1fm, %zu obstacle wall(s))",
    world_spec_.name.c_str(), world_spec_.dimensions.width_m,
    world_spec_.dimensions.height_m, world_spec_.walls.size());
}

void ArenaNode::SpawnRobot(const std::string & name, float x, float y, float theta)
{
  robots_.emplace(name, std::make_unique<RobotEntity>(*this, arena_world_, name, x, y, theta));
}

void ArenaNode::OnSpawn(
  const std::shared_ptr<turtlesim::srv::Spawn::Request> request,
  std::shared_ptr<turtlesim::srv::Spawn::Response> response)
{
  std::string name = request->name;
  if (name.empty()) {
    name = "turtle" + std::to_string(next_auto_name_id_++);
  }
  if (robots_.count(name) > 0) {
    RCLCPP_ERROR(get_logger(), "spawn: a robot named '%s' already exists", name.c_str());
    response->name = "";
    return;
  }

  SpawnRobot(name, request->x, request->y, request->theta);
  response->name = name;
}

void ArenaNode::OnKill(
  const std::shared_ptr<turtlesim::srv::Kill::Request> request,
  std::shared_ptr<turtlesim::srv::Kill::Response>)
{
  const auto it = robots_.find(request->name);
  if (it == robots_.end()) {
    RCLCPP_ERROR(get_logger(), "kill: no robot named '%s'", request->name.c_str());
    return;
  }
  challenge_manager_->Remove(request->name);
  robots_.erase(it);
}

void ArenaNode::OnWorldReset(
  const std::shared_ptr<std_srvs::srv::Trigger::Request>,
  std::shared_ptr<std_srvs::srv::Trigger::Response> response)
{
  const rclcpp::Time now = get_clock()->now();
  for (auto & [name, robot] : robots_) {
    robot->TeleportTo(world_spec_.start_zone.x, world_spec_.start_zone.y, 0.0f);
    arena_world_.ResetCollisionCount(robot->body());
    challenge_manager_->Reset(name, now);
  }
  response->success = true;
  response->message = "reset " + std::to_string(robots_.size()) + " robot(s) to start zone";
}

void ArenaNode::DriveDirect(const std::string & name, float linear_x, float angular_z)
{
  const auto it = robots_.find(name);
  if (it != robots_.end()) {
    it->second->DriveDirect(linear_x, angular_z);
  }
}

void ArenaNode::Tick(float dt)
{
  arena_world_.Step(dt);

  const rclcpp::Time now = get_clock()->now();
  for (auto & [name, robot] : robots_) {
    robot->PublishState(now);
    challenge_manager_->Update(
      name, robot->x(), robot->y(), arena_world_.CollisionCountFor(robot->body()), now);
  }
}

}  // namespace turtle_arena
