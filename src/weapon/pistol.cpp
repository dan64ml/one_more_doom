#include "pistol.h"

namespace wpn {

//std::string Pistol::ready_sprite = "PISGA0";
std::string Pistol::ready_sprite = "PISFA0";

std::vector<std::pair<std::string, int>> Pistol::fire_sprites = 
    {{"PISFA0", 3},
    {"PISGB0", 3},
    {"PISGC0", 3},
    {"PISGD0", 3},
    {"PISGE0", 3}};

bool Pistol::GetAmmo(Ammo& am) {
  if (am.bullets >= volley_ammo_number) {
    am.bullets -= volley_ammo_number;
    return true;
  }

  return false;
}

} // namespace wpn