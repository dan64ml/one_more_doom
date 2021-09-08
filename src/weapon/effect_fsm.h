#ifndef EFFECT_FSM_H_
#define EFFECT_FSM_H_

#include <string>
#include <vector>

#include "objects/info.h"

namespace wpn {

class EffectFSM {
 public:
  EffectFSM(id::statenum_t state);

  // Returns false if the fsm should be destroyed
  bool Tick(std::vector<id::FuncId>& commands);

  std::string GetSpriteName() const;

 private:
  id::state_t current_state_;
  int state_number_;
};

} // namespace wpn

#endif  // EFFECT_FSM_H_
