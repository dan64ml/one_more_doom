#include "heavy_dude.h"

#include "world/world.h"

namespace mobj {

HeavyDude::HeavyDude(id::mobjtype_t type) : Monster(type) {

}

void HeavyDude::A_CPosAttack() {
  if (!target_) {
    return;
  }

  PlaySound(id::sfx_shotgn);
  A_FaceTarget();

  rend::BamAngle vert_angle = world_->GetTargetAngle(x, y, z, angle, kMissileRange);
  int damage = ((rand() % 5) + 1) * 3;
  world_->HitAngleLineAttack(this, damage, kMissileRange, angle, vert_angle);
}

void HeavyDude::A_CPosRefire() {
  A_FaceTarget();

  if ((rand() % 256) < 40) {
    return;
  }

  if (!target_ || target_->GetHealth() <= 0 ||
      !world_->CheckSight(this, target_)) {
    fsm_.ToSeeState(this);
  }
}


} // namespace mobj
