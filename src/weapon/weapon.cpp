#include "weapon.h"

#include "weapon_ready_state.h"

#include "pistol.h"
#include "plasma_gun.h"
#include "rocket_launcher.h"

namespace wpn {

bool Weapon::ChangeWeapon(WeaponType wp) {
  switch (wp)
  {
  case WeaponType::kMelee:
    /* code */
    break;
  
  case WeaponType::kPistol:
    state_ = &WeaponReadyState<Pistol>::GetInstance();
    break;
  
  default:
    break;
  }

  return true;
}

Weapon::Weapon() {
  //state_ = &WeaponReadyState<Pistol>::GetInstance();
  //state_ = &WeaponReadyState<PlasmaGun>::GetInstance();
  state_ = &WeaponReadyState<RocketLauncher>::GetInstance();
}

} // namespace wpn
