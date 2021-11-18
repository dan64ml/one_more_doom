#include "map_object.h"

#include <cassert>
#include <algorithm>
#include <iostream>
#include <cmath>

#include "world/world.h"
#include "utils/world_utils.h"
#include "renderer/plane_utils.h"

#define DEBUG_CODE

#ifdef DEBUG_CODE
  #define D_PRINT_STEPS
  #define D_PRINT_LINES
  #define D_PRINT_MOBJS
#endif

namespace mobj {

MapObject::MapObject(id::mobjtype_t type) : fsm_(id::mobjinfo[type]) {
  assert(type < id::NUMMOBJTYPES);
  mobj_type = type;

  flags = id::mobjinfo[type].flags;
  height = id::mobjinfo[type].height;
  radius = id::mobjinfo[type].radius;
  speed = id::mobjinfo[type].speed;

  spawn_health_ = health_ = id::mobjinfo[type].spawnhealth;
  pain_chance_ = id::mobjinfo[type].painchance;
}

std::string MapObject::GetSpriteName(int vp_x, int vp_y) const {
  auto vp_angle = rend::CalcAngle(x, y, vp_x, vp_y);

  rend::BamAngle da = (vp_angle - (rend::BamAngle)(angle - rend::kBamAngle45 / 2));
  int sector = da / rend::kBamAngle45;
  ++sector;
  if (sector > 8) {
    sector -= 8;
  }

  return fsm_.GetSpriteName() + std::to_string(sector);
}

bool MapObject::TickTime() {
  if (delete_me_) {
    return false;
  }

  // TODO: should I use return value??? E.g. for deleting the mobj?
  fsm_.Tick(this);
  
  MoveObject();
  return true;
}

void MapObject::MoveObject() {
  XYMove();
  ZMove();
}

void MapObject::XYMove() {
  if (!mom_x && ! mom_y) {
    return;
  }

  // Keep original behavior
  if (mom_x > kMaxMove) {
    mom_x = kMaxMove;
  } else if (mom_x < -kMaxMove) {
    mom_x = -kMaxMove;
  }

  if (mom_y > kMaxMove) {
    mom_y = kMaxMove;
  } else if (mom_y < -kMaxMove) {
    mom_y = -kMaxMove;
  }
  
  double dx = mom_x;
  double dy = mom_y;
  double x_dest;
  double y_dest;

  do {
    if (dx > kMaxMove / 2 || dy > kMaxMove / 2) {
      dx /= 2;
      dy /= 2;
      x_dest = x + dx;
      y_dest = y + dy;
    } else {
      x_dest = x + dx;
      y_dest = y + dy;
      dx = dy = 0;
    }

    if (!TryMoveTo(x_dest, y_dest)) {
      if (!RunIntoAction(x_dest, y_dest)) {
        // TODO: return or break!???
        return;
      }
    }
  } while (dx || dy);

  SlowDown();
}

void MapObject::ZMove() {
  // Flying objects have another behavior !!!
  z += mom_z;

  if (z <= floor_z) {
    z = floor_z;

    if (mom_z < 0) {
      mom_z = 0;
    } 
  } else if (!(flags & MF_NOGRAVITY)) {
    if (mom_z == 0) {
      mom_z = -kGravity;
    } else {
      mom_z -= kGravity;
    }
  }
}

// By default - do nothing and continue
bool MapObject::RunIntoAction(double new_x, double new_y) {
  return true;
}

// By defaul - stop the object
void MapObject::SlowDown() {
  mom_x = mom_y = 0;
}

// By default - find walls and ignore portals
bool MapObject::ProcessLine(world::Line* line) {
  if (line->sides[1] == nullptr) {
    return false;
  }

  return true;
}

// There are two steps check: CheckPosition() checks obstacles and returns
// true if the position is clear. Also it updates lowest ceiling and 
// highest floor. It's necessary for second step - heights check.
// If both steps are OK, the foo changes current position of the mobs.
bool MapObject::TryMoveTo(double new_x, double new_y) {
  #ifdef D_PRINT_STEPS
    std::cout << "TryMoveTo: (" << x << ", " << y << ") => (" 
              << new_x << ", " << new_y << "): " << std::endl;
  #endif

  float_ok_ = false;
  
  // check if the new position empty
  if (!CheckPosition(new_x, new_y)) {
    return false;
  }

  // Check heights. Keep original behavior
  if (tmp_ceiling - tmp_floor < height) {
    // mobj too high
    return false;
  } else {
    float_ok_ = true;
  }

  if (!(flags & MF_TELEPORT) && (tmp_ceiling - z < height)) {
    // hit the ceiling by the head :)))
    return false;
  }
  if (!(flags & MF_TELEPORT) && (tmp_floor - z > kMaxStepSize)) {
    // The step too high
    return false;
  }
  if (!(flags & (MF_DROPOFF|MF_FLOAT)) && (tmp_floor - tmp_dropoff > height)) {
    // prevent falling
    return false;
  }

  // New position is OK, trigger special lines and update coordinates
  for (auto line : spec_lines_) {
    auto old_pos = math::LinePointPosition(line, x, y);
    auto new_pos = math::LinePointPosition(line, new_x, new_y);
    if (old_pos != new_pos) {
      ProcessSpecialLine(line);
    }
  }

  // Rebind the mobj and change x and y
  floor_z = tmp_floor;
  ceiling_z = tmp_ceiling;

  world_->blocks_.MoveMapObject(this, new_x, new_y);
  x = new_x;
  y = new_y;

  ChangeSubSector(tmp_ss);

  return true;
}

bool MapObject::CheckPosition(double new_x, double new_y) {
  int ss_idx = world_->bsp_.GetSubSectorIdx(new_x, new_y);
  auto* ss = &world_->sub_sectors_[ss_idx];

  // Get initial values from destination point
  tmp_floor = tmp_dropoff = ss->sector->floor_height;
  tmp_ceiling = ss->sector->ceiling_height;

  line_obstacle_ = nullptr;
  mobj_obstacle_ = nullptr;

  // Possible area where collision of mobjs can happen
  double dist = kMaxRadius + radius;
  world::BBox bbox;
  bbox.left = new_x - dist;
  bbox.right = new_x + dist;
  bbox.top = new_y + dist;
  bbox.bottom = new_y - dist;

  for (auto mobj : world_->blocks_.GetMapObjects(bbox)) {
    #ifdef D_PRINT_MOBJS
      std::cout << "Checking mobj (" << mobj->x << ", " << mobj->y << ")" << std::endl;
    #endif
    if (!(mobj->flags & (MF_SOLID | MF_SPECIAL | MF_SHOOTABLE))) {
      // This object don't interact with anything (???)
      continue;
    }

    double collision_dist = radius + mobj->radius;
    if (std::abs(new_x - mobj->x) >= collision_dist || std::abs(new_y - mobj->y) >= collision_dist) {
      // Didn't hit
      continue;
    }
    if (mobj == this) {
      continue;
    }

    if (!InfluenceObject(mobj)) {
      mobj_obstacle_ = mobj;
      return false;
    }
  }

  // Possible area where collision of mobj and line can happen
  bbox.left = new_x - radius;
  bbox.right = new_x + radius;
  bbox.top = new_y + radius;
  bbox.bottom = new_y - radius;

  spec_lines_.resize(0);

  for (auto line : world_->blocks_.GetLines(bbox)) {
    #ifdef D_PRINT_LINES
      std::cout << "Checking line (" << line->x1 << ", " << line->y1 << ") -> ("
                << line->x2 << ", " << line->y2 << "): ";
    #endif
    // Check bboxes
    if (line->bbox.left > bbox.right || bbox.left > line->bbox.right ||
        line->bbox.top < bbox.bottom || line->bbox.bottom > bbox.top) {
      #ifdef D_PRINT_LINES
        std::cout << "don't cross" << std::endl;
      #endif
      continue;
    }

    if (math::LineBBoxPosition(line, &bbox) != math::kCross) {
      // Didn't cross
      #ifdef D_PRINT_LINES
        std::cout << "don't cross" << std::endl;
      #endif
      continue;
    }
    #ifdef D_PRINT_LINES
      std::cout << "cross" << std::endl;
    #endif

    // TMP!!!!!!!!!!!!!!!!!!!!
    if (!ProcessLine(line)) {
      line_obstacle_ = line;
      return false;
    }

    if (!line->sides[1]) {
      // Wall
      line_obstacle_ = line;
      return false;
    }
    if (!(flags & MF_MISSILE)) {
      // Check for bloking line flags
      if (line->flags & world::kLDFBlockEveryOne) {
        line_obstacle_ = line;
        return false;
      }
      if ((mobj_type != id::MT_PLAYER) && (line->flags & world::kLDFBlockMonsters)) {
        line_obstacle_ = line;
        return false;
      }
    }

    // Every line can change current opening. We are looking for the smallest
    UpdateOpening(line);

    // Collect special lines
    if (line->specials) {
      spec_lines_.push_back(line);
    }
  }

  tmp_ss = ss;
  return true;
}

void MapObject::UpdateOpening(const world::Line* line) {
  double fl = std::max(line->sides[0]->sector->floor_height, line->sides[1]->sector->floor_height);
  double low_fl = std::min(line->sides[0]->sector->floor_height, line->sides[1]->sector->floor_height);
  double ceil = std::min(line->sides[0]->sector->ceiling_height, line->sides[1]->sector->ceiling_height);

  tmp_ceiling = std::min(tmp_ceiling, ceil);
  tmp_floor = std::max(tmp_floor, fl);
  tmp_dropoff = std::min(tmp_dropoff, low_fl);
}

bool MapObject::ChangeSubSector(world::SubSector* new_ss) {
  auto it = std::find(begin(ss_->mobjs), end(ss_->mobjs), this);
  if (it == end(ss_->mobjs)) {
    return false;
  }

  ss_->mobjs.erase(it);

  new_ss->mobjs.push_back(this);

  ss_ = new_ss;

  return true;
}

void MapObject::CauseDamage(int damage) {
  health_ -= damage;

  if (health_ > 0) {
    if (rand() % 256 < pain_chance_) {
      fsm_.ToPainState(this);
    }
    return;
  } else {
    if (health_ < -spawn_health_) {
      fsm_.ToXDeathState(this);
    } else {
      fsm_.ToDeathState(this);
    }

    // remains are visible but not interactable
    // TODO: flying objects must fall down
    flags &= ~(MF_SOLID | MF_SHOOTABLE);
  }
}

// TODO: there is duplicate code with CheckPosition()
void MapObject::UpdateOpening() {
  int ss_idx = world_->bsp_.GetSubSectorIdx(x, y);
  auto* ss = &world_->sub_sectors_[ss_idx];

  tmp_floor = tmp_dropoff = ss->sector->floor_height;
  tmp_ceiling = ss->sector->ceiling_height;
  
  // Possible area where collision of mobj and line can happen
  world::BBox bbox;
  bbox.left = x - radius;
  bbox.right = x + radius;
  bbox.top = y + radius;
  bbox.bottom = y - radius;

  for (auto line : world_->blocks_.GetLines(bbox)) {
    #ifdef D_PRINT_LINES
      std::cout << "Checking line (" << line->x1 << ", " << line->y1 << ") -> ("
                << line->x2 << ", " << line->y2 << "): ";
    #endif
    // Check bboxes
    if (line->bbox.left > bbox.right || bbox.left > line->bbox.right ||
        line->bbox.top < bbox.bottom || line->bbox.bottom > bbox.top) {
      #ifdef D_PRINT_LINES
        std::cout << "don't cross" << std::endl;
      #endif
      continue;
    }

    if (math::LineBBoxPosition(line, &bbox) != math::kCross) {
      // Didn't cross
      #ifdef D_PRINT_LINES
        std::cout << "don't cross" << std::endl;
      #endif
      continue;
    }
    #ifdef D_PRINT_LINES
      std::cout << "cross" << std::endl;
    #endif

    if (line->sides[1] == nullptr) {
      break;
    }

    // Every line can change current opening. We are looking for the smallest
    UpdateOpening(line);
  }

  ceiling_z = tmp_ceiling;
  floor_z = tmp_floor;
}

void MapObject::DamageBySobj(int damage) {
  if (health_ <= 0) {
    // Get giblets
    fsm_.ToGibsState(this);
    flags &= ~mobj::MF_SOLID;
    height = 0;
    radius = 0;
    return;
  }

  if (flags & mobj::MF_DROPPED) {
    // Should disapear
    delete_me_ = true;
  }

  if (!(flags & mobj::MF_SHOOTABLE)) {
    // Can't interact
    return;
  }

  CauseDamage(damage);
}

/*rend::BamAngle MapObject::ZZDirToBam(ZZDir dir) {
  using namespace rend;

  switch (dir)
  {
  case ZZDir::kEast:
    return kBamAngle0;
  case ZZDir::kNorthEast:
    return kBamAngle45;
  case ZZDir::kNorth:
    return kBamAngle90;
  case ZZDir::kNorthWest:
    return kBamAngle135;
  case ZZDir::kWest:
    return kBamAngle180;

  case ZZDir::kSouthWest:
    return kBamAngle180 + kBamAngle45;
  case ZZDir::kSouth:
    return kBamAngle180 + kBamAngle90;
  case ZZDir::kSouthEast:
    return kBamAngle180 + kBamAngle135;
  
  default:
    return kBamAngle0;
  }
}

bool MapObject::TryWalk(ZZDir dir) {
  move_dir_ = dir;
  if (!ZZMove()) {
    return false;
  }

  move_count_ = rand() & 15;

  return true;
}

// Keep original behavior !!! ******************************************************
ZZDir opposite[] = {
  ZZDir::kWest, ZZDir::kSouthWest, ZZDir::kSouth, ZZDir::kSouthEast,
  ZZDir::kEast, ZZDir::kNorthEast, ZZDir::kNorth, ZZDir::kNorthWest, ZZDir::kNoDir
};

ZZDir diags[] = {
  ZZDir::kNorthWest, ZZDir::kNorthEast, ZZDir::kSouthWest, ZZDir::kSouthEast
};*/

/*void MapObject::NewChaseDirection() {
  assert(target_);

  auto old_dir = move_dir_;
  auto turn_around = opposite[static_cast<int>(old_dir)];

  auto dx = target_->x - x;
  auto dy = target_->y - y;

  ZZDir d1, d2;
  
  if (dx > 10) {
    d1 = ZZDir::kEast;
  } else if (dx < -10) {
    d1 = ZZDir::kWest;
  } else {
    d1 = ZZDir::kNoDir;
  }

  if (dy < -10) {
    d2 = ZZDir::kSouth;
  } else if (dy > 10) {
    d2 = ZZDir::kNorth;
  } else {
    d2 = ZZDir::kNoDir;
  }

  // Try to move directly
  if (d1 != ZZDir::kNoDir && d2 != ZZDir::kNoDir) {
    // Funny and tricky :)
    int dir_idx = ((dy < 0) << 1) + (dx > 0);
    if (diags[dir_idx] != turn_around && TryWalk(diags[dir_idx])) {
      return;
    }
  }

  // Try another directions
  if (rand() % 255 > 200 ||
      std::abs(dy) > std::abs(dx)) {
    std::swap(d1, d2);
  }

  if (d1 == turn_around) {
    d1 = ZZDir::kNoDir;
  }
  if (d2 == turn_around) {
    d2 = ZZDir::kNoDir;
  }

  if (d1 != ZZDir::kNoDir) {
    if (TryWalk(d1)) {
      return;
    }
  }
  if (d2 != ZZDir::kNoDir) {
    if (TryWalk(d2)) {
      return;
    }
  }

  // There is no direct path to the player...
  if (old_dir != ZZDir::kNoDir) {
    if (TryWalk(old_dir)) {
      return;
    }
  }

  // The worst case, random direction
  if (rand() & 1) {
    for (int dir = static_cast<int>(ZZDir::kEast); dir <= static_cast<int>(ZZDir::kSouthEast); ++dir) {
      auto cand_dir = static_cast<ZZDir>(dir);
      if (cand_dir != turn_around) {
        if (TryWalk(cand_dir)) {
          return;
        }
      }
    }
  } else {
    for (int dir = static_cast<int>(ZZDir::kSouthEast); dir != static_cast<int>(ZZDir::kEast) - 1; --dir) {
      auto cand_dir = static_cast<ZZDir>(dir);
      if (cand_dir != turn_around) {
        if (TryWalk(cand_dir)) {
          return;
        }
      }
    }
  }

  // The last chance
  if (turn_around != ZZDir::kNoDir) {
    if (TryWalk(old_dir)) {
      return;
    }
  }

  move_dir_ = ZZDir::kNoDir;
}

bool MapObject::ZZMove() {
  if (move_dir_ == ZZDir::kNoDir) {
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
    return false;
  } else {
    // TODO: What is it ???????
    flags &= ~ mobj::MF_INFLOAT;
  }

  if (!(flags & mobj::MF_FLOAT)) {
    z = floor_z;
  }

  return true;
}*/

} // namespace mobj