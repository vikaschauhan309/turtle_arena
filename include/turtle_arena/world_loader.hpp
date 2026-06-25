#pragma once

#include <string>
#include <vector>

#include "turtle_arena/arena_world.hpp"

namespace turtle_arena
{

struct ZoneSpec
{
  float x = 1.0f;
  float y = 1.0f;
  float radius = 0.5f;
};

struct WorldSpec
{
  std::string name = "default";
  ArenaDimensions dimensions;
  std::vector<WallSpec> walls;
  ZoneSpec start_zone{1.0f, 1.0f, 0.5f};
  ZoneSpec goal_zone{9.0f, 6.5f, 0.5f};
};

// Loads a world from YAML. An empty path returns a default empty arena
// with no interior walls -- the world_file parameter is optional.
WorldSpec LoadWorld(const std::string & path);

}  // namespace turtle_arena
