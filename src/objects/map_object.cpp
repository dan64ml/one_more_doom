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
  mass_ = id::mobjinfo[type].mass;
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

bool MapObject::ChangeSubSector(int new_x, int new_y) {
  auto it = std::find(begin(ss_->mobjs), end(ss_->mobjs), this);
  if (it == end(ss_->mobjs)) {
    return false;
  }

  ss_->mobjs.erase(it);

  int ss_idx = world_->bsp_.GetSubSectorIdx(new_x, new_y);
  auto* new_ss = &world_->sub_sectors_[ss_idx];

  new_ss->mobjs.push_back(this);

  ss_ = new_ss;

  return true;
}

void MapObject::CauseDamage(int damage, MapObject* inflictor, MapObject* source) {
  if (!(flags & MF_SHOOTABLE)) {
    return;
  }

  if (health_ <= 0) {
    return;
  }

  // Thrust emulation
  if (inflictor) {
    auto an = rend::CalcAngle(inflictor->x, inflictor->y, x, y);
    double thrust = damage * 12.5 / mass_;

    // Original behavior
    if (damage < 40 && damage > health_ && (rand() & 1) &&
        (z - inflictor->z > 64)) {
      an += rend::kBamAngle180;
      thrust *= 4;
    }

    mom_x += thrust * rend::BamCos(an);
    mom_y += thrust * rend::BamSin(an);
  }

  health_ -= damage;
  if (health_ <= 0) {
    KillMobj(source);
    return;
  }

  if (rand() % 256 < pain_chance_) {
    flags |= MF_JUSTHIT;
    fsm_.ToPainState(this);
  }

  reaction_time_ = 0;

  if (!threshold_ && source && source != this) {
    target_ = source;
    threshold_ = kBaseThreshold;

    if (fsm_.IsSpawnState() && fsm_.HasState(FsmState::kSee)) {
      fsm_.ToSeeState(this);
    }
  }
}

void MapObject::KillMobj(MapObject* source) {
  flags &= ~(MF_SHOOTABLE | MF_FLOAT | MF_SKULLFLY);

  if (mobj_type != id::MT_SKULL) {
    flags &= ~MF_NOGRAVITY;
  }

  flags |= MF_CORPSE | MF_DROPOFF;
  height /= 4;

  // Count frags
  if (source && source->mobj_type == id::MT_PLAYER) {
    if (flags & MF_COUNTKILL) {
      // TODO: add count
    }
  }

  if (health_ < -spawn_health_ && fsm_.HasState(FsmState::kXDeath)) {
    fsm_.ToXDeathState(this);
  } else {
    fsm_.ToDeathState(this);
  }

  id::mobjtype_t drop;
  switch (mobj_type)
  {
    case id::MT_WOLFSS:
    case id::MT_POSSESSED:
      drop = id::MT_CLIP;
      break;
    case id::MT_SHOTGUY:
      drop = id::MT_SHOTGUN;
      break;
    case id::MT_CHAINGUY:
      drop = id::MT_CHAINGUN;
      break;
    
    default:
      return;
  }

  world_->SpawnMapObject(drop, x, y, MF_DROPPED);
}

bool MapObject::AdjustMobjHeight() {
  bool is_on_floor = (abs(z - floor_z_) < kEps);

  // Simplified case: mobj can't change its (x, y) position
  floor_z_ = ss_->sector->floor_height;
  ceiling_z_ = ss_->sector->ceiling_height;

  if (is_on_floor) {
    z = floor_z_;
  } else {
    if (z + height > ceiling_z_) {
      z = ceiling_z_ - height;
    }
  }

  return (ceiling_z_ - floor_z_ < height) ? false : true;
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

  CauseDamage(damage, nullptr, nullptr);
}

void MapObject::TieToMap(world::World* world, world::SubSector* ss, bool keep_z) {
  world_ = world;
  ss_ = ss;

  floor_z_ = ss_->sector->floor_height;
  ceiling_z_ = ss_->sector->ceiling_height;
  if (!keep_z) {
    z = ss_->sector->floor_height;
  }
}

bool MapObject::IsResurrectable() const {
  if (!(flags & MF_CORPSE)) {
    // A thing or still alive
    return false;
  }

  if (!fsm_.HasState(FsmState::kRaise) || !fsm_.IsSpinState()) {
    return false;
  }

  return true;
}

void MapObject::Resurrect() {
  flags = id::mobjinfo[mobj_type].flags;
  height = id::mobjinfo[mobj_type].height;
  health_ = id::mobjinfo[mobj_type].spawnhealth;

  mom_x = mom_y = mom_z = 0;

  target_ = nullptr;

  fsm_.SetState(static_cast<id::statenum_t>(id::mobjinfo[mobj_type].raisestate), this);
}

} // namespace mobj