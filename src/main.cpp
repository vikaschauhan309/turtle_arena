#include <SFML/Graphics.hpp>
#include <rclcpp/rclcpp.hpp>

#include "turtle_arena/arena_node.hpp"
#include "turtle_arena/renderer.hpp"

namespace
{
constexpr float kPixelsPerMeter = 80.0f;
constexpr float kMaxLinearSpeed = 2.0f;    // m/s, matches turtlesim's feel
constexpr float kMaxAngularSpeed = 2.5f;   // rad/s
const char kKeyboardControlledRobot[] = "turtle1";

// Keyboard fallback/debug control -- /cmd_vel is the primary input path,
// this just lets you drive turtle1 directly without publishing a topic.
bool ReadKeyboardCommand(float & linear_x, float & angular_z)
{
  linear_x = 0.0f;
  angular_z = 0.0f;
  bool any_key = false;
  if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up) ||
    sf::Keyboard::isKeyPressed(sf::Keyboard::W))
  {
    linear_x += kMaxLinearSpeed;
    any_key = true;
  }
  if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down) ||
    sf::Keyboard::isKeyPressed(sf::Keyboard::S))
  {
    linear_x -= kMaxLinearSpeed;
    any_key = true;
  }
  if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left) ||
    sf::Keyboard::isKeyPressed(sf::Keyboard::A))
  {
    angular_z += kMaxAngularSpeed;
    any_key = true;
  }
  if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right) ||
    sf::Keyboard::isKeyPressed(sf::Keyboard::D))
  {
    angular_z -= kMaxAngularSpeed;
    any_key = true;
  }
  return any_key;
}

// No window, no keyboard -- just physics ticking at a fixed rate driven by
// a wall timer, so this path works under launch_testing with no display.
void RunHeadless(const std::shared_ptr<turtle_arena::ArenaNode> & arena_node)
{
  auto timer = arena_node->create_wall_timer(
    std::chrono::milliseconds(16),
    [arena_node]() {arena_node->Tick(1.0f / 60.0f);});
  rclcpp::spin(arena_node);
}

void RunWithWindow(const std::shared_ptr<turtle_arena::ArenaNode> & arena_node)
{
  const auto & dims = arena_node->arena_world().dimensions();
  sf::RenderWindow window(
    sf::VideoMode(
      static_cast<unsigned int>(dims.width_m * kPixelsPerMeter),
      static_cast<unsigned int>(dims.height_m * kPixelsPerMeter)),
    "turtle_arena");
  window.setFramerateLimit(60);

  turtle_arena::Renderer renderer(window, kPixelsPerMeter);

  sf::Clock clock;
  while (window.isOpen() && rclcpp::ok()) {
    sf::Event event;
    while (window.pollEvent(event)) {
      if (event.type == sf::Event::Closed) {
        window.close();
      }
    }

    float linear_x = 0.0f;
    float angular_z = 0.0f;
    if (ReadKeyboardCommand(linear_x, angular_z)) {
      arena_node->DriveDirect(kKeyboardControlledRobot, linear_x, angular_z);
    }

    const float dt = clock.restart().asSeconds();
    arena_node->Tick(dt > 0.0f ? dt : 1.0f / 60.0f);

    rclcpp::spin_some(arena_node);

    window.clear(sf::Color(245, 245, 245));
    renderer.DrawArena(arena_node->arena_world());
    const auto & spec = arena_node->world_spec();
    renderer.DrawZone(
      spec.start_zone.x, spec.start_zone.y, spec.start_zone.radius, sf::Color(52, 152, 219));
    renderer.DrawZone(
      spec.goal_zone.x, spec.goal_zone.y, spec.goal_zone.radius, sf::Color(241, 196, 15));
    for (const auto & [name, robot] : arena_node->robots()) {
      renderer.DrawRobot(robot->x(), robot->y(), robot->theta());
    }
    window.display();
  }
}
}  // namespace

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);
  auto arena_node = std::make_shared<turtle_arena::ArenaNode>();

  if (arena_node->headless()) {
    RunHeadless(arena_node);
  } else {
    RunWithWindow(arena_node);
  }

  rclcpp::shutdown();
  return 0;
}
