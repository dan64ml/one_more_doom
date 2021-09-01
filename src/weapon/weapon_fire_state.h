#ifndef WEAPON_FIRE_STATE_H_
#define WEAPON_FIRE_STATE_H_

#include "weapon.h"
//#include "weapon_ready_state.h"

namespace wpn {

template<class T>
class WeaponReadyState;

template<class Arms>
class WeaponFireState : public WeaponState {
 public:
  std::variant<bool, Projectile, HitScan> Fire(Ammo& am, Weapon* w) override {
    if (sprite_idx_ < Arms::fire_again_idx) {
      return true;
    } else if (Arms::GetAmmo(am)) {
      w->SetNewState(&WeaponFireState<Arms>::GetInstance());
      return Projectile(Arms());
    } else {
      return false;
    }
  }

  std::string GetSprite() const override { return Arms::fire_sprites[sprite_idx_].first; }

  bool TickTime(Weapon* w) override {
    if (frame_count_ == 0) {
      if (sprite_idx_ + 1 < Arms::fire_sprites.size()) {
        ++sprite_idx_;
        frame_count_ = Arms::fire_sprites[sprite_idx_].second;
      } else {
        w->SetNewState(&WeaponReadyState<Arms>::GetInstance());
      }
    } else {
      --frame_count_;
    }

    return true;
  }

  static WeaponFireState& GetInstance() {
    static WeaponFireState singleton;
    singleton.sprite_idx_ = 0;
    singleton.frame_count_ = Arms::fire_sprites[0].second;
    return singleton;
  }

 private:
  int sprite_idx_;
  int frame_count_;
};

} // namespace wpn

#endif  // WEAPON_FIRE_STATE_H_
