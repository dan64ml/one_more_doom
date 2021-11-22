#include "trooper.h"

#include "world/world.h"
#include "renderer/plane_utils.h"

namespace mobj {

Trooper::Trooper(id::mobjtype_t type) : Monster(type) {

}

void Trooper::A_PosAttack() {
  if (!target_) {
    return;
  }

  // Keep original behavior
  A_FaceTarget();

  PlaySound(id::sfx_pistol);

  rend::BamAngle vert_angle = world_->GetTargetAngle(x, y, z, angle, kMissileRange);
  int damage = ((rand() % 5) + 1) * 3;
  world_->HitAngleLineAttack(this, damage, kMissileRange, angle, vert_angle);
}

bool Trooper::InfluenceObject(MapObject* obj) {
  if (obj->flags & MF_SOLID) {
    return false;
  }

  return true;
}

} // namespace mobj
