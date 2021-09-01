#include "plasma_gun.h"

namespace wpn {

std::string PlasmaGun::ready_sprite = "PLSGA0";

std::vector<std::pair<std::string, int>> PlasmaGun::fire_sprites = 
    {{"PLSFA0", 3},
    {"PLSFB0", 3},
    {"PLSGB0", 3}};

bool PlasmaGun::GetAmmo(Ammo& am) {
  if (am.bullets >= volley_ammo_number) {
    am.bullets -= volley_ammo_number;
    return true;
  }

  return false;
}

} // namespace wpn
