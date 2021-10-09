#include "platform.h"

#include <cassert>

#include "utils/world_utils.h"
#include "world/world.h"

namespace sobj {

Platform::Platform(world::World* w, world::Sector* s, world::Line* l, PlatformType type, int extra_height) 
  : world_(w), sector_(s), type_(type) {
  switch (type_)
  {
    case PlatformType::kRaiseToNearestAndChange:
    // TODO: origin speed_ = 0.5, double requires :(
      //speed_ = kPlatformSpeed / 2;
      speed_ = kPlatformSpeed;
      sector_->floor_pic = l->sides[0]->sector->floor_pic;
      high_pos_ = math::GetNextHighestFloorHeight(sector_, sector_->floor_height);
      wait_count_ = 0;
      state_ = PlatformState::kUp;
      // Disable platform damage
      sector_->special = 0;
      break;
    case PlatformType::kRaiseAndChange:
      //speed_ = kPlatformSpeed / 2;
      speed_ = kPlatformSpeed ;
      sector_->floor_pic = l->sides[0]->sector->floor_pic;
      high_pos_ = sector_->floor_height + extra_height;
      wait_count_ = 0;
      state_ = PlatformState::kUp;
      break;
    case PlatformType::kDownWaitUpStay:
      speed_ = kPlatformSpeed * 4;
      low_pos_ = math::GetLowestFloorHeight(sector_);
      if (low_pos_ > sector_->floor_height) {
        low_pos_ = sector_->floor_height;
      }
      high_pos_ = sector_->floor_height;
      wait_count_ = kPlatformWait * 35;
      state_ = PlatformState::kDown;
      break;
    case PlatformType::kBlazeDWUS:
      speed_ = kPlatformSpeed * 8;
      low_pos_ = math::GetLowestFloorHeight(sector_);
      if (low_pos_ > sector_->floor_height) {
        low_pos_ = sector_->floor_height;
      }
      high_pos_ = sector_->floor_height;
      wait_count_ = kPlatformWait * 35;
      state_ = PlatformState::kDown;
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
      wait_count_ = kPlatformWait * 35;
      // tick_counter_ instead rand()
      state_ = (world_->tick_counter_ % 2) ? PlatformState::kDown : PlatformState::kUp;
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
  int new_pos = 0;

  switch (state_)
  {
    case PlatformState::kUp:
      new_pos = sector_->floor_height + speed_;
      if (new_pos > high_pos_) {
        new_pos = high_pos_;
      }

      if (world_->TryToChangeSectorHeight(sector_, new_pos, sector_->ceiling_height, false)) {
        sector_->floor_height = new_pos;
        if (new_pos == high_pos_) {
          if (type_ == PlatformType::kPerpetualRaise) {
            count_ = wait_count_;
            state_ = PlatformState::kWait;
            return true;
          } else {
            // We got the finish
            return false;
          }
        }
      } else {
        count_ = wait_count_;
        state_ = PlatformState::kDown;
        return true;
      }
      break;
    
    case PlatformState::kDown:
      new_pos = sector_->floor_height - speed_;
      if (new_pos < low_pos_) {
        new_pos = low_pos_;
      }

      world_->TryToChangeSectorHeight(sector_, new_pos, sector_->ceiling_height, false);
      sector_->floor_height = new_pos;
      if (new_pos == low_pos_) {
        count_ = wait_count_;
        state_ = PlatformState::kWait;
        return true;
      }
      break;
    
    case PlatformState::kWait:
      if (!--count_) {
        if (sector_->floor_height == low_pos_) {
          state_ = PlatformState::kUp;
        } else {
          state_ = PlatformState::kDown;
        }
      }
      return true;

    case PlatformState::kSuspend:
      break;
  }

  return true;
}

} // namespace sobj
