#include "weapon.h"

#include <iostream>

#include "objects/info.h"

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

Weapon::Weapon() {
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

void Weapon::Fire() {
  fire_ = true;
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

void Weapon::Raise() {
  std::cout << "Raise" << std::endl;
  current_weapon_top_ -= kRaiseSpeed;
  if (current_weapon_top_ > kWeaponTop) {
    return;
  }

  current_weapon_top_ = kWeaponTop;

  weapon_fsm_.SetState(weapons_[current_weapon_].ready_state, this);
}

// Lowers current weapon and then starts raising of new weapon
void Weapon::Lower() {
  std::cout << "Lower" << std::endl;

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
  // Change current weapon
  if (pending_weapon_ != kNotPending) {
    fire_ = false;

    weapon_fsm_.SetState(weapons_[current_weapon_].down_state, this);

    return;
  }

  if (fire_) {
    fire_ = false;

    FireCurrentWeapon();
  }
}

void Weapon::ReFire() {
  std::cout << "ReFire" << std::endl;

}

void Weapon::FireShotgun2() {
  std::cout << "FireShotgun2" << std::endl;
  flash_fsm_.SetState(weapons_[current_weapon_].flash_state, this);
}

void Weapon::FireShotgun() {
  std::cout << "FireShotgun" << std::endl;
  flash_fsm_.SetState(weapons_[current_weapon_].flash_state, this);
}

void Weapon::FirePistol() {
  std::cout << "FirePistol" << std::endl;
  fire_ = false;
  flash_fsm_.SetState(weapons_[current_weapon_].flash_state, this);
}

void Weapon::FireMissile() {
  std::cout << "FireMissile" << std::endl;
  flash_fsm_.SetState(weapons_[current_weapon_].flash_state, this);
}

void Weapon::FirePlasma() {
  std::cout << "FirePlasma" << std::endl;
  flash_fsm_.SetState(weapons_[current_weapon_].flash_state, this);
}

void Weapon::FireBFG() {
  std::cout << "FireBFG" << std::endl;
  flash_fsm_.SetState(weapons_[current_weapon_].flash_state, this);
}

void Weapon::FireCurrentWeapon() {
  if (!CheckAmmo()) {
    return;
  }

  weapon_fsm_.SetState(weapons_[current_weapon_].active_state, this);
}

bool Weapon::CheckAmmo() {
  return true;
}

} // namespace wpn
