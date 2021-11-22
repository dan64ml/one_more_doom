#include "shotguy.h"

#include "world/world.h"

namespace mobj {

ShotGuy::ShotGuy(id::mobjtype_t type)
  : Monster(type) {

}

void ShotGuy::A_SPosAttack() {
  if (!target_) {
    return;
  }

  PlaySound(id::sfx_shotgn);
  A_FaceTarget();

  rend::BamAngle vert_angle = world_->GetTargetAngle(x, y, z, angle, kMissileRange);

  for (int i = 0; i < 3; ++i) {
    auto an = angle;
    an += ((rand() - rand()) % 256) * 8;

    int damage = ((rand() % 5) + 1) * 3;
    world_->HitAngleLineAttack(this, damage, kMissileRange, an, vert_angle);
  }

}

} // namespace mobj
