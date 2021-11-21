#include "player.h"

#include <cassert>
#include <variant>
#include <cmath>

#include "world/world.h"
#include "utils/world_utils.h"

namespace mobj {

void Player::Move(rend::BamAngle delta_angle, double forward_move, double side_move) {
  angle += delta_angle;

  if (z > floor_z_) {
    // can't move when fly
    return;
  }

  double dx = forward_move * rend::BamCos(angle);
  double dy = forward_move * rend::BamSin(angle);
  dx += side_move * rend::BamCos(angle - rend::kBamAngle90);
  dy += side_move * rend::BamSin(angle - rend::kBamAngle90);

  mom_x += dx;
  mom_y += dy;
}

void Player::SetFireFlag(bool fire) {
  weapon_.SetFireFlag(fire);
}

bool Player::TickTime() {
  if (use_command_) {
    use_command_ = false;
    auto line = FindSpecialLine();
    if (line) {
      world_->UseLine(line, this);
    }
  }

  MovingObject::TickTime();
  return weapon_.TickTime();
}

world::IntersectedObject Player::GetClosestObstacle(double new_x, double new_y) {
  double dx = new_x - x;
  double dy = new_y - y;

  // Corners' coordinates
  double front_x, front_y, back_x, back_y;
  if (dx > 0) {
    front_x = x + radius;
    back_x = x - radius;
  } else {
    front_x = x - radius;
    back_x = x + radius;
  }

  if (dy > 0) {
    front_y = y + radius;
    back_y = y - radius;
  } else {
    front_y = y - radius;
    back_y = y + radius;
  }

  std::vector<std::pair<double, double>> corns = {{front_x, front_y}, {back_x, front_y}, {front_x, back_y}};
  world::IntersectedObject result {1'000'000, 0, 0, {}};

  for (auto [cx, cy] : corns) {
    auto ret = world_->CreateIntersectedObjList(cx, cy, cx + dx, cy + dy);
    for (auto& obj : ret) {
      if (obj.obj.index() == 1) {
        // mobj
        if (obj.distance < result.distance) {
          result = obj;
        }
      } else {
        auto line = std::get<0>(obj.obj);
        if (!line->sides[1]) {
          // wall
          if (obj.distance < result.distance) {
            result = obj;
          }
        } else {
          // portal. passable?
          auto [floor_h, ceiling_h] = math::GetOppositeFloorCeilingHeight(line, cx, cy);
          if ((line->flags & world::kLDFBlockEveryOne) || floor_h - z > kMaxStepSize || ceiling_h < z + height) {
            // obstacle
            if (obj.distance < result.distance) {
              result = obj;
            }
          }
        }
      }
    }
  }

  return result;
}

bool Player::RunIntoAction(double new_x, double new_y) {
  auto obstacle = GetClosestObstacle(new_x, new_y);

  world::Line* line = nullptr;

  if (obstacle.obj.index() == 0) {
    line = std::get<0>(obstacle.obj);
  } else if (obstacle.obj.index() == 1) {
    auto mobj = std::get<1>(obstacle.obj);
  } else {
    assert(false);
  }


  if (line) {
    double mom_dx = new_x - x;
    double mom_dy = new_y - y;

    double lx = line->x1 - line->x2;
    double ly = line->y1 - line->y2;

    double llen = sqrt(lx*lx + ly*ly);
    double nlx = lx / llen;
    double nly = ly / llen;

    double coef = nlx * mom_dx + nly * mom_dy;

    double dx = nlx * std::abs(coef);
    double dy = nly * std::abs(coef);

    if (coef < 0) {
      dx *= -1;
      dy *= -1;
    }

    TryMoveTo(x + dx, y + dy);

    return true;
  }

  return true;
}

void Player::FireMissile() {
  world_->SpawnProjectile(id::MT_ROCKET, this);
}

void Player::FirePlasma() {
  world_->SpawnProjectile(id::MT_PLASMA, this);
}

void Player::SlowDown() {
  if (z > floor_z_) {
    return;
  }

  mom_x = mom_y = 0;
}

void Player::FirePistol(bool refire) {
  auto slope = world_->GetTargetAngle(x, y, z + kWeaponHeight, angle, kMissileRange);
  int damage = 5 * (rand() % 3 + 1);
  rend::BamAngle dh = refire ? (rand() - rand()) % 512 : 0;
  rend::BamAngle dv = (rand() - rand()) % 128;

  world_->HitAngleLineAttack(this, damage, kMissileRange, angle + dh, slope + dv);
}

void Player::FireShotgun() {
  auto slope = world_->GetTargetAngle(x, y, z + kWeaponHeight, angle, kMissileRange);

  for (int i = 0; i < 7; ++i) {
    int damage = 5 * (rand() % 3 + 1);
    rend::BamAngle dh = (rand() - rand()) % 512;
    rend::BamAngle dv = (rand() - rand()) % 128;

    world_->HitAngleLineAttack(this, damage, kMissileRange, angle + dh, slope + dv);
  }
}

void Player::FireSuperShotgun() {
  auto slope = world_->GetTargetAngle(x, y, z + kWeaponHeight, angle, kMissileRange);

  for (int i = 0; i < 20; ++i) {
    int damage = 5 * (rand() % 3 + 1);
    rend::BamAngle dh = (rand() - rand()) % 1400;
    rend::BamAngle dv = (rand() - rand()) % 1400;

    world_->HitAngleLineAttack(this, damage, kMissileRange, angle + dh, slope + dv);
  }
}

void Player::Punch() {
  auto hit_direction = angle + (rand() - rand()) % 128;
  auto slope = world_->GetTargetAngle(x, y, z + kWeaponHeight, hit_direction, kMeleeRange);

  int damage = 2 * (rand() % 10 + 1);
  world_->HitAngleLineAttack(this, damage, kMeleeRange, hit_direction, slope);
}

void Player::ChainSaw() {
  auto hit_direction = angle + (rand() - rand()) % 512;
  auto slope = world_->GetTargetAngle(x, y, z + kWeaponHeight, hit_direction, kMeleeRange);
  rend::BamAngle dv = (rand() - rand()) % 512;

  int damage = 2 * (rand() % 10 + 1);
  world_->HitAngleLineAttack(this, damage, kMeleeRange, hit_direction, slope + dv);
}

void Player::FireBFG() {
  world_->SpawnProjectile(id::MT_BFG, this);
}

world::Line* Player::FindSpecialLine() {
  auto obj_list = world_->CreateIntersectedObjList(x, y, angle, kUseDistance);
  for (auto obj : obj_list) {
    int idx = obj.obj.index();
    if (idx == 0) {
      auto line = std::get<0>(obj.obj);
      if (line->specials) {
        // Check the side, we should use only front side
        if (math::LinePointPosition(line, x, y) != math::ObjPosition::kRightSide) {
          continue;
        }
        // TODO: should I check height???
        return const_cast<world::Line*>(line);
      } else {
        // Check for a wall or closed portal
        if (math::GetOpenRange(line) <= 0) {
          break;
        }
      }
    } else {
      // found a mobj
      auto mobj = std::get<1>(obj.obj);
      if (mobj == this) {
        // Ignore yourself
        continue;
      }
      break;
    }
  }
  return nullptr;
}

void Player::ProcessSpecialLine(world::Line* line) {
  world_->CrossLine(line, this);
}

} // namespace mobj