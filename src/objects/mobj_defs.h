#ifndef MOBJ_DEFS_H_
#define MOBJ_DEFS_H

namespace mobj {

const int kMaxRadius = 32;
const double kGravity = 1.0;

// Distance for missile attack
const int kMissileRange = 32 * 64;
// Distance for melee attack
const int kMeleeRange = 64;

const int kWeaponHeight = 42; // Why??!

// Max passible step height
const int kMaxStepSize = 24;

const double kMaxMove = 30;

// Flying mobj's vertical speed;
const int kFloatSpeed = 4;

// To compare double values
const double kEps = 0.0001;

const int kRocketBlastDamage = 128;

const int kBarrelHeight = 32; // original value

enum CardType {
  kBlueCard,
  kYellowCard,
  kRedCard,
  kBlueScull,
  kYellowScull,
  kRedScull,

  kCardsNumber
};

} // namespace mobj

#endif  // MOBJ_DEFS_H
