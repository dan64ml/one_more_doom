#include "ceiling.h"

#include <cassert>

#include "sobj_utils.h"
#include "utils/world_utils.h"

namespace sobj {

Ceiling::Ceiling(world::World* w, world::Sector* s, world::Line* l, CeilingType type) 
  : world_(w), sector_(s), type_(type) {
  tag_ = l->tag;
  
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
  MoveResult res;

  switch (direction_)
  {
    case MoveDirection::kInStasis:
      break;

    case MoveDirection::kUp:
      res = MoveCeiling(sector_, speed_, high_pos_, false, direction_);
      if (res == MoveResult::kGotDest) {
        switch (type_)
        {
          case CeilingType::kRaiseToHighest:
            return false;
          
          case CeilingType::kSilentCrushAndRaise:
          case CeilingType::kFastCrushAndRaise:
          case CeilingType::kCrushAndRaise:
            direction_ = MoveDirection::kDown;
            break;
          
          default:
            break;
        }
      }
      break;

    case MoveDirection::kDown:
      res = MoveCeiling(sector_, speed_, low_pos_, crush_, direction_);
      if (res == MoveResult::kGotDest) {
        switch (type_)
        {
          case CeilingType::kSilentCrushAndRaise:
          case CeilingType::kCrushAndRaise:
            speed_ = kCeilingSpeed;
            [[fallthrough]];
          case CeilingType::kFastCrushAndRaise:
            direction_ = MoveDirection::kUp;
            return true;
  
          case CeilingType::kLowerAndCrush:
          case CeilingType::kLowerToFloor:
            return false;
          
          default:
            break;
        }
      } else {
        if (res == MoveResult::kCrushed) {
          switch (type_)
          {
            case CeilingType::kSilentCrushAndRaise:
            case CeilingType::kCrushAndRaise:
            case CeilingType::kLowerAndCrush:
              speed_ = kCeilingSpeed / 8;
              break;
            
            default:
              break;
          }
        }
      }
      break;
  
    default:
      break;
  }

  return true;
}

void Ceiling::StopObject(int tag) {
  if (tag_ != tag || direction_ == MoveDirection::kInStasis) {
    return;
  }

  old_direction_ = direction_;
  direction_ = MoveDirection::kInStasis;
}

void Ceiling::ActivateInStasis(int tag) {
  if (tag_ != tag || direction_ != MoveDirection::kInStasis) {
    return;
  }

  direction_ = old_direction_;
}

} // namespace sobj
