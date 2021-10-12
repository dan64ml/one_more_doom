#include "platform.h"

#include <cassert>

#include "sobj_types.h"
#include "sobj_utils.h"

#include "utils/world_utils.h"
#include "world/world.h"

namespace sobj {

Platform::Platform(world::World* w, world::Sector* s, world::Line* l, PlatformType type, int extra_height) 
  : world_(w), sector_(s), type_(type) {
  tag_ = l->tag;

  switch (type_)
  {
    case PlatformType::kRaiseToNearestAndChange:
      speed_ = kPlatformSpeed / 2;
      sector_->floor_pic = l->sides[0]->sector->floor_pic;
      high_pos_ = math::GetNextHighestFloorHeight(sector_, sector_->floor_height);
      wait_ = 0;
      state_ = MoveDirection::kUp;
      // Disable platform damage
      sector_->special = 0;
      break;
    case PlatformType::kRaiseAndChange:
      speed_ = kPlatformSpeed / 2;
      sector_->floor_pic = l->sides[0]->sector->floor_pic;
      high_pos_ = sector_->floor_height + extra_height;
      wait_ = 0;
      state_ = MoveDirection::kUp;
      break;
    case PlatformType::kDownWaitUpStay:
      speed_ = kPlatformSpeed * 4;
      low_pos_ = math::GetLowestFloorHeight(sector_);
      if (low_pos_ > sector_->floor_height) {
        low_pos_ = sector_->floor_height;
      }
      high_pos_ = sector_->floor_height;
      wait_ = kPlatformWait * 35;
      state_ = MoveDirection::kDown;
      break;
    case PlatformType::kBlazeDWUS:
      speed_ = kPlatformSpeed * 8;
      low_pos_ = math::GetLowestFloorHeight(sector_);
      if (low_pos_ > sector_->floor_height) {
        low_pos_ = sector_->floor_height;
      }
      high_pos_ = sector_->floor_height;
      wait_ = kPlatformWait * 35;
      state_ = MoveDirection::kDown;
      break;
    case PlatformType::kPerpetualRaise:
      speed_ = kPlatformSpeed;
      low_pos_ = math::GetLowestFloorHeight(sector_);
      if (low_pos_ > sector_->floor_height) {
        low_pos_ = sector_->floor_height;
      }
      high_pos_ = math::GetHighestFloorHeight(sector_);
      if (high_pos_ < sector_->floor_height) {
        high_pos_ = sector_->floor_height;
      }
      wait_ = kPlatformWait * 35;
      // tick_counter_ instead rand()
      state_ = (world_->tick_counter_ % 2) ? MoveDirection::kDown : MoveDirection::kUp;
      break;

    default:
      assert(false);
      break;
  }

  sector_->has_sobj = true;
}

Platform::~Platform() {
  sector_->has_sobj = false;
}

bool Platform::TickTime() {
  MoveResult res;

  switch (state_)
  {
    case MoveDirection::kUp:
      res = MoveFloor(sector_, speed_, high_pos_, crush_, MoveDirection::kUp);
      if (res == MoveResult::kCrushed && !crush_) {
        count_ = wait_;
        state_ = MoveDirection::kDown;
      } else if (res == MoveResult::kGotDest) {
        count_ = wait_;
        state_ = MoveDirection::kWait;
        switch (type_)
        {
          case PlatformType::kBlazeDWUS:
          case PlatformType::kDownWaitUpStay:
          case PlatformType::kRaiseAndChange:
          case PlatformType::kRaiseToNearestAndChange:
            return false;
        
          default:
            break;
        }
      }
      break;
    
    case MoveDirection::kDown:
      res = MoveFloor(sector_, speed_, low_pos_, false, state_);
      if (res == MoveResult::kGotDest) {
        count_ = wait_;
        state_ = MoveDirection::kWait;
      }
      break;
    
    case MoveDirection::kWait:
      if (!--count_) {
        if (abs(sector_->floor_height - low_pos_) < kEps) {
          state_ = MoveDirection::kUp;
        } else {
          state_ = MoveDirection::kDown;
        }
      }
      break;

    case MoveDirection::kInStasis:
      break;

    default:
      break;
  }

  return true;
}

void Platform::StopObject(int tag) {
  if (tag_ != tag || state_ == MoveDirection::kInStasis) {
    return;
  }

  old_state_ = state_;
  state_ = MoveDirection::kInStasis;
}

void Platform::ActivateInStasis(int tag) {
  if (tag_ != tag || state_ != MoveDirection::kInStasis) {
    return;
  }

  state_ = old_state_;
}

} // namespace sobj
