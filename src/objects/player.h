#ifndef PLAYER_H_
#define PLAYER_H_

#include "map_object.h"
#include "renderer/bam.h"
#include "weapon/weapon.h"
#include "weapon/ammunition.h"

#include <string>

namespace mobj {

enum CardType {
  kBlueCard,
  kYellowCard,
  kRedCard,
  kBlueScull,
  kYellowScull,
  kRedScull,

  kCardsNumber
};

class Player : public MapObject {
 public:
  Player(const id::mobjinfo_t& info) : MapObject(info), weapon_(this) {}
  
  // Sets current button state
  void SetFireFlag(bool fire);

  void ChangeWeapon(char key) { weapon_.ChangeWeapon(key); }

  void Move(rend::BamAngle delta_angle, double forward_move, double side_move);

  bool TickTime();
 
  const wpn::Weapon& GetWeapon() const { return weapon_; }
  
  int GetArmor() const { return armor_; }
  bool IsCardPresent(CardType c) const { return cards_[c]; }

  std::string GetFaceSpriteName() const { return "STFST01"; }

  void FireMissile();
  void FirePlasma();

  void Punch();
  void ChainSaw();
  void FirePistol(bool refire);
  void FireShotgun();
  void FireSuperShotgun();

 private:
  virtual bool RunIntoAction() override;

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
};

} // namespace mobj

#endif  // PLAYER_H_