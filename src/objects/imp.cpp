#include "imp.h"

#include "world/world.h"

namespace mobj {

Imp::Imp(id::mobjtype_t type) 
  : Monster(type) {

}

void Imp::A_TroopAttack() {
  if (!target_) {
    return;
  }

  A_FaceTarget();
  if (CheckMeleeRange()) {
    PlaySound(id::sfx_claw);

    int damage = (rand() % 8 + 1) * 3;
    target_->CauseDamage(damage, this, this);
    return;
  }

  world_->SpawnProjectile(id::MT_TROOPSHOT, this, target_);
}

} // namespace mobj
