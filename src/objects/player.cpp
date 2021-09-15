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

void Player::SetFireFlag(bool fire) {
  weapon_.SetFireFlag(fire);
}

bool Player::TickTime() {
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

void Player::FirePistol() {
  world_->HitLineAttack(this, 1, 1024, 0);
}

} // namespace mobj