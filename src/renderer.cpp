#include "turtle_arena/renderer.hpp"

#include <cmath>

namespace turtle_arena
{

Renderer::Renderer(sf::RenderWindow & window, float pixels_per_meter)
: window_(window), ppm_(pixels_per_meter)
{
}

sf::Vector2f Renderer::ToScreen(float x_m, float y_m) const
{
  const float screen_height = static_cast<float>(window_.getSize().y);
  return sf::Vector2f(x_m * ppm_, screen_height - y_m * ppm_);
}

void Renderer::DrawArena(const ArenaWorld & arena_world)
{
  const auto & dims = arena_world.dimensions();

  sf::RectangleShape border(sf::Vector2f(dims.width_m * ppm_, dims.height_m * ppm_));
  border.setPosition(ToScreen(0.0f, dims.height_m));
  border.setFillColor(sf::Color::Transparent);
  border.setOutlineColor(sf::Color(60, 60, 60));
  border.setOutlineThickness(-4.0f);
  window_.draw(border);

  for (const auto & wall : arena_world.obstacle_walls()) {
    sf::RectangleShape rect(sf::Vector2f(wall.hx * 2.0f * ppm_, wall.hy * 2.0f * ppm_));
    rect.setOrigin(wall.hx * ppm_, wall.hy * ppm_);
    rect.setPosition(ToScreen(wall.cx, wall.cy));
    rect.setFillColor(sf::Color(80, 80, 80));
    window_.draw(rect);
  }
}

void Renderer::DrawZone(float x_m, float y_m, float radius_m, sf::Color color)
{
  const float radius_px = radius_m * ppm_;
  sf::CircleShape zone(radius_px);
  zone.setOrigin(radius_px, radius_px);
  zone.setPosition(ToScreen(x_m, y_m));
  zone.setFillColor(sf::Color(color.r, color.g, color.b, 60));
  zone.setOutlineColor(color);
  zone.setOutlineThickness(2.0f);
  window_.draw(zone);
}

void Renderer::DrawRobot(float x_m, float y_m, float theta)
{
  const float radius_px = ArenaWorld::kRobotRadiusM * ppm_;
  sf::CircleShape robot(radius_px);
  robot.setOrigin(radius_px, radius_px);
  robot.setPosition(ToScreen(x_m, y_m));
  robot.setFillColor(sf::Color(46, 204, 113));
  robot.setOutlineColor(sf::Color(30, 132, 73));
  robot.setOutlineThickness(2.0f);
  window_.draw(robot);

  // Heading indicator: a short line from the robot's center pointing along
  // its current angle, so direction is visible even when stationary.
  const float tip_x = x_m + ArenaWorld::kRobotRadiusM * 1.6f * std::cos(theta);
  const float tip_y = y_m + ArenaWorld::kRobotRadiusM * 1.6f * std::sin(theta);

  sf::Vertex heading_line[] = {
    sf::Vertex(ToScreen(x_m, y_m), sf::Color::Black),
    sf::Vertex(ToScreen(tip_x, tip_y), sf::Color::Black),
  };
  window_.draw(heading_line, 2, sf::Lines);
}

}  // namespace turtle_arena
