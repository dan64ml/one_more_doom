#ifndef WEAPON_H_
#define WEAPON_H_

#include <string>
#include <variant>
#include <memory>

#include "ammunition.h"
#include "weapon_types.h"
#include "weapon_fsm.h"
#include "flash_fsm.h"

#include "hitscan_params.h"

namespace mobj {
class Player;
}

namespace wpn {

class Weapon {
 public:
  Weapon(mobj::Player* player);

  bool TickTime();

  // Changes current weapon to new WeaponType
  void ChangeWeapon(WeaponType wp);
  // Change current weapon according to pressed key (hardcoded)
  void ChangeWeapon(char key);
  
  // Fire flag will be analyzed by weapon fsm
  void SetFireFlag(bool fire);
  
  // Can return empty string if underlying FSM is disabled
  std::string GetWeaponSpriteName() const { return weapon_fsm_.GetSpriteName(); }
  std::string GetFlashSpriteName() const { return flash_fsm_.GetSpriteName(); }

  int GetWeaponTopPosition() const { return current_weapon_top_; }

  // For status bar
  int GetCurrentAmmoNumber() const { return ammo_[weapons_[current_weapon_].ammo]; }
  bool IsWeaponAvailable(int keyboard_key) const;
  // Returns current/maximum amount
  std::pair<int, int> GetAmmo(AmmoType am) const;

 private:
  mobj::Player* player_;

  // Original values from id
  const int kLowerSpeed = 6;
  const int kRaiseSpeed = 6;
  const int kWeaponBottom = 128;
  const int kWeaponTop = 16;  // Original value is 32. Why?!!

  // Power for one BFG shot
  const int kBFGCharge = 40;

  WeaponFSM weapon_fsm_;
  FlashFSM flash_fsm_;

  int refire_count_ = 0;

  // Opened weapons
  bool has_weapon_[WeaponType::kWeaponNumber] {
    true,   // kFist
    true,   // kPistol
    true,   // kShotgun
    true,   // kChaingun
    true,   // kMissile
    true,   // kPlasma
    true,   // kBFG
    true,   // kChainsaw
    true,   // kSuperShotgun
  };
  // Ammunition
  int ammo_[kAmmoNumber] {
    200,    // kAmClip
    50,    // kAmShell
    300,    // kAmCell
    50     // kAmMisl
  };
  const int max_ammo_[kAmmoNumber] {
    200,    // kAmClip
    50,    // kAmShell
    300,    // kAmCell
    50     // kAmMisl
  };

  // Used to raise/lower a weapon
  int current_weapon_top_ = kWeaponBottom;

  // Reflects fire button state. Should be updated every tick.
  bool fire_flag_ = false;

  WeaponType current_weapon_;
  WeaponType pending_weapon_ = kNotPending;

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
  void FireChaingun();
  void Punch();
  void ChainSaw();

  void FireCurrentWeapon();

  // Check the ammo and change current weapon if there is not enough ammo
  bool CheckAmmo();
  
  const static WeaponParam weapons_[WeaponType::kWeaponNumber];

  friend class WeaponFSM;
};

} // namespace wpn

#endif  // WEAPON_H_