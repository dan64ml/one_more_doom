#include "weapon.h"

#include <iostream>
#include <cstdlib>

#include "objects/info.h"
#include "objects/player.h"

namespace wpn {

const WeaponParam Weapon::weapons_[WeaponType::kWeaponNumber] = {
  {
	  // fist
	  kAmNoAmmo,
	  id::S_PUNCHUP,
	  id::S_PUNCHDOWN,
	  id::S_PUNCH,
	  id::S_PUNCH1,
	  id::S_NULL
  },	
  {
    // pistol
    kAmClip,
    id::S_PISTOLUP,
    id::S_PISTOLDOWN,
    id::S_PISTOL,
    id::S_PISTOL1,
    id::S_PISTOLFLASH
  },	
  {
    // shotgun
    kAmShell,
    id::S_SGUNUP,
    id::S_SGUNDOWN,
    id::S_SGUN,
    id::S_SGUN1,
    id::S_SGUNFLASH1
  },
  {
    // chaingun
    kAmClip,
    id::S_CHAINUP,
    id::S_CHAINDOWN,
    id::S_CHAIN,
    id::S_CHAIN1,
    id::S_CHAINFLASH1
  },
  {
    // missile launcher
    kAmMisl,
    id::S_MISSILEUP,
    id::S_MISSILEDOWN,
    id::S_MISSILE,
    id::S_MISSILE1,
    id::S_MISSILEFLASH1
  },
  {
    // plasma rifle
    kAmCell,
    id::S_PLASMAUP,
    id::S_PLASMADOWN,
    id::S_PLASMA,
    id::S_PLASMA1,
    id::S_PLASMAFLASH1
  },
  {
    // bfg 9000
    kAmCell,
    id::S_BFGUP,
    id::S_BFGDOWN,
    id::S_BFG,
    id::S_BFG1,
    id::S_BFGFLASH1
  },
  {
    // chainsaw
    kAmNoAmmo,
    id::S_SAWUP,
    id::S_SAWDOWN,
    id::S_SAW,
    id::S_SAW1,
    id::S_NULL
  },
  {
    // super shotgun
    kAmShell,
    id::S_DSGUNUP,
    id::S_DSGUNDOWN,
    id::S_DSGUN,
    id::S_DSGUN1,
    id::S_DSGUNFLASH1
  }
};

Weapon::Weapon(mobj::Player* player) : player_(player) {
  current_weapon_ = kPistol;
  //current_weapon_ = kShotgun;
  //current_weapon_ = kSuperShotgun;
  //current_weapon_ = kMissile;
  //current_weapon_ = kPlasma;
  //current_weapon_ = kBFG;
  
  //current_weapon_ = kChainsaw;

  //fsm_ = new WeaponFSM(weapons_[current_weapon_].ready_state);
  weapon_fsm_.SetState(weapons_[current_weapon_].up_state, this);
}

bool Weapon::TickTime() {
  weapon_fsm_.Tick(this);

  flash_fsm_.Tick(this);

  return true; 
}

void Weapon::SetFireFlag(bool fire) {
  fire_flag_ = fire;
}

void Weapon::ChangeWeapon(WeaponType wp) {
  if (wp == kNotPending || wp == current_weapon_) {
    return;
  }

  if (!has_weapon_[wp]) {
    return;
  }

  pending_weapon_ = wp;
}

void Weapon::ChangeWeapon(char key) {
  WeaponType new_weapon;

  switch (key)
  {
  case '1':
    if (has_weapon_[kChainsaw]) {
      new_weapon = kChainsaw;
    } else {
      new_weapon = kFist;
    }
    break;
  case '2':
    new_weapon = kPistol;
    break;
  case '3':
    if (has_weapon_[kSuperShotgun]) {
      new_weapon = kSuperShotgun;
    } else {
      new_weapon = kShotgun;
    }
    break;
  case '4':
    new_weapon = kChaingun;
    break;
  case '5':
    new_weapon = kMissile;
    break;
  case '6':
    new_weapon = kPlasma;
    break;
  case '7':
    new_weapon = kBFG;
    break;
  case '8':
    new_weapon = kFist;
    break;
  case '9':
    new_weapon = kShotgun;
    break;

  default:
    new_weapon = kNotPending;
    break;
  }

  ChangeWeapon(new_weapon);
}

bool Weapon::IsWeaponAvailable(int keyboard_key) const {
  switch (keyboard_key)
  {
    case 2:
      return has_weapon_[kPistol];
    case 3:
      return (has_weapon_[kShotgun] || has_weapon_[kSuperShotgun]);
    case 4:
      return has_weapon_[kChaingun];
    case 5:
      return has_weapon_[kMissile];
    case 6:
      return has_weapon_[kPlasma];
    case 7:
      return has_weapon_[kBFG];
    
    default:
      return false;
  }
}

std::pair<int, int> Weapon::GetAmmo(AmmoType am) const {
  return {ammo_[am], max_ammo_[am]};
}

void Weapon::Raise() {
  //std::cout << "Raise" << std::endl;
  current_weapon_top_ -= kRaiseSpeed;
  if (current_weapon_top_ > kWeaponTop) {
    return;
  }

  current_weapon_top_ = kWeaponTop;

  weapon_fsm_.SetState(weapons_[current_weapon_].ready_state, this);
}

// Lowers current weapon and then starts raising of new weapon
void Weapon::Lower() {
  //std::cout << "Lower" << std::endl;

  current_weapon_top_ += kLowerSpeed;
  if (current_weapon_top_ < kWeaponBottom) {
    return;
  }

  current_weapon_ = pending_weapon_;
  pending_weapon_ = kNotPending;

  weapon_fsm_.SetState(weapons_[current_weapon_].up_state, this);
}

// The place to fire or change current weapon
void Weapon::WeaponReady() {
  //std::cout << "WeaponReady" << std::endl;
  // Change current weapon
  if (pending_weapon_ != kNotPending) {
    fire_flag_ = false;

    weapon_fsm_.SetState(weapons_[current_weapon_].down_state, this);

    return;
  }

  if (fire_flag_) {
    FireCurrentWeapon();
  }
}

void Weapon::ReFire() {
  //std::cout << "ReFire" << std::endl;
  if (fire_flag_) {
    FireCurrentWeapon();
  }
}

void Weapon::FireShotgun2() {
  std::cout << "FireShotgun2" << std::endl;
  ammo_[weapons_[current_weapon_].ammo] -= 2;
  flash_fsm_.SetState(weapons_[current_weapon_].flash_state, this);
}

void Weapon::FireShotgun() {
  std::cout << "FireShotgun" << std::endl;
  ammo_[weapons_[current_weapon_].ammo]--;
  flash_fsm_.SetState(weapons_[current_weapon_].flash_state, this);
}

void Weapon::FirePistol() {
  //std::cout << "FirePistol" << std::endl;
  ammo_[weapons_[current_weapon_].ammo]--;
  flash_fsm_.SetState(weapons_[current_weapon_].flash_state, this);

  player_->FirePistol();
}

void Weapon::FireMissile() {
  std::cout << "FireMissile" << std::endl;
  ammo_[weapons_[current_weapon_].ammo]--;
  flash_fsm_.SetState(weapons_[current_weapon_].flash_state, this);

  player_->FireMissile();
}

void Weapon::FirePlasma() {
  //std::cout << "FirePlasma" << std::endl;
  ammo_[weapons_[current_weapon_].ammo]--;
  // Keep original behavior
  if (rand() % 2) {
    flash_fsm_.SetState(id::S_PLASMAFLASH1, this);
  } else {
    flash_fsm_.SetState(id::S_PLASMAFLASH2, this);
  }

  player_->FirePlasma();
}

void Weapon::FireBFG() {
  std::cout << "FireBFG" << std::endl;
  ammo_[weapons_[current_weapon_].ammo] -= kBFGCharge;
  flash_fsm_.SetState(weapons_[current_weapon_].flash_state, this);
}

void Weapon::FireChaingun() {
  std::cout << "FireBFG" << std::endl;
  ammo_[weapons_[current_weapon_].ammo]--;
  flash_fsm_.SetState(weapons_[current_weapon_].flash_state, this);
}

void Weapon::FireCurrentWeapon() {
  if (!CheckAmmo()) {
    return;
  }

  weapon_fsm_.SetState(weapons_[current_weapon_].active_state, this);
}

bool Weapon::CheckAmmo() {
  int ammo_num = 1;
  switch (current_weapon_)
  {
    case kBFG:
      ammo_num = kBFGCharge;
      break;
    case kSuperShotgun:
      ammo_num = 2;
      break;
    
    default:
      ammo_num = 1;
      break;
  }

  if (weapons_[current_weapon_].ammo == kAmNoAmmo 
      || ammo_[weapons_[current_weapon_].ammo] >= ammo_num) {
    return true;
  }

  // Select new weapon. Keep original behavior
  if (ammo_[kAmCell] && has_weapon_[kPlasma]) {
    pending_weapon_ = kPlasma;
  } else if (ammo_[kAmShell] > 2 && has_weapon_[kSuperShotgun]) {
    pending_weapon_ = kSuperShotgun;
  } else if (ammo_[kAmClip] && has_weapon_[kChaingun]) {
    pending_weapon_ = kChaingun;
  } else if (ammo_[kAmShell] && has_weapon_[kShotgun]) {
    pending_weapon_ = kShotgun;
  } else if (ammo_[kAmClip] && has_weapon_[kPistol]) {
    pending_weapon_ = kPistol;
  } else if (has_weapon_[kChainsaw]) {
    pending_weapon_ = kChainsaw;
  } else if (ammo_[kAmMisl] && has_weapon_[kMissile]) {
    pending_weapon_ = kMissile;
  } else if (ammo_[kAmCell] > kBFGCharge && has_weapon_[kBFG]) {
    pending_weapon_ = kBFG;
  } else {
    pending_weapon_ = kFist;
  }

  weapon_fsm_.SetState(weapons_[current_weapon_].down_state, this);

  return false;
}

} // namespace wpn
