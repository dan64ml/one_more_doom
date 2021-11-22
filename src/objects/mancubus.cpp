#include "mancubus.h"

#include "world/world.h"

namespace mobj {

Mancubus::Mancubus(id::mobjtype_t type)
  : Monster(type) {

}

void Mancubus::A_FatRaise() {
  A_FaceTarget();
  PlaySound(id::sfx_manatk);
}

void Mancubus::A_FatAttack1() {
  if (!target_) {
    return;
  }

  angle += kFatSpread;
  world_->SpawnProjectile(id::MT_FATSHOT, this, target_);

  angle += kFatSpread;
  world_->SpawnProjectile(id::MT_FATSHOT, this);
  angle -= kFatSpread;
}

void Mancubus::A_FatAttack2() {
  if (!target_) {
    return;
  }

  angle -= kFatSpread;
  world_->SpawnProjectile(id::MT_FATSHOT, this, target_);

  angle -= 2 * kFatSpread;
  world_->SpawnProjectile(id::MT_FATSHOT, this);
  angle += 2 * kFatSpread;
}

void Mancubus::A_FatAttack3() {
  if (!target_) {
    return;
  }

  auto old_an = angle;
  angle -= kFatSpread / 2;
  world_->SpawnProjectile(id::MT_FATSHOT, this);

  angle += kFatSpread;
  world_->SpawnProjectile(id::MT_FATSHOT, this);
  angle = old_an;
}


} // namespace mobj
