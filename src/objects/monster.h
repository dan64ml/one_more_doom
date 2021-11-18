#ifndef MONSTER_H_
#define MONSTER_H_

#include "map_object.h"

#include "sounds.h"

namespace mobj {

class Monster : public MapObject {
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
  void CallStateFunction(id::FuncId foo_id) override;

 private:
  // ORIGIN: if > 0 the target will be chased
  int threshold_;
  // ORIGIN: if non 0, don't attack yet.
  int reaction_time_;

  // The target for chasing
  MapObject* target_ = nullptr;

  // Monsters move in a zig-zag way
  ZZDir move_dir_;
  // When 0, select a new direction
  int move_count_;

  id::sfxenum_t see_sound_;
  id::sfxenum_t attack_sound_;
  id::sfxenum_t pain_sound_;
  id::sfxenum_t death_sound_;
  id::sfxenum_t active_sound_;

 private:
  void A_Look();
  void A_Chase();
  void A_FaceTarget();

  // Temporary stub for sound subsystem
  void PlaySound([[maybe_unused]] id::sfxenum_t sound) {}

  rend::BamAngle ZZDirToBam(ZZDir dir);
  // Looks for new move direction. Messy original algorithm
  void NewChaseDirection();
  // Checks if melee attack exists and the distance is ok
  bool CheckMeleeAttack();
  // Checks missile attack existence, move_count_ and distance
  bool CheckMissileAttack();
  // Check if monster is ready to distance attack
  bool CheckMissileRange();
  // Check the distance and obstacles
  bool CheckMeleeRange();
  // Attempts to move it the direction, if ok set random move_count_
  bool TryWalk(ZZDir dir);
  // Tries to move according to zigzag params and to use special lines
  bool ZZMove();

};

} // namespace mobj

#endif  // MONSTER_H_
