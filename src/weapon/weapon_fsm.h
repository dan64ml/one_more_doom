#ifndef WEAPON_FSM_H_
#define WEAPON_FSM_H_

#include "weapon_types.h"

#include <string>

#include "objects/info.h"

namespace wpn {

class WeaponFSM {
 public:
  WeaponFSM(const WeaponParam& p);

  bool Tick();
  std::string GetSpriteName() const;

 private:
  const WeaponParam params_;

  id::state_t current_state_;

};

}

#endif  // WEAPON_FSM_H_