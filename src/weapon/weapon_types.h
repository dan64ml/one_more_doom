#ifndef WEAPON_TYPES_H_
#define WEAPON_TYPES_H_

#include "objects/info.h"

namespace wpn {

enum WeaponType {
  kFist = 0,
  kPistol,
  kShotgun,
  kChaingun,
  kMissile,
  kPlasma,
  kBFG,
  kChainsaw,
  kSuperShotgun,

  kWeaponNumber,

  kNotPending   // Special flag for weapon changing
};

enum AmmoType {
  kAmClip,
  kAmShell,
  kAmCell,
  kAmMisl,

  kAmmoNumber,

  kAmNoAmmo
};

struct WeaponParam {
  AmmoType ammo;
  id::statenum_t up_state;
  id::statenum_t down_state;
  id::statenum_t ready_state;
  id::statenum_t active_state;
  id::statenum_t flash_state;
};

} // namespace wpn

#endif  // WEAPON_TYPES_H_
