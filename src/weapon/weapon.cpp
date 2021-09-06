#include "weapon.h"

#include "weapon_ready_state.h"

#include "pistol.h"
#include "plasma_gun.h"
#include "rocket_launcher.h"
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
  //state_ = &WeaponReadyState<Pistol>::GetInstance();
  //state_ = &WeaponReadyState<PlasmaGun>::GetInstance();
  //state_ = &WeaponReadyState<RocketLauncher>::GetInstance();

  //fsm_ = new WeaponFSM(weapons_[kMissile]);
  //state_ = new WeaponFSM(weapons_[kMissile]);
  state_ = new WeaponFSM(weapons_[kPlasma]);
}

} // namespace wpn

namespace id {
void A_WeaponReady() {
  return;
}

}

