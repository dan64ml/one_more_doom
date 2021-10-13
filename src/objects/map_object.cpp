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
      if (!RunIntoAction()) {
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
bool MapObject::RunIntoAction() {
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

  // check if the new position empty
  if (!CheckPosition(new_x, new_y)) {
    return false;
  }

  // Check heights. Keep original behavior
  if (tmp_ceiling - tmp_floor < height) {
    // mobj too high
    return false;
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

  ChangeSubSector(ss);

  return true;
}

bool MapObject::CheckPosition(double new_x, double new_y) {
  int ss_idx = world_->bsp_.GetSubSectorIdx(new_x, new_y);
  auto* ss = &world_->sub_sectors_[ss_idx];

  // Get initial values from destination point
  tmp_floor = tmp_dropoff = ss->sector->floor_height;
  tmp_ceiling = ss->sector->ceiling_height;

  // Possible area where collision of mobjs can happen
  int dist = kMaxRadius + radius;
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

    int collision_dist = radius + mobj->radius;
    if (abs(new_x - mobj->x) >= collision_dist || abs(new_y - mobj->y) >= collision_dist) {
      // Didn't hit
      continue;
    }
    if (mobj == this) {
      continue;
    }

    if (!InfluenceObject(mobj)) {
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
      return false;
    }

    if (!line->sides[1]) {
      // Wall
      return false;
    }
    if (!(flags & MF_MISSILE)) {
      // Check for bloking line flags
      if (line->flags & world::kLDFBlockEveryOne) {
        return false;
      }
      if ((mobj_type != id::MT_PLAYER) && (line->flags & world::kLDFBlockMonsters)) {
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
  auto it = std::find(begin(ss->mobjs), end(ss->mobjs), this);
  if (it == end(ss->mobjs)) {
    return false;
  }

  ss->mobjs.erase(it);

  new_ss->mobjs.push_back(this);

  ss = new_ss;

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

} // namespace mobj