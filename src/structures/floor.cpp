#include "floor.h"

#include "utils/world_utils.h"
#include "world/world.h"

namespace sobj {

Floor::Floor(world::World* w, world::Sector* s, FloorType type) 
  : world_(w), sector_(s), type_(type) {
  sector_->has_sobj = true;

  switch (type)
  {
  case FloorType::kLowerFloor:
    direction_ = -1;
    target_height_ = math::GetHighestFloorHeight(sector_);
    break;
  case FloorType::kLowerFloorToLowest:
    direction_ = -1;
    target_height_ = math::GetLowestFloorHeight(sector_);
    break;
  case FloorType::kTurboLower:
    direction_ = -1;
    speed_ = kTurboFloorSpeed;
    target_height_ = math::GetHighestFloorHeight(sector_);
    if (target_height_ != sector_->floor_height) {
      target_height_ += 8;
    }
    break;

  case FloorType::kRaiseFloorCrush:
    crush_ = true;
    [[fallthrough]];
  case FloorType::kRaiseFloor:
    direction_ = 1;
    target_height_ = math::GetLowestCeilingHeight(sector_);
    if (target_height_ > sector_->ceiling_height) {
      target_height_ = sector_->ceiling_height;
    }
    target_height_ -= (type_ == FloorType::kRaiseFloorCrush) ? 8 : 0;
    break;

  case FloorType::kRaiseFloorTurbo:
    direction_ = 1;
    speed_ = kTurboFloorSpeed;
    target_height_ = math::GetNextHighestFloorHeight(sector_, sector_->floor_height);
    break;
  case FloorType::kRaiseFloorToNearest:
    direction_ = 1;
    speed_ = kFloorSpeed;
    target_height_ = math::GetNextHighestFloorHeight(sector_, sector_->floor_height);
    break;
  case FloorType::kRaiseFloor24:
    direction_ = 1;
    speed_ = kFloorSpeed;
    target_height_ = sector_->floor_height + 24;
    break;
  case FloorType::kRaiseFloor512:
    direction_ = 1;
    speed_ = kFloorSpeed;
    target_height_ = sector_->floor_height + 512;
    break;
    
  case FloorType::kRaiseFloor24AndChange:
    direction_ = 1;
    speed_ = kFloorSpeed;
    target_height_ = sector_->floor_height + 24;
    //sector_->floor_pic = line->
    // TODO!!!!!!!
    // Require line :(((
    break;

  case FloorType::kRaiseToTexture:
    direction_ = 1;
    speed_ = kFloorSpeed;
    target_height_ = sector_->floor_height + 24;
    // TODO!!!!!!!
    break;

  case FloorType::kLowerAndChange:
    direction_ = -1;
    speed_ = kFloorSpeed;
    target_height_ = math::GetLowestFloorHeight(sector_);
    // TODO!!!!!!!
    break;


  default:
    break;
  }
}

Floor::~Floor() {
  sector_->has_sobj = false;
}

bool Floor::TickTime() {
  if (direction_ == 1) {
    // Move up
    int new_height = sector_->floor_height + speed_;
    if (new_height > target_height_) {
      new_height = target_height_;
    }

    if (world_->TryToChangeSectorHeight(sector_, new_height, sector_->ceiling_height, crush_)) {
      sector_->floor_height = new_height;
    } else {
      return true;
    }

    if (new_height == target_height_) {
      // TODO: extra actions for some floor types
      return false;
    }

    return true;
  } else {
    // Move down
    int new_height = sector_->floor_height - speed_;
    if (new_height < target_height_) {
      new_height = target_height_;
    }

    if (world_->TryToChangeSectorHeight(sector_, new_height, sector_->ceiling_height, crush_)) {
      sector_->floor_height = new_height;
    } else {
      return true;
    }

    if (new_height == target_height_) {
      // TODO: extra actions for some floor types
      return false;
    }

    return true;
  }
  
  return true;
}

} // namespace sobj
