#ifndef MONSTER_H_
#define MONSTER_H_

#include "map_object.h"
#include "moving_object.h"

#include "sounds.h"

namespace mobj {

class Monster : public MovingObject {
 private:
  // Mobj's zig-zag direction
  enum ZZDir {
    kEast,
    kNorthEast,
    kNorth,
    kNorthWest,
    kWest,
    kSouthWest,
    kSouth,
    kSouthEast,
    kNoDir
  };

  const std::vector<ZZDir> opposite = {
    kWest, kSouthWest, kSouth, kSouthEast,
    kEast, kNorthEast, kNorth, kNorthWest, kNoDir
  };

  const std::vector<ZZDir> diags = {
    kNorthWest, kNorthEast, kSouthWest, kSouthEast
  };

 public:
  Monster(id::mobjtype_t type);

 protected:
  // FSM's functions. Some of them are used directly by derived code.
  // Some functions are common for all monsters, they are defined in
  // this class.
  // Some functions are unique for a specific monster, they should be
  // overridden in derived classes.
  void A_FaceTarget();
  // Zombieman attack
  virtual void A_PosAttack() {}
  // Cacodemon attack
  virtual void A_HeadAttack() {}
  // Imp attack
  virtual void A_TroopAttack() {}
  // Mancubus specific
  virtual void A_FatRaise() {}
  virtual void A_FatAttack1() {}
  virtual void A_FatAttack2() {}
  virtual void A_FatAttack3() {}
  // Shotguy
  virtual void A_SPosAttack() {}
  // ChainGuy
  virtual void A_CPosAttack() {}
  virtual void A_CPosRefire() {}
  // Demon and Spectre
  virtual void A_SargAttack() {}

  // Check if monster is ready to distance attack
  bool CheckMissileRange();
  // Check the distance and obstacles
  bool CheckMeleeRange();

  void CallStateFunction(id::FuncId foo_id) override;

  // Temporary stub for sound subsystem
  void PlaySound([[maybe_unused]] id::sfxenum_t sound) {}

 private:
  // Monsters move in a zig-zag way
  ZZDir move_dir_ = kNoDir;
  // When 0, select a new direction
  int move_count_ = 0;

  id::sfxenum_t see_sound_;
  id::sfxenum_t attack_sound_;
  id::sfxenum_t pain_sound_;
  id::sfxenum_t death_sound_;
  id::sfxenum_t active_sound_;

 private:
  // FSM's common functions
  void A_Look();
  void A_Chase();
  void A_Fall();
  void A_BossDeath();

  rend::BamAngle ZZDirToBam(ZZDir dir);
  // Looks for new move direction. Messy original algorithm
  void NewChaseDirection();
  // Checks if melee attack exists and the distance is ok
  bool CheckMeleeAttack();
  // Checks missile attack existence, move_count_ and distance
  bool CheckMissileAttack();
  // Attempts to move it the direction, if ok set random move_count_
  bool TryWalk(ZZDir dir);
  // Tries to move according to zigzag params and to use special lines
  bool ZZMove();

};

} // namespace mobj

#endif  // MONSTER_H_
