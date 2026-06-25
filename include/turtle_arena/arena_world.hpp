#pragma once

#include <unordered_map>
#include <vector>

#include <box2d/box2d.h>

namespace turtle_arena
{

struct ArenaDimensions
{
  float width_m = 10.0f;
  float height_m = 7.5f;
};

// An axis-aligned interior obstacle: center (cx, cy) and half-extents
// (hx, hy), all in meters. Used for maze walls loaded from a world file.
struct WallSpec
{
  float cx, cy, hx, hy;
};

namespace detail
{
// Counts collisions per-body so challenges can report "you hit a wall N
// times" without each caller needing its own b2ContactListener.
class CollisionCounter : public b2ContactListener
{
public:
  void BeginContact(b2Contact * contact) override;
  int CountFor(const b2Body * body) const;
  void Reset(const b2Body * body);

private:
  std::unordered_map<const b2Body *, int> counts_;
};
}  // namespace detail

// Owns the Box2D world: a static rectangular boundary plus any number of
// dynamic robot bodies spawned at runtime (one per RobotEntity), and any
// interior obstacle walls loaded from a world file. Each robot's velocity
// is set directly on its body whenever a new command arrives (keyboard or
// /cmd_vel) -- Box2D's contact solver still resolves collisions on the
// next Step() regardless of when velocity was last set, so robots don't
// tunnel through walls or each other.
class ArenaWorld
{
public:
  static constexpr float kRobotRadiusM = 0.18f;

  explicit ArenaWorld(ArenaDimensions dims = ArenaDimensions{});

  void Step(float dt);

  b2Body * SpawnRobotBody(float x, float y, float theta);
  void DestroyRobotBody(b2Body * body);

  void AddObstacleWalls(const std::vector<WallSpec> & walls);

  int CollisionCountFor(const b2Body * body) const {return collision_counter_.CountFor(body);}
  void ResetCollisionCount(const b2Body * body) {collision_counter_.Reset(body);}

  const ArenaDimensions & dimensions() const {return dims_;}
  const b2World & world() const {return world_;}
  const std::vector<WallSpec> & obstacle_walls() const {return obstacle_walls_;}

private:
  void CreateBoundaryWalls();

  ArenaDimensions dims_;
  b2World world_;
  detail::CollisionCounter collision_counter_;
  std::vector<WallSpec> obstacle_walls_;
};

}  // namespace turtle_arena
