#include "turtle_arena/world_loader.hpp"

#include <stdexcept>

#include <yaml-cpp/yaml.h>

namespace turtle_arena
{

WorldSpec LoadWorld(const std::string & path)
{
  WorldSpec spec;
  if (path.empty()) {
    return spec;
  }

  const YAML::Node root = YAML::LoadFile(path);

  if (root["name"]) {
    spec.name = root["name"].as<std::string>();
  }
  if (root["dimensions"]) {
    spec.dimensions.width_m = root["dimensions"]["width"].as<float>(spec.dimensions.width_m);
    spec.dimensions.height_m = root["dimensions"]["height"].as<float>(spec.dimensions.height_m);
  }
  if (root["walls"]) {
    for (const auto & w : root["walls"]) {
      spec.walls.push_back(
        WallSpec{
          w["cx"].as<float>(), w["cy"].as<float>(),
          w["hx"].as<float>(), w["hy"].as<float>()});
    }
  }
  if (root["start_zone"]) {
    const auto & z = root["start_zone"];
    spec.start_zone = ZoneSpec{
      z["x"].as<float>(spec.start_zone.x), z["y"].as<float>(spec.start_zone.y),
      z["radius"].as<float>(spec.start_zone.radius)};
  }
  if (root["goal_zone"]) {
    const auto & z = root["goal_zone"];
    spec.goal_zone = ZoneSpec{
      z["x"].as<float>(spec.goal_zone.x), z["y"].as<float>(spec.goal_zone.y),
      z["radius"].as<float>(spec.goal_zone.radius)};
  }

  return spec;
}

}  // namespace turtle_arena
