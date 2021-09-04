#ifndef PLASMA_GUN_H_
#define PLASMA_GUN_H_

#include <string>
#include <vector>

#include "ammunition.h"
#include "projectile_params.h"

namespace wpn {

struct PlasmaGun {
  static std::string ready_sprite;
  static std::vector<std::pair<std::string, int>> fire_sprites;

  static bool GetAmmo(Ammo& am);
  static ProjectileParams GetShot() { return {0, 36, id::MT_PLASMA}; }

  static const int fire_again_idx = 1;

 private:
  static const int volley_ammo_number = 1;
};

} // namespace wpn

#endif  // PLASMA_GUN_H_