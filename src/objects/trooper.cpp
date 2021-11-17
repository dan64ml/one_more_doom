#include "trooper.h"

#include "world/world.h"
#include "renderer/plane_utils.h"

namespace mobj {

Trooper::Trooper(id::mobjtype_t type) : MapObject(type) {

}

void Trooper::CallStateFunction([[maybe_unused]] id::FuncId foo_id) {
  switch (foo_id)
  {
    case id::A_Look:
      LookFoo();
      break;

    case id::A_Chase:
      ChaseFoo();
      break;
    
    case id::A_FaceTarget:
      FaceTargetFoo();
      break;

    case id::A_PosAttack:
      Attack();
      break;

    default:
      break;
  }
}

void Trooper::LookFoo() {
  // TODO: process noise !!!!!
  target_ = world_->IsPlayerVisible(this);
  if (target_) {
    fsm_.ToSeeState(this);
  }
}

void Trooper::ChaseFoo() {
  // TODO: add reaction time

  if (move_dir_ != ZZDir::kNoDir) {
    // Sync angle
    angle = ZZDirToBam(move_dir_);
  }

  if (!target_ || !(target_->flags & mobj::MF_SHOOTABLE)) {
    target_ = world_->LookForPlayer(this, true);
    if (!target_) {
      fsm_.ToSpawnState(this);
    }
    return;
  }

  // Keep original behavior, don't attack twice in a row
  if (flags & mobj::MF_JUSTATTACKED) {
    flags &= ~mobj::MF_JUSTATTACKED;
    NewChaseDirection();
    return;
  }

  if (CheckMeleeAttack()) {
    fsm_.ToMeleeState(this);
    return;
  }

  if (CheckMissileAttack()) {
    fsm_.ToMissileState(this);
    flags |= mobj::MF_JUSTATTACKED;
    return;
  }

  // Movement
  if (--move_count_ < 0 || !ZZMove()) {
    NewChaseDirection();
  }
}

bool Trooper::CheckMissileAttack() {
  if (!world_->IsPlayerVisible(this)) {
    return false;
  }

  if (flags & mobj::MF_JUSTHIT) {
    flags &= ~mobj::MF_JUSTHIT;
    return true;
  }

  double dist = rend::SegmentLength(x, y, target_->x, target_->y) - 64;
  dist = std::min(dist, 200.0);

  return ((rand() % 255) < dist) ? false : true;
}

void Trooper::FaceTargetFoo() {
  if (!target_) {
    return;
  }

  flags &= ~mobj::MF_AMBUSH;

  angle = rend::CalcAngle(x, y, target_->x, target_->y);
}

void Trooper::Attack() {
  if (!target_) {
    return;
  }

  // Keep original behavier
  FaceTargetFoo();

  rend::BamAngle vert_angle = world_->GetTargetAngle(x, y, z, angle, kMissileRange);
  int damage = ((rand() % 5) + 1) * 3;
  world_->HitAngleLineAttack(this, damage, kMissileRange, angle, vert_angle);
}

} // namespace mobj
