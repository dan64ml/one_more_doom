#include "moving_object.h"

#include <iostream>

#include "world/world_types.h"
#include "world/world.h"
#include "utils/world_utils.h"

#define DEBUG_CODE

#ifdef DEBUG_CODE
  #define D_PRINT_STEPS
  #define D_PRINT_MOBJS
  #define D_PRINT_LINES
#endif

namespace mobj {

MovingObject::MovingObject(id::mobjtype_t type) : MapObject(type) {

}

bool MovingObject::TickTime() {
  if (delete_me_) {
    return false;
  }

  fsm_.Tick(this);
  
  MoveObject();
  return true;
}

void MovingObject::MoveObject() {
  XYMove();
  ZMove();
}

void MovingObject::XYMove() {
  if (!mom_x && !mom_y) {
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

void MovingObject::ZMove() {
  // Flying objects have another behavior !!!
  z += mom_z;

  if (z <= floor_z_) {
    z = floor_z_;

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
bool MovingObject::RunIntoAction([[maybe_unused]] double new_x, [[maybe_unused]] double new_y) {
  return true;
}

// By defaul - stop the object
void MovingObject::SlowDown() {
  if (z > floor_z_) {
    return;
  }

  // Original behavior
  if (flags & MF_CORPSE) {
    if (mom_x > 0.25 || mom_x < -0.25 ||
        mom_y > 0.25 || mom_y < -.025) {
      if (std::abs(floor_z_ - ss_->sector->floor_height) > kEps) {
        return;
      }
    }
  }

  if (mom_x > -kStopSpeed && mom_x < kStopSpeed &&
      mom_y > -kStopSpeed && mom_y < kStopSpeed) {
    mom_x = mom_y = 0;
  } else {
    mom_x *= kFriction;
    mom_y *= kFriction;
  }
}

// By default - find walls and ignore portals
bool MovingObject::ProcessLine(world::Line* line) {
  if (line->sides[1] == nullptr) {
    return false;
  }

  return true;
}

// There are two steps check: CheckPosition() checks obstacles and returns
// true if the position is clear. Also it updates lowest ceiling and 
// highest floor. It's necessary for second step - heights check.
// If both steps are OK, the foo changes current position of the mobs.
bool MovingObject::TryMoveTo(double new_x, double new_y) {
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
  floor_z_ = tmp_floor;
  ceiling_z_ = tmp_ceiling;

  world_->blocks_.MoveMapObject(this, new_x, new_y);
  x = new_x;
  y = new_y;

  ChangeSubSector(x, y);

  return true;
}

bool MovingObject::CheckPosition(double new_x, double new_y) {
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

  return true;
}

void MovingObject::UpdateOpening(const world::Line* line) {
  double fl = std::max(line->sides[0]->sector->floor_height, line->sides[1]->sector->floor_height);
  double low_fl = std::min(line->sides[0]->sector->floor_height, line->sides[1]->sector->floor_height);
  double ceil = std::min(line->sides[0]->sector->ceiling_height, line->sides[1]->sector->ceiling_height);

  tmp_ceiling = std::min(tmp_ceiling, ceil);
  tmp_floor = std::max(tmp_floor, fl);
  tmp_dropoff = std::min(tmp_dropoff, low_fl);
}

// TODO: there is duplicate code with CheckPosition()
void MovingObject::UpdateOpening() {
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

  ceiling_z_ = tmp_ceiling;
  floor_z_ = tmp_floor;
}

bool MovingObject::AdjustMobjHeight() {
  bool is_on_floor = (abs(z - floor_z_) < kEps);
  UpdateOpening();

  if (is_on_floor) {
    z = floor_z_;
  } else {
    if (z + height > ceiling_z_) {
      z = ceiling_z_ - height;
    }
  }

  return (ceiling_z_ - floor_z_ < height) ? false : true;
}

} // namespace mobj
