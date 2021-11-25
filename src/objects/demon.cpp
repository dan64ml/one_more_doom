#include "demon.h"

#include "world/world.h"

namespace mobj {

Demon::Demon(id::mobjtype_t type) : Monster(type) {

}

void Demon::A_SargAttack() {
  if (!target_) {
    return;
  }

  A_FaceTarget();
  if (CheckMeleeRange()) {
    int damage = ((rand() % 10) + 1) * 4;
    target_->CauseDamage(damage, this, this);
  }
}

} // namespace mobj
