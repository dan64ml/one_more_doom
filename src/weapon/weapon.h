#ifndef WEAPON_H_
#define WEAPON_H_

#include "weapon_state.h"

#include <string>
#include <variant>

#include "ammunition.h"

namespace wpn {

enum class WeaponType {
  kMelee,
  kPistol
};

class Weapon {
 public:
  Weapon();

  bool TickTime() { return state_->TickTime(this); }

  bool ChangeWeapon(WeaponType wp);
  
  // Depending on the active weapon and ammo Fire() can return false if there is no enough ammo,
  // true if ammo is ok but active weapon is not ready yet or Projectile/HitScan object if shot happened
  std::variant<bool, ProjectileParams, HitscanParams> Fire(Ammo& am) { return state_->Fire(am, this); }

  std::string GetSprite() const { return state_->GetSprite(); };

  void SetNewState(WeaponState* new_state) { state_ = new_state; }

 private:
  WeaponState* state_;
};

} // namespace wpn

#endif  // WEAPON_H_