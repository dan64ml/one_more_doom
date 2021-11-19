#include "door.h"

#include <cassert>

#include "sobj_utils.h"

#include "utils/world_utils.h"
#include "world/world.h"

namespace sobj {

Door::Door(world::World* w, world::Sector* s, world::Line* l, DoorType type) 
  : world_(w), sector_(s), line_(l), type_(type) {
  wait_time_ = kDoorWaitTime;
  speed_ = kDoorSpeed;

  sector_->has_sobj = true;

  switch (type_)
  {
    case DoorType::kBlazeClose:
      top_pos_ = math::GetLowestCeilingHeight(sector_) - 4.0;
      direction_ = MoveDirection::kDown;
      speed_ = 4 * kDoorSpeed;
      break;

    case DoorType::kClose:
      top_pos_ = math::GetLowestCeilingHeight(sector_) - 4.0;
      direction_ = MoveDirection::kDown;
      break;

    case DoorType::kClose30ThenOpen:
      top_pos_ = sector_->ceiling_height;
      direction_ = MoveDirection::kDown;
      break;

    case DoorType::kBlazeRaise:
    case DoorType::kBlazeOpen:
      direction_ = MoveDirection::kUp;
      top_pos_ = math::GetLowestCeilingHeight(sector_) - 4.0;
      speed_ = 4 * kDoorSpeed;
      break;

    case DoorType::kNormal:
    case DoorType::kOpen:
      direction_ = MoveDirection::kUp;
      top_pos_ = math::GetLowestCeilingHeight(sector_) - 4.0;
      break;

    default:
      assert(false);
      break;
  }
}

Door::~Door() {
  sector_->has_sobj = false;
}

bool Door::TickTime() {
  MoveResult res;

  switch (direction_ )
  {
    case MoveDirection::kWait:
      if (!(--wait_counter_)) {
        switch (type_)
        {
          case DoorType::kBlazeRaise:
            direction_ = MoveDirection::kDown;
            break;
          case DoorType::kNormal:
            direction_ = MoveDirection::kDown;
            break;
          case DoorType::kClose30ThenOpen:
            direction_ = MoveDirection::kUp;
            break;
          
          default:
            break;
        }
      }
      break;
    
    case MoveDirection::kInitialWait:
      if (!(--wait_counter_)) {
        switch (type_)
        {
          case DoorType::kRaiseIn5Mins:
            direction_ = MoveDirection::kUp;
            type_ = DoorType::kNormal;
            break;
          
          default:
            break;
        }
      }
      break;

    case MoveDirection::kDown:
      res = MoveCeiling(sector_, speed_, sector_->floor_height, false, direction_);
      if (res == MoveResult::kGotDest) {
        switch (type_)
        {
          case DoorType::kBlazeRaise:
          case DoorType::kBlazeOpen:
            return false;
          
          case DoorType::kNormal:
          case DoorType::kClose:
            return false;;
  
          case DoorType::kClose30ThenOpen:
            direction_ = MoveDirection::kWait;
            wait_counter_ = 35 * 30;  // Original behavier
            break;

          default:
            break;
        }
      } else if (res == MoveResult::kCrushed) {
        switch (type_)
        {
          case DoorType::kBlazeClose:
          case DoorType::kClose:
            // Do not change direction
            break;

          default:
            direction_ = MoveDirection::kUp;
            break;
        }

      }
      break;

    case MoveDirection::kUp:
      res = MoveCeiling(sector_, speed_, top_pos_, false, direction_);
      if (res == MoveResult::kGotDest) {
        switch (type_)
        {
          case DoorType::kBlazeRaise:
          case DoorType::kNormal:
            direction_ = MoveDirection::kWait;
            wait_counter_ = wait_time_;
            break;
          
          case DoorType::kClose30ThenOpen:
          case DoorType::kBlazeOpen:
          case DoorType::kOpen:
            return false;

          default:
            break;
        }
      }
      break;

    default:
      break;
  }

  return true;
}

//bool Door::CanChangeHeight(int ceiling_height) {
//  return world_->TryToChangeSectorHeight(sector_, sector_->floor_height, ceiling_height, false);
//}

} // namespace sobj