#ifndef ROCKET_LAUNCHER_H_
#define ROCKET_LAUNCHER_H_

#include <string>
#include <vector>

#include "ammunition.h"
#include "projectile_params.h"

namespace wpn {

struct RocketLauncher {
  static std::string ready_sprite;
  static std::vector<std::pair<std::string, int>> fire_sprites;

  static bool GetAmmo(Ammo& am);
  static ProjectileParams GetShot() {
    return {0,    // blast damage
            36,   // spawn height
            id::MT_ROCKET
    }; 
  }

  static const int fire_again_idx = 1;

 private:
  static const int volley_ammo_number = 1;
};

} // namespace wpn

#endif  // ROCKET_LAUNCHER_H_