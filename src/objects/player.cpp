#include "player.h"

#include <cassert>
#include <variant>

#include "world/world.h"
#include "utils/world_utils.h"

namespace mobj {

void Player::Move(rend::BamAngle delta_angle, double forward_move, double side_move) {
  angle += delta_angle;

  if (z > floor_z) {
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

  MapObject::TickTime();
  return weapon_.TickTime();
}

bool Player::RunIntoAction() {
  return true;
}

void Player::FireMissile() {
  world_->SpawnProjectile(id::MT_ROCKET, this);
}

void Player::FirePlasma() {
  world_->SpawnProjectile(id::MT_PLASMA, this);
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

bool Player::ProcessLine(world::Line* line) {
  if (line->sides[1] == nullptr) {
    return false;
  }

  if (line->specials) {
    world_->CrossLine(line, this);
  }

  return true;
}

} // namespace mobj