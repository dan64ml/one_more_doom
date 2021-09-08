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
      current_state_ = nullptr;
    } else {
      current_state_ = &id::states[state];
    }
  }

  // Returns false if the fsm is disabled. It can mean the object must be deleted
  bool Tick(T* obj) {
    if (current_state_) {
      // Special case, spin forever
      if (current_state_->tics == -1) {
        return true;
      }

      if (--current_state_->tics == 0) {
        SetState(current_state_->nextstate, obj);
      }
    } else {
      return false;
    }

    return true;
  }

  void SetState(id::statenum_t new_state, T* obj) {
    do {
      if (new_state == id::S_NULL) {
        // Disable the fsm.
        current_state_ = nullptr;
        return;
      }

      current_state_ = &id::states[new_state];

      CallStateFunction(current_state_->action, obj);

      // Cunning case. If in new state tick == 0 we must switch to the next state immediately
      new_state = current_state_->nextstate;

    } while(current_state_->tics == 0);
  }

  // Returns sprite name without direction index
  std::string GetSpriteBaseName() const {
    if (!current_state_) {
      return "";
    } else {
      return id::sprnames[current_state_->sprite] + ('A' + current_state_->frame);
    }
  }

 protected:
  virtual void CallStateFunction(id::FuncId foo_id, T* obj) {}

 private:
  id::state_t* current_state_ = nullptr;

};

} // namespace mobj

#endif  // BASE_FSM_H_
