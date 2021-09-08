#ifndef WEAPON_FSM_H_
#define WEAPON_FSM_H_

#include "objects/base_fsm.h"

namespace wpn {

class Weapon;

class WeaponFSM : public mobj::BaseFSM<Weapon> {
 public:
  WeaponFSM() = default;
  WeaponFSM(id::statenum_t state) : BaseFSM(state) {}

  std::string GetSpriteName() const;

 private:
  void CallStateFunction(id::FuncId foo_id, Weapon* obj) override;

};

} // namespace wpn

#endif  // WEAPON_FSM_H_
