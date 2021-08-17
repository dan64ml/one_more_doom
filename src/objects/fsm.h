#ifndef FSM_H_
#define FSM_H_

#include <unordered_map>
#include <string>

#include "info.h"

namespace mobj {

class FSM {
 public:
  FSM() = default;
  FSM(const id::mobjinfo_t& info);
  bool Tick();

  std::string GetSpriteName() const;
  
  void ToSeeState();
  void ToDeathState();

 private:
  id::state_t current_state;

  id::statenum_t spawn_state;
  id::statenum_t see_state;
  id::statenum_t pain_state;
  id::statenum_t melee_state;
  id::statenum_t missile_state;
  id::statenum_t death_state;
  id::statenum_t xdeath_state;
  id::statenum_t raise_state;
};

} // namespace mobj

#endif  // FSM_H_
