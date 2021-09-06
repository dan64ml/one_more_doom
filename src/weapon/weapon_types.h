#ifndef WEAPON_TYPES_H_
#define WEAPON_TYPES_H_

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

  kWeaponNumber
};

struct WeaponParam {
  int up_state;
  int down_state;
  int ready_state;
  int active_state;
  int flash_state;
};

} // namespace wpn

#endif  // WEAPON_TYPES_H_
