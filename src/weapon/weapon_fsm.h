#ifndef WEAPON_FSM_H_
#define WEAPON_FSM_H_

#include "weapon_types.h"

#include <string>
#include <vector>

#include "objects/info.h"

namespace wpn {

class WeaponFSM {
 public:
  WeaponFSM(const WeaponParam& p);

  // FSM can requires several commands (if some of them have tic == 0)
  std::vector<id::FuncId> Tick();
  
  std::string GetSpriteName() const;

  void ToReadyState();
  void ToActiveState();
  void ToFlashState();

 private:
  const WeaponParam params_;

  id::state_t current_state_;
};

}

#endif  // WEAPON_FSM_H_