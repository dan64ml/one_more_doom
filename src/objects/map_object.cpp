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

//MapObject::MapObject(world::World* world, const id::mobjinfo_t& info) : fsm_(info), world_(world) {
//  
//}
MapObject::MapObject(const id::mobjinfo_t& info) : fsm_(info) {
  flags = info.flags;
  height = info.height;
  radius = info.radius;
  speed = info.speed;
  
  spawn_health_ = health_ = info.spawnhealth;
  pain_chance_ = info.painchance;
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
  int x_dest;
  int y_dest;

  do {
    if (dx > kMaxMove / 2 || dy > kMaxMove / 2) {
      dx /= 2;
      dy /= 2;

      // TODO: controversial hack to avoid direction changing by very small values...
      x_dest = std::round(x + dx);
      y_dest = std::round(y + dy);
    } else {
      x_dest = std::round(x + dx);
      y_dest = std::round(y + dy);

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
bool MapObject::ProcessLine(const world::Line* line) {
  if (line->sides[1] == nullptr) {
    return false;
  }

  return true;
}

// There is two steps check: CheckPosition() checks obstacles and returns
// true if the position is clear. Also it updates lowest ceiling and 
// highest floor. It's necessary for second step - heights check.
// If both steps are OK, the foo changes current position of the mobs.
bool MapObject::TryMoveTo(int new_x, int new_y) {
  #ifdef D_PRINT_STEPS
    std::cout << "TryMoveTo: (" << x << ", " << y << ") => (" 
              << new_x << ", " << new_y << "): " << std::endl;
  #endif

  // check if the new position empty
  Opening op;
  if (!CheckPosition(new_x, new_y, op)) {
    return false;
  }

  // Check heights. Keep original behavior
  if (op.ceiling - op.floor < height) {
    // mobj too high
    return false;
  }
  if (!(flags & MF_TELEPORT) && (op.ceiling - z < height)) {
    // hit the ceiling by the head :)))
    return false;
  }
  if (!(flags & MF_TELEPORT) && (op.floor - z > kMaxStepSize)) {
    // The step too high
    return false;
  }
  if (!(flags & (MF_DROPOFF|MF_FLOAT)) && (op.floor - op.dropoff > height)) {
    // prevent falling
    return false;
  }

  // New position is OK, update coordinates
  floor_z = op.floor;
  ceiling_z = op.ceiling;

  // Rebind the mobj and change x and y
  world_->blocks_.MoveMapObject(this, new_x, new_y);
  x = new_x;
  y = new_y;
  ChangeSubSector(op.ss);

  // TODO: ??? Act special lines ???

  return true;
}

bool MapObject::CheckPosition(int new_x, int new_y, Opening& op) {
  int ss_idx = world_->bsp_.GetSubSectorIdx(new_x, new_y);
  auto* ss = &world_->sub_sectors_[ss_idx];

  op.floor = op.dropoff = ss->sector->floor_height;
  op.ceiling = ss->sector->ceiling_height;
  op.ss = ss;

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

    if (!ProcessLine(line)) {
      return false;
    }

    // Every line can change current opening. We are looking for the smallest
    UpdateOpening(op, line);
  }

  return true;
}

void MapObject::UpdateOpening(Opening& op, const world::Line* line) {
  if (line->sides[1] == nullptr) {
    // ProcessLine() should interrupt checking for a wall
    assert(false);
    return;
  }

  int fl = std::max(line->sides[0]->sector->floor_height, line->sides[1]->sector->floor_height);
  int low_fl = std::min(line->sides[0]->sector->floor_height, line->sides[1]->sector->floor_height);
  int ceil = std::min(line->sides[0]->sector->ceiling_height, line->sides[1]->sector->ceiling_height);

  if (ceil < op.ceiling) {
    op.ceiling = ceil;
  }
  if (fl > op.floor) {
    op.floor = fl;
  }
  if (low_fl < op.dropoff) {
    op.dropoff = low_fl;
  }
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

} // namespace mobj