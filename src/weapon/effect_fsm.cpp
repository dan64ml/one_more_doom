#include "effect_fsm.h"

#include <cassert>

namespace wpn {

EffectFSM::EffectFSM(id::statenum_t state) : current_state_(id::states[state]) {
  assert(state != id::S_NULL);
  state_number_ = state;
}

bool EffectFSM::Tick(std::vector<id::FuncId>& commands) {
  assert(current_state_.tics != 0);

  commands.resize(0);

  // Special case #2. We have to run this state together with next state
  // that has tics != 0.
  // TODO: several tics==0 in a row
  
  //if (current_state_.tics == 0) {
  //  commands.push_back(current_state_.action);
  //  if (current_state_.nextstate == id::S_NULL) {
  //    return false;
  //  }
  //  current_state_ = id::states[current_state_.nextstate];
  //}
  

  // Special case #1. Spin forever (?? or stop the fsm ??)
  if (current_state_.tics == -1) {
    commands.push_back(current_state_.action);
    return true;
  }

  // SC #2 two times in a row (??!)
  //assert(current_state_.tics != 0);

  if (current_state_.tics == id::states[state_number_].tics) {
    commands.push_back(current_state_.action);
  }

  if (--current_state_.tics == 0) {
    do {
      if (current_state_.nextstate == id::S_NULL) {
        return false;
      }
      current_state_ = id::states[current_state_.nextstate];
      commands.push_back(current_state_.action);
    } while(!current_state_.tics);
  }

  return true;
}

std::string EffectFSM::GetSpriteName() const {
  std::string result = id::sprnames[current_state_.sprite];
  result += 'A' + current_state_.frame;
  result += '0';
//  if (result == "SHTGE0") {
//    return "";
//  }
  return result;
}

} // namespace wpn
