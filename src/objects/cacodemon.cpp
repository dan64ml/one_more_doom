#include "cacodemon.h"

#include "renderer/plane_utils.h"
#include "world/world.h"

namespace mobj {

Cacodemon::Cacodemon(id::mobjtype_t type) : Monster(type) {
}

void Cacodemon::ZMove() {
  z += mom_z;

  // Float down to the target
  if (target_ && !(flags & MF_INFLOAT)) {
    double dist = rend::SegmentLength(x, y, target_->x, target_->y);

    int delta = target_->z + height / 2 - z;
    if (delta < 0 && dist < -3 * delta) {
      z -= kFloatSpeed;
    } else if (delta > 0 && dist < 3 * delta) {
      z += kFloatSpeed;
    }
  }

  if (z <= floor_z_) {
    z = floor_z_;
    mom_z = std::max(0.0, mom_z);
  } else if (!(flags & MF_NOGRAVITY)) {
    if (mom_z == 0) {
      mom_z = -2 * kGravity;
    } else {
      mom_z -= kGravity;
    }
  }

  if (z + height > ceiling_z_) {
    z = ceiling_z_ - height;
    mom_z = std::min(0.0, mom_z);
  }
}

void Cacodemon::A_HeadAttack() {
  if (!target_) {
    return;
  }

  A_FaceTarget();

  if (CheckMeleeRange()) {
    int damage = (rand() % 6 + 1) * 10;
    target_->CauseDamage(damage, this, this);
    return;
  }

  // TODO: Spawn headshot
  world_->SpawnProjectile(id::MT_HEADSHOT, this, target_);
}

} // namespace mobj
