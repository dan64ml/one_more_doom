// Finite state machine class that encapsulates the base logic
// of the original DOOM fsm.

#ifndef BASE_FSM_H_
#define BASE_FSM_H_

#include <string>

#include "objects/info.h"

namespace mobj {

template<class T>
class BaseFSM {
 public:
  BaseFSM() = default;
  BaseFSM(id::statenum_t state) {
    if (state == id::S_NULL) {
      disable_ = true;
    } else {
      current_state_ = id::states[state];
      disable_ = false;
    }
  }

  // Returns false if the fsm is disabled. It can mean the object must be deleted
  bool Tick(T* obj) {
    if (!disable_) {
      // Special case, spin forever
      if (current_state_.tics == -1) {
        return true;
      }

      if (--current_state_.tics == 0) {
        SetState(current_state_.nextstate, obj);
      }
    } else {
      return false;
    }

    return true;
  }

  // Flags that fsm reached the last state
  bool IsSpinState() const { return current_state_.tics == -1; }

  void SetState(id::statenum_t new_state, T* obj) {
    do {
      if (new_state == id::S_NULL) {
        // Disable the fsm.
        disable_ = true;
        return;
      }

      current_state_ = id::states[new_state];
      disable_ = false;

      CallStateFunction(current_state_.action, obj);
      if (disable_) {
        // State function can change current state...
        return;
      }

      // Cunning case. If in new state tick == 0 we must switch to the next state immediately
      new_state = current_state_.nextstate;

    } while(current_state_.tics == 0);
  }

  // Returns sprite name without direction index
  std::string GetSpriteBaseName() const {
    if (disable_) {
      return "";
    } else {
      return std::string(id::sprnames[current_state_.sprite]) 
        + static_cast<char>('A' + current_state_.frame);
    }
  }

 protected:
  virtual void CallStateFunction([[maybe_unused]] id::FuncId foo_id, [[maybe_unused]] T* obj) {}

  const id::state_t& CurrentState() const { return current_state_; }

 private:
  id::state_t current_state_;
  bool disable_ = true;
};

} // namespace mobj

#endif  // BASE_FSM_H_
