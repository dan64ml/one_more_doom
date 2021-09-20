#include "fsm.h"

#include <cassert>
#include <limits>

namespace mobj {

FSM::FSM(const id::mobjinfo_t& info) : BaseFSM(static_cast<id::statenum_t>(info.spawnstate)) {
  //current_state = id::states[info.spawnstate];

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

void FSM::CallStateFunction([[maybe_unused]] id::FuncId foo_id, [[maybe_unused]] MapObject* obj) {
  switch (foo_id)
  {
    case id::A_NULL:
      break;
    case id::A_BFGSpray:
      //obj->WeaponReady();
      break;

    default:
      break;
  }
}

} // namespace fsm
