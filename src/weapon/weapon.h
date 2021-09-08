#ifndef WEAPON_H_
#define WEAPON_H_

#include <string>
#include <variant>
#include <memory>

#include "ammunition.h"
#include "weapon_types.h"
#include "weapon_fsm.h"
#include "flash_fsm.h"

#include "projectile_params.h"
#include "hitscan_params.h"

namespace wpn {

class Weapon {
 public:
  Weapon();

  bool TickTime();

  bool ChangeWeapon(WeaponType wp);
  
  // Depending on the active weapon and ammo Fire() can return false if there is no enough ammo,
  // true if ammo is ok but active weapon is not ready yet or Projectile/HitScan object if shot happened
  //std::variant<bool, ProjectileParams, HitscanParams> Fire(Ammo& am) { return true; }
  std::variant<bool, ProjectileParams, HitscanParams> Fire(Ammo& am);

  //std::string GetSprite() const { return state_->GetSprite(); };
  std::string GetSprite() const { return fsm_->GetSpriteName(); }
  std::string GetEffectSprite() const;
  int GetWeaponTopPosition() const { return current_weapon_top_; }

  //void SetNewState(WeaponState* new_state) { state_ = new_state; }
  //void SetNewState(WeaponState* new_state) {  }

 private:
  WeaponFSM* fsm_;
  std::unique_ptr<FlashFSM> effect_fsm_;

  int current_weapon_top_ = 0;
  bool fire_ = false;
  int change_weapon_ = 0;

  WeaponType current_weapon_;

  void Raise();
  void Lower();
  void WeaponReady();
  void ReFire();
  void FireShotgun2();
  void FireShotgun();
  void FirePistol();
  void FireMissile();
  void FirePlasma();
  void FireBFG();

  const static WeaponParam weapons_[WeaponType::kWeaponNumber];

  // Original values from id
  const int kLowerSpeed = 6;
  const int kRaiseSpeed = 6;
  const int kWeaponBottom = 128;
  const int kWeaponTop = 32;

  friend class WeaponFSM;
};

} // namespace wpn

#endif  // WEAPON_H_