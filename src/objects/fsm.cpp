#include "fsm.h"

#include <cassert>
#include <limits>

namespace mobj {

FSM::FSM(const id::mobjinfo_t& info) {
  current_state = id::states[info.spawnstate];

  spawn_state = static_cast<id::statenum_t>(info.spawnstate);
  see_state = static_cast<id::statenum_t>(info.seestate);
  pain_state = static_cast<id::statenum_t>(info.painstate);
  melee_state = static_cast<id::statenum_t>(info.meleestate);
  missile_state = static_cast<id::statenum_t>(info.missilestate);
  death_state = static_cast<id::statenum_t>(info.deathstate);
  xdeath_state = static_cast<id::statenum_t>(info.xdeathstate);
  raise_state = static_cast<id::statenum_t>(info.raisestate);
}

bool FSM::Tick() {
  if (current_state.tics == -1 || current_state.tics == 0) {
    return true;
  }
  
  // TODO: run an action

  if (--current_state.tics == 0) {
    // check if it's the last state
    if (current_state.nextstate == id::S_NULL) {
      return false;
    }
    current_state = id::states[current_state.nextstate];
  }

  return true;
}

std::string FSM::GetSpriteName() const {
  std::string result = id::sprnames[current_state.sprite];
  result += 'A' + current_state.frame;
  return result;
}

void FSM::ToSeeState() {
  if (see_state == id::S_NULL) {
    return;
  }

  current_state = id::states[see_state];
}

void FSM::ToDeathState() {
  if (death_state == id::S_NULL) {
    return;
  }

  current_state = id::states[death_state];
}

} // namespace fsm
