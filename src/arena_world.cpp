#include "turtle_arena/arena_world.hpp"

namespace turtle_arena
{

namespace detail
{

void CollisionCounter::BeginContact(b2Contact * contact)
{
  ++counts_[contact->GetFixtureA()->GetBody()];
  ++counts_[contact->GetFixtureB()->GetBody()];
}

int CollisionCounter::CountFor(const b2Body * body) const
{
  const auto it = counts_.find(body);
  return it == counts_.end() ? 0 : it->second;
}

void CollisionCounter::Reset(const b2Body * body)
{
  counts_.erase(body);
}

}  // namespace detail

namespace
{
constexpr float kWallThicknessM = 0.05f;
}  // namespace

ArenaWorld::ArenaWorld(ArenaDimensions dims)
: dims_(dims), world_(b2Vec2(0.0f, 0.0f))
{
  world_.SetContactListener(&collision_counter_);
  CreateBoundaryWalls();
}

void ArenaWorld::CreateBoundaryWalls()
{
  b2BodyDef wall_def;
  wall_def.type = b2_staticBody;
  b2Body * walls = world_.CreateBody(&wall_def);

  const float w = dims_.width_m;
  const float h = dims_.height_m;
  const float t = kWallThicknessM;

  const WallSpec specs[4] = {
    {w / 2.0f, -t / 2.0f, w / 2.0f, t / 2.0f},          // bottom
    {w / 2.0f, h + t / 2.0f, w / 2.0f, t / 2.0f},       // top
    {-t / 2.0f, h / 2.0f, t / 2.0f, h / 2.0f},          // left
    {w + t / 2.0f, h / 2.0f, t / 2.0f, h / 2.0f},       // right
  };

  for (const auto & s : specs) {
    b2PolygonShape box;
    box.SetAsBox(s.hx, s.hy, b2Vec2(s.cx, s.cy), 0.0f);
    walls->CreateFixture(&box, 0.0f);
  }
}

void ArenaWorld::AddObstacleWalls(const std::vector<WallSpec> & walls)
{
  if (walls.empty()) {
    return;
  }
  b2BodyDef wall_def;
  wall_def.type = b2_staticBody;
  b2Body * body = world_.CreateBody(&wall_def);

  for (const auto & s : walls) {
    b2PolygonShape box;
    box.SetAsBox(s.hx, s.hy, b2Vec2(s.cx, s.cy), 0.0f);
    body->CreateFixture(&box, 0.0f);
  }
  obstacle_walls_.insert(obstacle_walls_.end(), walls.begin(), walls.end());
}

b2Body * ArenaWorld::SpawnRobotBody(float x, float y, float theta)
{
  b2BodyDef body_def;
  body_def.type = b2_dynamicBody;
  body_def.position.Set(x, y);
  body_def.angle = theta;
  b2Body * body = world_.CreateBody(&body_def);

  b2CircleShape circle;
  circle.m_radius = kRobotRadiusM;

  b2FixtureDef fixture_def;
  fixture_def.shape = &circle;
  fixture_def.density = 1.0f;
  fixture_def.friction = 0.3f;
  fixture_def.restitution = 0.0f;
  body->CreateFixture(&fixture_def);

  return body;
}

void ArenaWorld::DestroyRobotBody(b2Body * body)
{
  collision_counter_.Reset(body);
  world_.DestroyBody(body);
}

void ArenaWorld::Step(float dt)
{
  constexpr int32 kVelocityIterations = 8;
  constexpr int32 kPositionIterations = 3;
  world_.Step(dt, kVelocityIterations, kPositionIterations);
}

}  // namespace turtle_arena
