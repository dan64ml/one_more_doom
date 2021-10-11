#include "ceiling.h"

#include <cassert>

#include "utils/world_utils.h"

namespace sobj {

Ceiling::Ceiling(world::World* w, world::Sector* s, world::Line* l, CeilingType type) 
  : world_(w), sector_(s), type_(type) {
  switch (type_)
  {
    case CeilingType::kFastCrushAndRaise:
      crush_ = true;
      high_pos_ = sector_->ceiling_height;
      low_pos_ = sector_->floor_height + 8;
      direction_ = MoveDirection::kDown;
      speed_ = kCeilingSpeed * 2;
      break;
    
    case CeilingType::kSilentCrushAndRaise:
    case CeilingType::kCrushAndRaise:
      crush_ = true;
      high_pos_ = sector_->ceiling_height;
      [[fallthrough]];
    case CeilingType::kLowerAndCrush:
    case CeilingType::kLowerToFloor:
      low_pos_ = sector_->floor_height;
      if (type_ != CeilingType::kLowerToFloor) {
        low_pos_ += 8;
      }
      direction_ = MoveDirection::kDown;
      speed_ = kCeilingSpeed;
      break;

    case CeilingType::kRaiseToHighest:
      high_pos_ = math::GetHighestCeilingHeight(sector_);
      direction_ = MoveDirection::kUp;
      speed_ = kCeilingSpeed;
      break;

    default:
      assert(false);
      break;
  }

  sector_->has_sobj = true;
}

Ceiling::~Ceiling() {
  sector_->has_sobj = false;
}

bool Ceiling::TickTime() {
  int new_pos;

  switch (direction_)
  {
  case MoveDirection::kSuspend:
    break;

  case MoveDirection::kUp:
    new_pos = sector_->ceiling_height + speed_;
    if (new_pos > high_pos_) {
      new_pos = high_pos_;
    }

    break;
  
  default:
    break;
  }
}

} // namespace sobj
