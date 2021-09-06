#ifndef WEAPON_H_
#define WEAPON_H_

#include "weapon_state.h"

#include <string>
#include <variant>

#include "ammunition.h"
#include "weapon_types.h"
#include "weapon_fsm.h"

namespace wpn {

class Weapon {
 public:
  Weapon();

  //bool TickTime() { return state_->TickTime(this); }
  bool TickTime() { return state_->Tick(); }

  bool ChangeWeapon(WeaponType wp);
  
  // Depending on the active weapon and ammo Fire() can return false if there is no enough ammo,
  // true if ammo is ok but active weapon is not ready yet or Projectile/HitScan object if shot happened
  //std::variant<bool, ProjectileParams, HitscanParams> Fire(Ammo& am) { return state_->Fire(am, this); }
  std::variant<bool, ProjectileParams, HitscanParams> Fire(Ammo& am) { return true; }

  //std::string GetSprite() const { return state_->GetSprite(); };
  std::string GetSprite() const { return state_->GetSpriteName(); };

  //void SetNewState(WeaponState* new_state) { state_ = new_state; }
  void SetNewState(WeaponState* new_state) {  }

 private:
  //WeaponState* state_;
  //WeaponFSM* fsm_;
  WeaponFSM* state_;

  const static WeaponParam weapons_[WeaponType::kWeaponNumber];
};

} // namespace wpn

#endif  // WEAPON_H_