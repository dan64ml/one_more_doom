#pragma once

#include "objects/base_fsm.h"

namespace wpn {

class Weapon;

class NewWeaponFSM : public mobj::BaseFSM<Weapon> {
 public:
  NewWeaponFSM() = default;
  NewWeaponFSM(id::statenum_t state) : BaseFSM(state) {}

  std::string GetSpriteName() const;

 private:
  void CallStateFunction(id::FuncId foo_id, Weapon* obj) override;

};

} // namespace wpn
