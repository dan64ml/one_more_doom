#include "weapon_fsm.h"

#include <cassert>

#include "weapon.h"

namespace wpn {

void WeaponFSM::CallStateFunction(id::FuncId foo_id, Weapon* obj) {
  switch (foo_id)
  {
    case id::A_NULL:
      break;
    case id::A_WeaponReady:
      obj->WeaponReady();
      break;
    case id::A_Lower:
      obj->Lower();
      break;
    case id::A_Raise:
      obj->Raise();
      break;
    case id::A_ReFire:
      obj->ReFire();
      break;
    case id::A_FireShotgun2:
      obj->FireShotgun2();
      break;
    case id::A_FireShotgun:
      obj->FireShotgun();
      break;
    case id::A_FirePistol:
      obj->FirePistol();
      break;
    case id::A_FireMissile:
      obj->FireMissile();
      break;
    case id::A_FirePlasma:
      obj->FirePlasma();
      break;
    case id::A_FireBFG:
      obj->FireBFG();
      break;
    
    default:
      break;
  }
}

std::string WeaponFSM::GetSpriteName() const {
  auto base_name = GetSpriteBaseName();
  if (base_name.empty()) {
    assert(false);
    return base_name;
  }

  return base_name + "0";
}

} // namespace wpn
