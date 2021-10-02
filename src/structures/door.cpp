#include "door.h"

#include <cassert>

namespace sobj {

bool Door::TickTime(world::World* w) {
  if (current_state_ == kMove1) {
    if (MoveCeiling()) {
      if (wait_counter_ == -1) {
        return false;
      } else {
        current_state_ = kWait;
        return true;
      }
    } else {
      return true;
    }
  } else if (current_state_ == kWait) {
    if (wait_counter_-- == 0) {
      current_state_ = kMove2;
      move_direction_ *= -1;
    }
    return true;
  } else if (current_state_ == kMove2) {
    return !MoveCeiling();
  }

  assert(false);
  return false;
}

bool Door::MoveCeiling() {
  int new_height = sector_->ceiling_height + move_speed_;
  if (move_direction_ == 1 && new_height > move_stop_level_[current_state_]) {
    new_height = move_stop_level_[current_state_];
  }
  if (move_direction_ == -1 && new_height < move_stop_level_[current_state_]) {
    new_height = move_stop_level_[current_state_];
  }

  if (CheckObstacles(new_height)) {
    // Wait for free doorway
    return false;
  }

  sector_->ceiling_height = new_height;

  return (new_height == move_stop_level_[current_state_]);
}

bool Door::CheckObstacles(int ceiling_height) {
  return false;
}

} // namespace sobj