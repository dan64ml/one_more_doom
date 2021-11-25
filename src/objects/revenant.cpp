#include "revenant.h"

namespace mobj {

Revenant::Revenant(id::mobjtype_t type) : Monster(type) {

}

void Revenant::A_SkelWhoosh() {
  if (!target_) {
    return;
  }

  A_FaceTarget();
  PlaySound(id::sfx_skeswg);
}

void Revenant::A_SkelFist() {
  if (!target_) {
    return;
  }

  A_FaceTarget();

  if (CheckMeleeRange()) {
    int damage = ((rand() % 10) + 1) * 6;
    target_->CauseDamage(damage, this, this);
  }
}

void Revenant::A_SkelMissile() {
  if (!target_) {
    return;
  }

  A_FaceTarget();
  // TODO: dirty hack to change spawn height. Add explicit spawn height...
  z += 16;
  world_->SpawnProjectile(id::MT_TRACER, this, target_);
  z -= 16;
}


} // namespace mobj
