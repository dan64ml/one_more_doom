#ifndef PISTOL_H_
#define PISTOL_H_

#include <string>
#include <vector>

#include "ammunition.h"

namespace wpn {

struct Pistol {
  static std::string ready_sprite;
  static std::vector<std::pair<std::string, int>> fire_sprites;

  static bool GetAmmo(Ammo& am);

  static const int fire_again_idx = 1;

 private:
  static const int volley_ammo_number = 1;
};

} // namespace wpn

#endif  // PISTOL_H_
