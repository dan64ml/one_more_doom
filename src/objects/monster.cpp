#include "monster.h"

#include <cassert>

#include "renderer/plane_utils.h"
#include "world/world_types.h"
#include "world/world.h"

namespace mobj {

Monster::Monster(id::mobjtype_t type) : MovingObject(type) {
  reaction_time_ = id::mobjinfo[type].reactiontime;

  see_sound_ = static_cast<id::sfxenum_t>(id::mobjinfo[type].seesound);
  attack_sound_ = static_cast<id::sfxenum_t>(id::mobjinfo[type].attacksound);
  pain_sound_ = static_cast<id::sfxenum_t>(id::mobjinfo[type].painsound);
  death_sound_ = static_cast<id::sfxenum_t>(id::mobjinfo[type].deathsound);
  active_sound_ = static_cast<id::sfxenum_t>(id::mobjinfo[type].activesound);

}

void Monster::CallStateFunction(id::FuncId foo_id) {
  switch (foo_id)
  {
    case id::A_Look:
      A_Look();
      break;
    case id::A_Chase:
      A_Chase();
      break;
    case id::A_Fall:
      A_Fall();
      break;
    case id::A_FaceTarget:
      A_FaceTarget();
      break;
    case id::A_PosAttack:
      A_PosAttack();
      break;
    case id::A_HeadAttack:
      A_HeadAttack();
      break;
    case id::A_TroopAttack:
      A_TroopAttack();
      break;
    case id::A_FatRaise:
      A_FatRaise();
      break;
    case id::A_FatAttack1:
      A_FatAttack1();
      break;
    case id::A_FatAttack2:
      A_FatAttack2();
      break;
    case id::A_FatAttack3:
      A_FatAttack3();
      break;
    case id::A_SPosAttack:
      A_SPosAttack();
      break;
    case id::A_CPosAttack:
      A_CPosAttack();
      break;
    case id::A_CPosRefire:
      A_CPosRefire();
      break;
    case id::A_SargAttack:
      A_SargAttack();
      break;
    
    default:
      break;
  }
}

void Monster::A_Look() {
  threshold_ = 0;

  auto noise_src = ss_->sector->sound_source;
  if (noise_src && (noise_src->flags & MF_SHOOTABLE)) {
    // Monster in ambush, does'n move to sound source, waits for visible contact
    if (flags & MF_AMBUSH) {
      if (world_->CheckSight(this, noise_src)) {
        target_ = noise_src;
      } else {
        target_ = nullptr;
      }
    }
  }

  if (!target_ && !(target_ = world_->LookForPlayer(this, false))) {
    return;
  }

  PlaySound(see_sound_);

  fsm_.ToSeeState(this);
}

void Monster::A_Fall() {
  flags &= ~MF_SOLID;
}

void Monster::A_Chase() {
  // Original behavior
  if (reaction_time_) {
    --reaction_time_;
  }

  if (threshold_) {
    if (!target_ || target_->GetHealth() <= 0) {
      threshold_ = 0;
    } else {
      --threshold_;
    }
  }

  // Turn towards movement direction
  if (move_dir_ < kNoDir) {
    angle = ZZDirToBam(move_dir_);
  }

  if (!target_ || !(target_->flags & MF_SHOOTABLE)) {
    // Lost the target
    target_ = world_->LookForPlayer(this, true);
    if (!target_) {
      fsm_.ToSpawnState(this);
    }

    return;
  }

  // Keep original behavior, don't attack twice in a row
  if (flags & mobj::MF_JUSTATTACKED) {
    flags &= ~mobj::MF_JUSTATTACKED;
    NewChaseDirection();
    return;
  }

  // Check for melee attack
  if (CheckMeleeAttack()) {
    PlaySound(attack_sound_);

    fsm_.ToMeleeState(this);
    return;
  }

  // Check for missile attack
  if (CheckMissileAttack()) {
    fsm_.ToMissileState(this);
    flags |= mobj::MF_JUSTATTACKED;
    return;
  }

  if (!threshold_ && !world_->CheckSight(this, target_)) {
    auto new_target = world_->LookForPlayer(this, true);
    if (new_target) {
      target_ = new_target;
      return;
    }
  }

  // Movement
  if (--move_count_ < 0 || !ZZMove()) {
    NewChaseDirection();
  }

  if (rand() % 256 < 3) {
    PlaySound(active_sound_);
  }
}

void Monster::A_FaceTarget() {
  if (!target_) {
    return;
  }

  flags &= ~MF_AMBUSH;

  angle = rend::CalcAngle(x, y, target_->x, target_->y);

  // Blur the angle for invisible goals
  if (target_->flags & MF_SHADOW) {
    // Is the angle right???
    angle += (rand() - rand()) % (255 * 32);
  }
}

void Monster::A_BossDeath() {
  world_->BossDeath(mobj_type);
}

rend::BamAngle Monster::ZZDirToBam(ZZDir dir) {
  using namespace rend;

  switch (dir)
  {
  case kEast:
    return kBamAngle0;
  case kNorthEast:
    return kBamAngle45;
  case kNorth:
    return kBamAngle90;
  case kNorthWest:
    return kBamAngle135;
  case kWest:
    return kBamAngle180;

  case kSouthWest:
    return kBamAngle180 + kBamAngle45;
  case kSouth:
    return kBamAngle180 + kBamAngle90;
  case kSouthEast:
    return kBamAngle180 + kBamAngle135;
  
  default:
    return kBamAngle0;
  }
}

void Monster::NewChaseDirection() {
  assert(target_);

  auto old_dir = move_dir_;
  auto turn_around = opposite[old_dir];

  auto dx = target_->x - x;
  auto dy = target_->y - y;

  ZZDir d1, d2;
  
  if (dx > 10) {
    d1 = kEast;
  } else if (dx < -10) {
    d1 = kWest;
  } else {
    d1 = kNoDir;
  }

  if (dy < -10) {
    d2 = kSouth;
  } else if (dy > 10) {
    d2 = kNorth;
  } else {
    d2 = kNoDir;
  }

  // Try to move directly
  if (d1 != kNoDir && d2 != kNoDir) {
    // Funny and tricky :)
    int dir_idx = ((dy < 0) << 1) + (dx > 0);
    if (diags[dir_idx] != turn_around && TryWalk(diags[dir_idx])) {
      return;
    }
  }

  // Try other directions
  if (rand() % 255 > 200 ||
      std::abs(dy) > std::abs(dx)) {
    std::swap(d1, d2);
  }

  if (d1 == turn_around) {
    d1 = kNoDir;
  }
  if (d2 == turn_around) {
    d2 = kNoDir;
  }

  if (d1 != kNoDir) {
    if (TryWalk(d1)) {
      return;
    }
  }
  if (d2 != kNoDir) {
    if (TryWalk(d2)) {
      return;
    }
  }

  // There is no direct path to the player...
  if (old_dir != kNoDir) {
    if (TryWalk(old_dir)) {
      return;
    }
  }

  // The worst case, random direction
  if (rand() & 1) {
    for (int dir = kEast; dir <= kSouthEast; ++dir) {
      if (dir != turn_around) {
        if (TryWalk(static_cast<ZZDir>(dir))) {
          return;
        }
      }
    }
  } else {
    for (int dir = kSouthEast; dir != kEast - 1; --dir) {
      if (dir != turn_around) {
        if (TryWalk(static_cast<ZZDir>(dir))) {
          return;
        }
      }
    }
  }

  // The last chance
  if (turn_around != kNoDir) {
    if (TryWalk(old_dir)) {
      return;
    }
  }

  move_dir_ = kNoDir;
}

bool Monster::CheckMeleeAttack() {
//  if (!fsm_.IsMeleeState()) {
  if (!fsm_.HasState(FsmState::kMelee)) {
    return false;
  }

  return CheckMeleeRange();
}

bool Monster::CheckMissileAttack() {
//  if (!fsm_.IsMissileState()) {
  if (!fsm_.HasState(FsmState::kMissile)) {
    return false;
  }
  if (move_count_) {
    return false;
  }

  return CheckMissileRange();
}

bool Monster::CheckMeleeRange() {
  if (!target_) {
    return false;
  }

  double dist = rend::SegmentLength(x, y, target_->x, target_->y);
  if (dist > kMeleeRange - 20 + target_->radius) {
    return false;
  }

  return world_->CheckSight(this, target_);
}

bool Monster::CheckMissileRange() {
  if (!world_->CheckSight(this, target_)) {
    return false;
  }

  if (flags & MF_JUSTHIT) {
    // The monster has just been attacked
    flags &= ~MF_JUSTHIT;
    return true;
  }

  if (reaction_time_) {
    // Too early to attack
    return false;
  }

  double dist = rend::SegmentLength(x, y, target_->x, target_->y) - 64;
//  if (!fsm_.IsMeleeState()) {
  if (!fsm_.HasState(FsmState::kMelee)) {
    dist -= 128;
  }

  // Different monsters have different distance
  if (mobj_type == id::MT_VILE) {
    if (dist > 14 * 64) {
      return false;
    }
  }

  if (mobj_type == id::MT_UNDEAD) {
    if (dist < 196) {
      return false;
    }

    dist /= 2;
  }

  if (mobj_type == id::MT_CYBORG ||
      mobj_type == id::MT_SPIDER ||
      mobj_type == id::MT_SKULL) {
        dist /= 2;
  }

  dist = std::min(dist, 200.0);

  if (mobj_type == id::MT_CYBORG && dist > 160) {
    dist = 160;
  }

  if (rand() % 256 < dist) {
    return false;
  }

  return true;
}

bool Monster::TryWalk(ZZDir dir) {
  move_dir_ = dir;
  if (!ZZMove()) {
    return false;
  }

  move_count_ = rand() & 15;

  return true;
}

bool Monster::ZZMove() {
  if (move_dir_ == kNoDir) {
    return false;
  }

  auto move_angle = ZZDirToBam(move_dir_);
  double new_x = x + speed * rend::BamCos(move_angle);
  double new_y = y + speed * rend::BamSin(move_angle);

  if (!TryMoveTo(new_x, new_y)) {
    // It's impossible to get the new position, but there are several options why not

    // Unfitted height, but the monster can fly
    if ((flags & mobj::MF_FLOAT) && float_ok_) {
      if (z < tmp_floor) {
        z += kFloatSpeed;
      } else {
        z -= kFloatSpeed;
      }

      flags |= mobj::MF_INFLOAT;
      return true;
    }

    // The monster met a door, TODO!!!!!
    // Check special lines and try to open!!!!!! p_enemy.c/P_Move()
    return false;
  } else {
    flags &= ~ mobj::MF_INFLOAT;
  }

  if (!(flags & mobj::MF_FLOAT)) {
    z = floor_z_;
  }

  return true;
}

} // namespace mobj
