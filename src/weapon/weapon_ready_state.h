#ifndef WEAPON_READY_STATE_H_
#define WEAPON_READY_STATE_H_

#include "weapon_state.h"
#include "weapon_fire_state.h"
#include "weapon.h"

namespace wpn {

//template<class T>
//class WeaponFireState;

template<class Arms>
class WeaponReadyState : public WeaponState {
 public:
  std::variant<bool, Projectile, HitScan> Fire(Ammo& am, Weapon* w) override { 
    if (Arms::GetAmmo(am)) {
      w->SetNewState(&WeaponFireState<Arms>::GetInstance());
      return Projectile(Arms());
    } else {
      return false;
    }
  }

  std::string GetSprite() const override { return Arms::ready_sprite; }

  bool TickTime(Weapon* w) override { return true; }

  static WeaponReadyState& GetInstance() {
    static WeaponReadyState singleton;
    return singleton;
  }

 private:
  WeaponReadyState() = default;
  WeaponReadyState(const WeaponReadyState&) = default;
  WeaponReadyState& operator=(const WeaponReadyState&);
};

} // namespace wpn

#endif  // WEAPON_READY_STATE_H_
