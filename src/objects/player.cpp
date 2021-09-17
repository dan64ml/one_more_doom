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

void Player::FirePistol(bool refire) {
  int damage = 5 * (rand() % 3 + 1);
  rend::BamAngle da = refire ? (rand() - rand()) % 256 : 0;
  world_->HitLineAttack(this, damage, kMissileRange, da);
}

//void Player::FireShotgun() {
//  for (int i = 0; i < 7; ++i) {
//    int damage = 5 * (rand() % 3 + 1);
//    rend::BamAngle da = (rand() - rand()) % 512;
//    world_->HitLineAttack(this, damage, kMissileRange, da);
//  }
//}

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

} // namespace mobj