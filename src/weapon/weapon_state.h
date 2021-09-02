#ifndef WEAPON_STATE_H_
#define WEAPON_STATE_H_

#include <string>
#include <variant>

#include "ammunition.h"
#include "projectile_params.h"
#include "hitscan_params.h"

namespace wpn {

class Weapon;

class WeaponState {
 public:
  virtual bool TickTime(Weapon* w) = 0;

  virtual std::variant<bool, ProjectileParams, HitscanParams> Fire(Ammo&, Weapon* w) = 0;

  virtual std::string GetSprite() const = 0;
};

} // namespace wpn

#endif  // WEAPON_STATE_H_