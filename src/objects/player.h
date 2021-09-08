#ifndef PLAYER_H_
#define PLAYER_H_

#include "map_object.h"
#include "renderer/bam.h"
#include "weapon/weapon.h"
#include "weapon/ammunition.h"

#include <string>

namespace mobj {

class Player : public MapObject {
 public:
  Player(const id::mobjinfo_t& info) : MapObject(info) {}
  
  void Fire();
  void ChangeWeapon(char key) { weapon_.ChangeWeapon(key); }

  void Move(rend::BamAngle delta_angle, double forward_move, double side_move);

  bool TickTime();
 
  //std::string GetWeaponSprite() const { return weapon_.GetSprite(); }
  const wpn::Weapon& GetWeapon() const { return weapon_; }

 private:
  virtual bool RunIntoAction() override;

 private:
  wpn::Weapon weapon_;
  wpn::Ammo ammo_;
};

} // namespace mobj

#endif  // PLAYER_H_