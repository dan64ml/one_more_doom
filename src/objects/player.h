#ifndef PLAYER_H_
#define PLAYER_H_

#include "map_object.h"
#include "renderer/bam.h"
#include "weapon/weapon.h"
#include "weapon/ammunition.h"
#include "world/world_types.h"

#include <string>

namespace mobj {

// Max distance to button or door to get it used
const int kUseDistance = 48;

class Player : public MapObject {
 public:
  Player() : MapObject(id::MT_PLAYER), weapon_(this) { health_ = 999; }
  
  // Sets current button state
  void SetFireFlag(bool fire);
  void SetUseFlag() { use_command_ = true; }

  void ChangeWeapon(char key) { weapon_.ChangeWeapon(key); }

  void Move(rend::BamAngle delta_angle, double forward_move, double side_move);

  bool TickTime();
 
  const wpn::Weapon& GetWeapon() const { return weapon_; }
  
  int GetArmor() const { return armor_; }
  bool IsCardPresent(CardType c) const override { return cards_[c]; }

  std::string GetFaceSpriteName() const { return "STFST01"; }

  void FireMissile();
  void FirePlasma();

  void Punch();
  void ChainSaw();
  void FirePistol(bool refire);
  void FireShotgun();
  void FireSuperShotgun();
  void FireBFG();

 private:
  virtual bool RunIntoAction(double new_x, double new_y) override;

  void ProcessSpecialLine(world::Line* line) override;

  world::Line* FindSpecialLine();

  world::IntersectedObject GetClosestObstacle(double new_x, double new_y);
 private:
  wpn::Weapon weapon_;
  wpn::Ammo ammo_;

  int armor_ = 42;

  bool cards_[kCardsNumber] = {
    true,
    true,
    false,
    false,
    true,
    true
  };

  // Use command flag
  bool use_command_ = false;
};

} // namespace mobj

#endif  // PLAYER_H_