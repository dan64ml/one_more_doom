#include "floor.h"

#include <cassert>

#include "sobj_utils.h"

#include "utils/world_utils.h"
#include "world/world.h"

namespace sobj {

Floor::Floor(world::World* w, world::Sector* s, world::Line* line, FloorType type) 
  : world_(w), sector_(s), type_(type) {
  sector_->has_sobj = true;

  switch (type)
  {
    case FloorType::kLowerFloor:
      direction_ = MoveDirection::kDown;
      target_height_ = math::GetHighestFloorHeight(sector_);
      break;
    case FloorType::kLowerFloorToLowest:
      direction_ = MoveDirection::kDown;
      target_height_ = math::GetLowestFloorHeight(sector_);
      break;
    case FloorType::kTurboLower:
      direction_ = MoveDirection::kDown;
      speed_ = 4 * kFloorSpeed;
      target_height_ = math::GetHighestFloorHeight(sector_);
      if (target_height_ != sector_->floor_height) {
        target_height_ += 8;
      }
      break;

    case FloorType::kRaiseFloorCrush:
      crush_ = true;
      [[fallthrough]];
    case FloorType::kRaiseFloor:
      direction_ = MoveDirection::kUp;
      target_height_ = math::GetLowestCeilingHeight(sector_);
      if (target_height_ > sector_->ceiling_height) {
        target_height_ = sector_->ceiling_height;
      }
      target_height_ -= (type_ == FloorType::kRaiseFloorCrush) ? 8 : 0;
      break;

    case FloorType::kRaiseFloorTurbo:
      direction_ = MoveDirection::kUp;
      speed_ = 4 * kFloorSpeed;
      target_height_ = math::GetNextHighestFloorHeight(sector_, sector_->floor_height);
      break;
    case FloorType::kRaiseFloorToNearest:
      direction_ = MoveDirection::kUp;
      target_height_ = math::GetNextHighestFloorHeight(sector_, sector_->floor_height);
      break;
    case FloorType::kRaiseFloor24:
      direction_ = MoveDirection::kUp;
      target_height_ = sector_->floor_height + 24;
      break;
    case FloorType::kRaiseFloor512:
      direction_ = MoveDirection::kUp;
      target_height_ = sector_->floor_height + 512;
      break;
    
    case FloorType::kRaiseFloor24AndChange:
      direction_ = MoveDirection::kUp;
      target_height_ = sector_->floor_height + 24;
      sector_->floor_pic = line->sides[0]->sector->floor_pic;
      sector_->special = line->sides[0]->sector->special;
      break;

    case FloorType::kRaiseToTexture:
    {
      direction_ = MoveDirection::kUp;
      target_height_ = sector_->floor_height + 24;

      // TODO:
      assert(false);
      //for (auto ln : sector_->lines) {
      //  if (ln->sides[1]) {

      //  }
      //}
    }
    break;

    case FloorType::kLowerAndChange:
    {
      direction_ = MoveDirection::kDown;
      target_height_ = math::GetLowestFloorHeight(sector_);
    // TODO!!!!!!!
      assert(false);
    }
    break;

  default:
    break;
  }
}

Floor::~Floor() {
  sector_->has_sobj = false;
}

bool Floor::TickTime() {
  MoveResult res = MoveFloor(sector_, speed_, target_height_, crush_, direction_);

  if (res == MoveResult::kGotDest) {
    if (direction_ == MoveDirection::kUp) {
      switch (type_)
      {
        case FloorType::kDonutRaise:
          sector_->special = new_special_;
          sector_->floor_pic = texture_;
          break;
        
        default:
          break;
      }
    } else if (direction_ == MoveDirection::kDown) {
      switch (type_)
      {
        case FloorType::kLowerAndChange:
          sector_->special = new_special_;
          sector_->floor_pic = texture_;
          break;

        default:
          break;
      }
    }
    return false;
  }

  return true;
}

} // namespace sobj
