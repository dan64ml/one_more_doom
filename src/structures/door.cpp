#include "door.h"

#include <cassert>

#include "utils/world_utils.h"

namespace sobj {

Door::Door(world::World* w, world::Sector* s, DoorType type, int speed, bool wait_obstacle) 
  : world_(w), sector_(s), type_(type), move_speed_(speed), wait_obstacle_(wait_obstacle) {
  floor_level_ = sector_->floor_height;
  // TODO: find real height
  door_top_level_ = math::GetLowestCeilingHeight(sector_) - 4;

  switch (type)
  {
  case DoorType::kClose:
  case DoorType::kCloseThenOpen:
    move_direction_ = -1;
    break;
  
  case DoorType::kOpen:
  case DoorType::kOpenThenClose:
    move_direction_ = 1;
    break;

  default:
    assert(false);
    break;
  }
}

bool Door::TickTime() {
  if (move_direction_ == 1) {
    // Move up
    int new_height = sector_->ceiling_height + move_speed_;
    if (new_height > door_top_level_) {
      sector_->ceiling_height = door_top_level_;
      switch (type_)
      {
      case DoorType::kClose:
        move_direction_ = -1;
        return true;
      case DoorType::kOpen:
        return false;
      case DoorType::kCloseThenOpen:
        if (wait_counter_) {
          move_direction_ = -1;
          return true;
        }
        return false;
      case DoorType::kOpenThenClose:
        move_direction_ = 0;
        return true;
      }
    } else {
      sector_->ceiling_height = new_height;
      return true;
    }
  } else if (move_direction_ == -1) {
    // Move down
    int new_height = sector_->ceiling_height - move_speed_;
    if (new_height < sector_->floor_height) {
      // Reached the floor
      sector_->ceiling_height = sector_->floor_height;
      switch (type_)
      {
      case DoorType::kClose:
        return false;
      case DoorType::kOpen:
        assert(false);
        return true;
      case DoorType::kCloseThenOpen:
        move_direction_ = 0;
        return true;
      case DoorType::kOpenThenClose:
        return false;
      }
    }

    if (!CheckObstacles(new_height)) {
      sector_->ceiling_height = new_height;
      return true;
    } else {
      if (!wait_obstacle_) {
        move_direction_ *= -1;;
      }
      return true;
    }
  } else {
    // Wait state
    if (!--wait_counter_) {
      if (type_ == DoorType::kOpenThenClose) {
        move_direction_ = -1;
      } else if (type_ == DoorType::kCloseThenOpen) {
        move_direction_ = 1;
      } else {
        assert(false);
      }
    }
    return true;
  }
}

//bool Door::TickTime(world::World* w) {
//  if (current_state_ == kMove1) {
//    if (MoveCeiling()) {
//      if (wait_counter_ == -1) {
//        return false;
//      } else {
//        current_state_ = kWait;
//        return true;
//      }
//    } else {
//      return true;
//    }
//  } else if (current_state_ == kWait) {
//    if (wait_counter_-- == 0) {
//      current_state_ = kMove2;
//      move_direction_ *= -1;
//    }
//    return true;
//  } else if (current_state_ == kMove2) {
//    return !MoveCeiling();
//  }
//
//  assert(false);
//  return false;
//}

//bool Door::MoveCeiling() {
//  int new_height = sector_->ceiling_height + move_speed_;
//  if (move_direction_ == 1 && new_height > move_stop_level_[current_state_]) {
//    new_height = move_stop_level_[current_state_];
//  }
//  if (move_direction_ == -1 && new_height < move_stop_level_[current_state_]) {
//    new_height = move_stop_level_[current_state_];
//  }
//
//  if (CheckObstacles(new_height)) {
//    // Wait for free doorway
//    return false;
//  }
//
//  sector_->ceiling_height = new_height;
//
//  return (new_height == move_stop_level_[current_state_]);
//}

bool Door::CheckObstacles(int ceiling_height) {
  return false;
}

} // namespace sobj