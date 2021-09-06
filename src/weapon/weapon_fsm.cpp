#include "weapon_fsm.h"

namespace wpn {

WeaponFSM::WeaponFSM(const WeaponParam& p) : params_(p) {
  //current_state_ = id::states[params_.ready_state];
  current_state_ = id::states[params_.ready_state];
}

bool WeaponFSM::Tick() {
  if (current_state_.tics == -1 || current_state_.tics == 0) {
    return true;
  }
  
  // TODO: run an action
  if (current_state_.action) {
    current_state_.action();
  }

  if (--current_state_.tics == 0) {
    // check if it's the last state
    if (current_state_.nextstate == id::S_NULL) {
      return false;
    }
    current_state_ = id::states[current_state_.nextstate];
  }

  return true;
}

std::string WeaponFSM::GetSpriteName() const {
  std::string result = id::sprnames[current_state_.sprite];
  result += 'A' + current_state_.frame;
  result += '0';
  return result;
}

} // namespace wpn