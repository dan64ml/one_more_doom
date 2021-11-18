#include "map_object.h"

#include <cassert>
#include <algorithm>
#include <iostream>
#include <cmath>

#include "world/world.h"
#include "utils/world_utils.h"
#include "renderer/plane_utils.h"

namespace mobj {

MapObject::MapObject(id::mobjtype_t type) : fsm_(id::mobjinfo[type]) {
  assert(type < id::NUMMOBJTYPES);
  mobj_type = type;

  flags = id::mobjinfo[type].flags;
  height = id::mobjinfo[type].height;
  radius = id::mobjinfo[type].radius;
  speed = id::mobjinfo[type].speed;

  spawn_health_ = health_ = id::mobjinfo[type].spawnhealth;
  pain_chance_ = id::mobjinfo[type].painchance;
}

std::string MapObject::GetSpriteName(int vp_x, int vp_y) const {
  auto vp_angle = rend::CalcAngle(x, y, vp_x, vp_y);

  rend::BamAngle da = (vp_angle - (rend::BamAngle)(angle - rend::kBamAngle45 / 2));
  int sector = da / rend::kBamAngle45;
  ++sector;
  if (sector > 8) {
    sector -= 8;
  }

  return fsm_.GetSpriteName() + std::to_string(sector);
}

bool MapObject::TickTime() {
  if (delete_me_) {
    return false;
  }

  fsm_.Tick(this);
  
  return true;
}

bool MapObject::ChangeSubSector(world::SubSector* new_ss) {
  auto it = std::find(begin(ss_->mobjs), end(ss_->mobjs), this);
  if (it == end(ss_->mobjs)) {
    return false;
  }

  ss_->mobjs.erase(it);

  new_ss->mobjs.push_back(this);

  ss_ = new_ss;

  return true;
}

void MapObject::CauseDamage(int damage) {
  health_ -= damage;

  if (health_ > 0) {
    if (rand() % 256 < pain_chance_) {
      fsm_.ToPainState(this);
    }
    return;
  } else {
    if (health_ < -spawn_health_) {
      fsm_.ToXDeathState(this);
    } else {
      fsm_.ToDeathState(this);
    }

    // remains are visible but not interactable
    // TODO: flying objects must fall down
    flags &= ~(MF_SOLID | MF_SHOOTABLE);
  }
}

bool MapObject::AdjustMobjHeight() {
  bool is_on_floor = (abs(z - floor_z) < kEps);

  // Simplified case: mobj can't change its (x, y) position
  floor_z = ss_->sector->floor_height;
  ceiling_z = ss_->sector->ceiling_height;

  if (is_on_floor) {
    z = floor_z;
  } else {
    if (z + height > ceiling_z) {
      z = ceiling_z - height;
    }
  }

  return (ceiling_z - floor_z < height) ? false : true;
}

void MapObject::DamageBySobj(int damage) {
  if (health_ <= 0) {
    // Get giblets
    fsm_.ToGibsState(this);
    flags &= ~mobj::MF_SOLID;
    height = 0;
    radius = 0;
    return;
  }

  if (flags & mobj::MF_DROPPED) {
    // Should disapear
    delete_me_ = true;
    return;
  }

  if (!(flags & mobj::MF_SHOOTABLE)) {
    // Can't interact
    return;
  }

  CauseDamage(damage);
}

} // namespace mobj