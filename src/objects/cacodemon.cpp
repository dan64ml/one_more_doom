#include "cacodemon.h"

namespace mobj {

Cacodemon::Cacodemon(id::mobjtype_t type) : Monster(type) {
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
}

} // namespace mobj
