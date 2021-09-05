#include "rocket_launcher.h"

namespace wpn {

std::string RocketLauncher::ready_sprite = "MISFD0";

std::vector<std::pair<std::string, int>> RocketLauncher::fire_sprites = 
    {{"PLSFA0", 3},
    {"PLSFB0", 3},
    {"PLSGB0", 3}};

bool RocketLauncher::GetAmmo(Ammo& am) {
  if (am.bullets >= volley_ammo_number) {
    am.bullets -= volley_ammo_number;
    return true;
  }

  return false;
}

} // namespace wpn
