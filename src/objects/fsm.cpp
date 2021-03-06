#include "fsm.h"

#include <cassert>
#include <limits>

#include "map_object.h"

namespace mobj {

FSM::FSM(const id::mobjinfo_t& info) : BaseFSM(static_cast<id::statenum_t>(info.spawnstate)) {
  spawn_state = static_cast<id::statenum_t>(info.spawnstate);
  see_state = static_cast<id::statenum_t>(info.seestate);
  pain_state = static_cast<id::statenum_t>(info.painstate);
  melee_state = static_cast<id::statenum_t>(info.meleestate);
  missile_state = static_cast<id::statenum_t>(info.missilestate);
  death_state = static_cast<id::statenum_t>(info.deathstate);
  xdeath_state = static_cast<id::statenum_t>(info.xdeathstate);
  raise_state = static_cast<id::statenum_t>(info.raisestate);
}

std::string FSM::GetSpriteName() const {
  return GetSpriteBaseName();
}

void FSM::ToSeeState(MapObject* obj) {
  if (see_state == id::S_NULL) {
    // TODO: assert() would be better?
    return;
  }

  SetState(see_state, obj);
}

void FSM::ToSpawnState(MapObject* obj) {
  if (spawn_state == id::S_NULL) {
    return;
  }

  SetState(spawn_state, obj);
}
void FSM::ToMeleeState(MapObject* obj) {
  if (melee_state == id::S_NULL) {
    return;
  }

  SetState(melee_state, obj);
}

void FSM::ToMissileState(MapObject* obj) {
  if (missile_state == id::S_NULL) {
    return;
  }

  SetState(missile_state, obj);
}

void FSM::ToDeathState(MapObject* obj) {
  if (death_state == id::S_NULL) {
    return;
  }

  SetState(death_state, obj);
}

void FSM::ToXDeathState(MapObject* obj) {
  if (xdeath_state == id::S_NULL) {
    return;
  }

  SetState(xdeath_state, obj);
}

void FSM::ToPainState(MapObject* obj) {
  if (pain_state == id::S_NULL) {
    return;
  }

  SetState(pain_state, obj);
}

void FSM::ToGibsState(MapObject* obj) {
  SetState(id::S_GIBS, obj);
}

void FSM::CallStateFunction([[maybe_unused]] id::FuncId foo_id, [[maybe_unused]] MapObject* obj) {
  obj->CallStateFunction(foo_id);
}

bool FSM::HasState(FsmState state) const {
  switch (state)
  {
    case FsmState::kSee:
      return see_state != id::S_NULL;
    case FsmState::kSpawn:
      return spawn_state != id::S_NULL;
    case FsmState::kMelee:
      return melee_state != id::S_NULL;
    case FsmState::kMissile:
      return missile_state != id::S_NULL;
    case FsmState::kPain:
      return pain_state != id::S_NULL;
    case FsmState::kDeath:
      return death_state != id::S_NULL;
    case FsmState::kXDeath:
      return xdeath_state != id::S_NULL;
    
    default:
      break;
  }
  return false;
}

bool FSM::IsSpawnState() const {
  const id::state_t* cur = &CurrentState();
  return cur == &id::states[spawn_state];
}

} // namespace fsm
