#include "weapon.h"

#include <iostream>

#include "weapon_ready_state.h"

#include "objects/info.h"

namespace wpn {

const WeaponParam Weapon::weapons_[WeaponType::kWeaponNumber] = {
  {
	  // fist
	  //am_noammo,
	  id::S_PUNCHUP,
	  id::S_PUNCHDOWN,
	  id::S_PUNCH,
	  id::S_PUNCH1,
	  id::S_NULL
  },	
  {
    // pistol
    //am_clip,
    id::S_PISTOLUP,
    id::S_PISTOLDOWN,
    id::S_PISTOL,
    id::S_PISTOL1,
    id::S_PISTOLFLASH
  },	
  {
    // shotgun
    //am_shell,
    id::S_SGUNUP,
    id::S_SGUNDOWN,
    id::S_SGUN,
    id::S_SGUN1,
    id::S_SGUNFLASH1
  },
  {
    // chaingun
    //am_clip,
    id::S_CHAINUP,
    id::S_CHAINDOWN,
    id::S_CHAIN,
    id::S_CHAIN1,
    id::S_CHAINFLASH1
  },
  {
    // missile launcher
    //am_misl,
    id::S_MISSILEUP,
    id::S_MISSILEDOWN,
    id::S_MISSILE,
    id::S_MISSILE1,
    id::S_MISSILEFLASH1
  },
  {
    // plasma rifle
    //am_cell,
    id::S_PLASMAUP,
    id::S_PLASMADOWN,
    id::S_PLASMA,
    id::S_PLASMA1,
    id::S_PLASMAFLASH1
  },
  {
    // bfg 9000
    //am_cell,
    id::S_BFGUP,
    id::S_BFGDOWN,
    id::S_BFG,
    id::S_BFG1,
    id::S_BFGFLASH1
  },
  {
    // chainsaw
    //am_noammo,
    id::S_SAWUP,
    id::S_SAWDOWN,
    id::S_SAW,
    id::S_SAW1,
    id::S_NULL
  },
  {
    // super shotgun
    //am_shell,
    id::S_DSGUNUP,
    id::S_DSGUNDOWN,
    id::S_DSGUN,
    id::S_DSGUN1,
    id::S_DSGUNFLASH1
  }
};

bool Weapon::ChangeWeapon(WeaponType wp) {
  switch (wp)
  {
  case WeaponType::kFist:
    /* code */
    break;
  
  case WeaponType::kPistol:
    //state_ = &WeaponReadyState<Pistol>::GetInstance();
    break;
  
  default:
    break;
  }

  return true;
}

Weapon::Weapon() {
  //fsm_ = new WeaponFSM(weapons_[kMissile]);
  //fsm_ = new WeaponFSM(weapons_[kPlasma]);
  //fsm_ = new WeaponFSM(weapons_[kSuperShotgun]);
  //fsm_ = new NewWeaponFSM(weapons_[kShotgun]);
  //fsm_ = new WeaponFSM(weapons_[kPistol]);
  //fsm_ = new WeaponFSM(weapons_[kBFG]);
  //fsm_ = new WeaponFSM(weapons_[kChainsaw]);

  //current_weapon_ = kPistol;
  //current_weapon_ = kShotgun;
  current_weapon_ = kSuperShotgun;
  //current_weapon_ = kMissile;
  //current_weapon_ = kPlasma;
  //current_weapon_ = kBFG;
  
  //current_weapon_ = kChainsaw;

  fsm_ = new WeaponFSM(static_cast<id::statenum_t>(weapons_[current_weapon_].ready_state));
}

std::string Weapon::GetEffectSprite() const {
  if (effect_fsm_) {
    return effect_fsm_->GetSpriteName();
  } else {
    return "";
  }
}

bool Weapon::TickTime() {
  //std::cout << " <==\n";
  /*for (auto func : fsm_->Tick()) {
    switch (func) {
    case id::A_NULL:
      break;
    case id::A_WeaponReady:
      WeaponReady();
      break;
    case id::A_Lower:
      Lower();
      break;
    case id::A_Raise:
      Raise();
      break;
    case id::A_ReFire:
      ReFire();
      break;
    case id::A_FireShotgun2:
      FireShotgun2();
      break;
    case id::A_FireShotgun:
      FireShotgun();
      break;
    case id::A_FirePistol:
      FirePistol();
      break;
    case id::A_FireMissile:
      FireMissile();
      break;
    default:
      break;
    }
  }*/

  fsm_->Tick(this);
  if (effect_fsm_) {
    effect_fsm_->Tick(this);
  }

  /*if (effect_fsm_) {
    std::vector<id::FuncId> commands;
    auto ret = effect_fsm_->Tick(commands);
    if (!ret) {
      effect_fsm_.release();
    }
  }*/

  return true; 
}

void Weapon::Raise() {
  std::cout << "Raise" << std::endl;
  current_weapon_top_ += kRaiseSpeed;
  if (current_weapon_top_ < kWeaponTop) {
    return;
  }

  current_weapon_top_ = kWeaponTop;

  //fsm_->ToReadyState();
  fsm_->SetState(static_cast<id::statenum_t>(weapons_[current_weapon_].ready_state), this);
}

void Weapon::Lower() {
  std::cout << "Lower" << std::endl;

}

void Weapon::WeaponReady() {
  std::cout << "WeaponReady" << std::endl;

}

void Weapon::ReFire() {
  std::cout << "ReFire" << std::endl;

}

void Weapon::FireShotgun2() {
  std::cout << "FireShotgun2" << std::endl;
  //effect_fsm_.reset(new EffectFSM(id::S_DSGUNFLASH1));
  effect_fsm_.reset(new FlashFSM(static_cast<id::statenum_t>(weapons_[current_weapon_].flash_state)));
  //fsm_->ToFlashState();
}

void Weapon::FireShotgun() {
  std::cout << "FireShotgun" << std::endl;
  effect_fsm_.reset(new FlashFSM(static_cast<id::statenum_t>(weapons_[current_weapon_].flash_state)));
//  effect_fsm_.reset(new EffectFSM(id::S_SGUNFLASH1));
  //fsm_->ToFlashState();
}

void Weapon::FirePistol() {
  std::cout << "FirePistol" << std::endl;
  effect_fsm_.reset(new FlashFSM(static_cast<id::statenum_t>(weapons_[current_weapon_].flash_state)));
  //effect_fsm_.reset(new EffectFSM(id::S_PISTOLFLASH));
  //fsm_->ToFlashState();
}

void Weapon::FireMissile() {
  effect_fsm_.reset(new FlashFSM(static_cast<id::statenum_t>(weapons_[current_weapon_].flash_state)));
  //effect_fsm_.reset(new EffectFSM(id::S_MISSILEFLASH1));
  //fsm_->ToFlashState();
}

void Weapon::FirePlasma() {
  effect_fsm_.reset(new FlashFSM(static_cast<id::statenum_t>(weapons_[current_weapon_].flash_state)));
}

void Weapon::FireBFG() {
  effect_fsm_.reset(new FlashFSM(static_cast<id::statenum_t>(weapons_[current_weapon_].flash_state)));
}


std::variant<bool, ProjectileParams, HitscanParams> Weapon::Fire(Ammo& am) {
  //fsm_->ToActiveState();
  fsm_->SetState(static_cast<id::statenum_t>(weapons_[current_weapon_].active_state), this);
  return true;
}

} // namespace wpn
