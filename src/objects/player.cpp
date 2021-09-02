#include "player.h"

#include <cassert>

#include "world/world.h"

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

void Player::Fire() {
  auto ret = weapon_.Fire(ammo_);
  switch (ret.index())
  {
  case 0:
    if (!std::get<bool>(ret)) {
      // change active weapon
    }
    break;
  
  case 1:
    // process Projectile
    world_->SpawnProjectile(Projectile(std::get<wpn::ProjectileParams>(ret)), this);
    break;

  case 2:
    // process Hitscan
    break;

  default:
    // something went wrong ((
    assert(false);
    break;
  }
}

bool Player::TickTime() {
  MapObject::TickTime();
  return weapon_.TickTime();
}

bool Player::RunIntoAction() {
  return true;
}

} // namespace mobj