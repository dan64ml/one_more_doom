#pragma once

#include "objects/base_fsm.h"

namespace wpn {

class Weapon;

class NewWeaponFSM : public mobj::BaseFSM<Weapon> {
 public:
  NewWeaponFSM() = default;

  std::string GetSpriteName() const;
  
 private:
  void CallStateFunction(id::FuncId foo_id, Weapon* obj) override;

};

} // namespace wpn
