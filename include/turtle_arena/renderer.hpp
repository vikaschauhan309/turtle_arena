#pragma once

#include <SFML/Graphics.hpp>

#include "turtle_arena/arena_world.hpp"

namespace turtle_arena
{

// Draws an ArenaWorld and its robots into an SFML window. Box2D uses
// meters with a y-up frame; SFML uses pixels with a y-down frame, so all
// drawing goes through ToScreen() to keep that conversion in one place.
class Renderer
{
public:
  Renderer(sf::RenderWindow & window, float pixels_per_meter);

  void DrawArena(const ArenaWorld & arena_world);
  void DrawRobot(float x_m, float y_m, float theta);
  void DrawZone(float x_m, float y_m, float radius_m, sf::Color color);

private:
  sf::Vector2f ToScreen(float x_m, float y_m) const;

  sf::RenderWindow & window_;
  float ppm_;
};

}  // namespace turtle_arena
