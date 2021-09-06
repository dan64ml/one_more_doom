#include "weapon_fsm.h"

#include <cassert>

namespace wpn {

WeaponFSM::WeaponFSM(const WeaponParam& p) : params_(p) {
  //current_state_ = id::states[params_.ready_state];
  //current_state_ = id::states[params_.active_state];
  current_state_ = id::states[params_.up_state];
}

std::vector<id::FuncId> WeaponFSM::Tick() {
  std::vector<id::FuncId> actions;

  // Special case #2. We have to run this state together with next state
  // that has tics != 0.
  if (current_state_.tics == 0) {
    actions.push_back(current_state_.action);
    current_state_ = id::states[current_state_.nextstate];
  }
  
  // Special case #1. Spin forever (?? or stop the fsm ??)
  if (current_state_.tics == -1) {
    actions.push_back(current_state_.action);
    return actions;
  }

  // SC #2 two times in a row (??!)
  assert(current_state_.tics != 0);

  actions.push_back(current_state_.action);

  if (--current_state_.tics == 0) {
    current_state_ = id::states[current_state_.nextstate];
  }

  return actions;
}

std::string WeaponFSM::GetSpriteName() const {
  std::string result = id::sprnames[current_state_.sprite];
  result += 'A' + current_state_.frame;
  result += '0';
  return result;
}

void WeaponFSM::ToReadyState() {
  current_state_ = id::states[params_.ready_state];
}

void WeaponFSM::ToActiveState() {
  current_state_ = id::states[params_.active_state];
}

void WeaponFSM::ToFlashState() {
  current_state_ = id::states[params_.flash_state];
}

} // namespace wpn