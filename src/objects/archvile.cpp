#include "archvile.h"

namespace mobj {

ArchVile::ArchVile(id::mobjtype_t type) : Monster(type) {

}

void ArchVile::A_VileChase() {
  // Look for corpses to resurrect
  if (GetMoveDirection() != ZZDir::kNoDir) {
    double dist = 2 *kMaxRadius;
    double new_x = x + speed * rend::BamCos(ZZDirToBam(GetMoveDirection()));
    double new_y = x + speed * rend::BamSin(ZZDirToBam(GetMoveDirection()));
    world::BBox bbox;
    bbox.left = new_x - dist;
    bbox.right = new_x + dist;
    bbox.top = new_y + dist;
    bbox.bottom = new_y - dist;

    for (auto mobj : world_->GetBlocks().GetMapObjects(bbox)) {
      if (!mobj->IsResurrectable()) {
        continue;
      }
      
      double collision_dist = radius + mobj->radius;
      if (std::abs(new_x - mobj->x) > collision_dist || std::abs(new_y - mobj->y) > collision_dist) {
        // Too far
        continue;
      }
  
      auto tmp = target_;
      target_ = mobj;
      A_FaceTarget();
      target_ = tmp;
  
      fsm_.SetState(id::S_VILE_HEAL1, this);
      PlaySound(id::sfx_slop);
  
      mobj->Resurrect();
      return;
    }
  }

  A_Chase();
}

void ArchVile::A_VileStart() {
  PlaySound(id::sfx_vilatk);
}

void ArchVile::A_VileTarget() {
  if (!target_) {
    return;
  }

  A_FaceTarget();

  world_->SpawnMapObject(id::MT_FIRE, target_->x, target_->y, 0);
}

void ArchVile::A_VileAttack() {

}

} // namespace mobj
