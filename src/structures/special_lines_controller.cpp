#include "special_lines_controller.h"

#include <iostream>
#include <cassert>

#include "world/world.h"
#include "door.h"
#include "floor.h"
#include "ceiling.h"
#include "platform.h"
#include "line_texture_switcher.h"

#define DEBUG_CODE

#ifdef DEBUG_CODE
  #define D_PRINT_UNPROCESSED_LINES
#endif

namespace sobj {

SpecialLinesController::SpecialLinesController(world::World* w) : world_(w) {
  for (auto& sec : world_->GetSectors()) {
    if (sec.tag != 0) {
      tag_sectors_[sec.tag].push_back(&sec);
    }
  }
}

void SpecialLinesController::UseLine(world::Line* line, mobj::MapObject* mobj) {
  if (mobj->mobj_type != id::MT_PLAYER) {
    if (line->flags & world::kLDFSecret) {
      // Only player can open secrets
      return;
    }

    switch (line->specials)
    {
      case 1:
      case 32:
      case 33:
      case 34:
        break;
      
      default:
        return;
    }
  }

  switch (line->specials)
  {
    case 1:
    case 26:
    case 27:
    case 28:
    case 31:
    case 32:
    case 33:
    case 34:
    case 117:
    case 118:
      // Manual doors, don't have tag
      UseManualDoor(line, mobj);
      break;

    case 29:
    case 50:
    case 103:
    case 111:
    case 112:
    case 113:
    case 42:
    case 61:
    case 63:
    case 114:
    case 115:
    case 116:
      UseTagDoor(line, mobj);
      break;

    case 99:
    case 134:
    case 136:
    case 133:
    case 135:
    case 137:
      UseLockedTagDoor(line, mobj);
      break;

    case 18:
    case 23:
    case 71:
    case 55:
    case 101:
    case 102:
    case 131:
    case 140:
    case 45:
    case 60:
    case 64:
    case 65:
    case 69:
    case 70:
    case 132:
      UseFloor(line, mobj);
      break;

    case 14:
    case 15:
    case 20:
    case 21:
    case 122:
    case 62:
    case 66:
    case 67:
    case 68:
    case 123:
      UsePlatform(line, mobj);
      break;

    case 41:
    case 49:
    case 43:
      UseCeiling(line, mobj);
      break;

    case 138:
    case 139:
      // TODO: Light control
      break;

    case 7:
    case 127:
      // TODO: BuildStairs
      break;

    case 9:
      // TODO: Donut ???
      break;

    case 11:
      // TODO: Exit level
      break;

    case 51:
      // Secret exit
      break;

    default:
        #ifdef D_PRINT_UNPROCESSED_LINES
        std::cout << "UseLine(): undispatched line->specials = " << line->specials << std::endl;
        #endif
      break;
  }
}

void SpecialLinesController::HitLine(world::Line* l, mobj::MapObject* mobj) {

}

void SpecialLinesController::CrossLine(world::Line* l, mobj::MapObject* mobj) {
  // TODO: this foo uses FloorType::kLowerAndChange, FloorType::kRaiseFloor24AndChange, FloorType::kRaiseToTexture

}

void SpecialLinesController::UseManualDoor(world::Line* line, mobj::MapObject* mobj) {
  world::Sector* sec = line->sides[1]->sector;
  if (sec->has_sobj) {
    return;
  }

  switch (line->specials) {
    case 26:
    case 32:
      // Check for blue key
      if (!mobj->IsCardPresent(mobj::CardType::kBlueCard) &&
          !mobj->IsCardPresent(mobj::CardType::kBlueScull)) {
            return;
          }
      break;
    case 27:
    case 34:
      // Check for yellow key
      if (!mobj->IsCardPresent(mobj::CardType::kYellowCard) &&
          !mobj->IsCardPresent(mobj::CardType::kYellowScull)) {
            return;
          }
      break;
    case 28:
    case 33:
      // Check for red key
      if (!mobj->IsCardPresent(mobj::CardType::kBlueCard) &&
          !mobj->IsCardPresent(mobj::CardType::kBlueScull)) {
            return;
          }
      break;
  }

  std::unique_ptr<Door> door;

  switch (line->specials) {
    case 1:
    case 26:
    case 27:
    case 28:
      door.reset(new Door(world_, sec, line, DoorType::kNormal));
      break;

    case 31:
    case 32:
    case 33:
    case 34:
      door.reset(new Door(world_, sec, line, DoorType::kOpen));
      line->specials = 0;  // one time action
      break;

    case 117:
      door.reset(new Door(world_, sec, line, DoorType::kBlazeRaise));
      break;
    case 118:
      door.reset(new Door(world_, sec, line, DoorType::kBlazeOpen));
      line->specials = 0;  // one time action
      break;

    default:
      #ifdef D_PRINT_UNPROCESSED_LINES
      std::cout << "UseManualDoor(): unprocessed line->specials = " << line->specials << std::endl;
      #endif
      return;
  }

  sobjs_.push_back(std::move(door));
}

void SpecialLinesController::UseTagDoor(world::Line* line, mobj::MapObject* mobj) {
  assert(tag_sectors_.count(line->tag));
  auto& sectors = tag_sectors_[line->tag];

  bool is_ok = false;
  bool clear_special = false;

  for (auto sec : sectors) {
    if (sec->has_sobj) {
      continue;
    }

    is_ok = true;

    std::unique_ptr<Door> door;

    switch (line->specials)
    {
      // Use line action
      case 29:
        door.reset(new Door(world_, sec, line, DoorType::kNormal));
        clear_special = true;
        break;
      case 50:
        door.reset(new Door(world_, sec, line, DoorType::kClose));
        clear_special = true;
        break;
      case 103:
        door.reset(new Door(world_, sec, line, DoorType::kOpen));
        clear_special = true;
        break;
      case 111:
        door.reset(new Door(world_, sec, line, DoorType::kBlazeRaise));
        clear_special = true;
        break;
      case 112:
        door.reset(new Door(world_, sec, line, DoorType::kBlazeOpen));
        clear_special = true;
        break;
      case 113:
        door.reset(new Door(world_, sec, line, DoorType::kBlazeClose));
        clear_special = true;
        break;
      case 42:
        door.reset(new Door(world_, sec, line, DoorType::kClose));
        break;
      case 61:
        door.reset(new Door(world_, sec, line, DoorType::kOpen));
        break;
      case 63:
        door.reset(new Door(world_, sec, line, DoorType::kNormal));
        break;
      case 114:
        door.reset(new Door(world_, sec, line, DoorType::kBlazeRaise));
        break;
      case 115:
        door.reset(new Door(world_, sec, line, DoorType::kBlazeOpen));
        break;
      case 116:
        door.reset(new Door(world_, sec, line, DoorType::kBlazeClose));
        break;
      // Use line tag closed door
      case 99:
      case 134:
      case 136:
        door.reset(new Door(world_, sec, line, DoorType::kBlazeOpen));
        break;
      case 133:
      case 135:
      case 137:
        door.reset(new Door(world_, sec, line, DoorType::kBlazeOpen));
        clear_special = true;
        break;

      default:
        #ifdef D_PRINT_UNPROCESSED_LINES
        std::cout << "UseTagDoor(): unprocessed line->specials = " << line->specials << std::endl;
        #endif
        continue;
    }

    sobjs_.push_back(std::move(door));
  }

  if (clear_special) {
    line->specials = 0;
  }
  if (is_ok && LineTextureSwitcher::IsSwitch(line)) {
    sobjs_.push_back(std::unique_ptr<LineTextureSwitcher>(new LineTextureSwitcher(line)));
  }
}

void SpecialLinesController::UseFloor(world::Line* line, mobj::MapObject* mobj) {
  assert(tag_sectors_.count(line->tag));
  auto& sectors = tag_sectors_[line->tag];

  bool is_ok = false;
  bool clear_special = false;

  for (auto sec : sectors) {
    if (sec->has_sobj) {
      continue;
    }

    is_ok = true;

    std::unique_ptr<Floor> floor;

    switch (line->specials)
    {
      case 18:
        floor.reset(new Floor(world_, sec, line, FloorType::kRaiseFloorToNearest));
        clear_special = true;
        break;
      case 23:
        floor.reset(new Floor(world_, sec, line, FloorType::kLowerFloorToLowest));
        clear_special = true;
        break;
      case 71:
        floor.reset(new Floor(world_, sec, line, FloorType::kTurboLower));
        clear_special = true;
        break;
      case 55:
        floor.reset(new Floor(world_, sec, line, FloorType::kRaiseFloorCrush));
        clear_special = true;
        break;
      case 101:
        floor.reset(new Floor(world_, sec, line, FloorType::kRaiseFloor));
        clear_special = true;
        break;
      case 102:
        floor.reset(new Floor(world_, sec, line, FloorType::kLowerFloor));
        clear_special = true;
        break;
      case 131:
        floor.reset(new Floor(world_, sec, line, FloorType::kRaiseFloorTurbo));
        clear_special = true;
        break;
      case 140:
        floor.reset(new Floor(world_, sec, line, FloorType::kRaiseFloor512));
        clear_special = true;
        break;
      case 45:
        floor.reset(new Floor(world_, sec, line, FloorType::kLowerFloor));
        break;
      case 60:
        floor.reset(new Floor(world_, sec, line, FloorType::kLowerFloorToLowest));
        break;
      case 64:
        floor.reset(new Floor(world_, sec, line, FloorType::kRaiseFloor));
        break;
      case 65:
        floor.reset(new Floor(world_, sec, line, FloorType::kRaiseFloorCrush));
        break;
      case 69:
        floor.reset(new Floor(world_, sec, line, FloorType::kRaiseFloorToNearest));
        break;
      case 70:
        floor.reset(new Floor(world_, sec, line, FloorType::kTurboLower));
        break;
      case 132:
        floor.reset(new Floor(world_, sec, line, FloorType::kRaiseFloorTurbo));
        break;

      default:
        #ifdef D_PRINT_UNPROCESSED_LINES
        std::cout << "UseFloor(): unprocessed line->specials = " << line->specials << std::endl;
        #endif
        continue;
    }

    sobjs_.push_back(std::move(floor));
  }

  if (clear_special) {
    line->specials = 0;
  }
  if (is_ok && LineTextureSwitcher::IsSwitch(line)) {
    sobjs_.push_back(std::unique_ptr<LineTextureSwitcher>(new LineTextureSwitcher(line)));
  }

}

void SpecialLinesController::UseLockedTagDoor(world::Line* line, mobj::MapObject* mobj) {
  switch (line->specials)
  {
    case 99:
    case 133:
      // Blue key
      if (!mobj->IsCardPresent(mobj::CardType::kBlueCard) &&
          !mobj->IsCardPresent(mobj::CardType::kBlueScull)) {
            return;
          }
      break;
    
    case 136:
    case 137:
      // Yellow key
      if (!mobj->IsCardPresent(mobj::CardType::kYellowCard) &&
          !mobj->IsCardPresent(mobj::CardType::kYellowScull)) {
            return;
          }
      break;
    
    case 134:
    case 135:
      // Red key
      if (!mobj->IsCardPresent(mobj::CardType::kRedCard) &&
          !mobj->IsCardPresent(mobj::CardType::kRedScull)) {
            return;
          }
      break;
  }

  UseTagDoor(line, mobj);
}

void SpecialLinesController::UsePlatform(world::Line* line, [[maybe_unused]] mobj::MapObject* mobj) {
  assert(tag_sectors_.count(line->tag));
  auto& sectors = tag_sectors_[line->tag];

  bool is_ok = false;
  bool clean_special = false;

  for (auto& plat : sobjs_) {
    plat->ActivateInStasis(line->tag);
  }

  for (auto sec : sectors) {
    if (sec->has_sobj) {
      continue;
    }

    is_ok = true;

    std::unique_ptr<Platform> platform;

    switch (line->specials)
    {
      case 14:
        platform.reset(new Platform(world_, sec, line, PlatformType::kRaiseAndChange, 32));
        clean_special = true;
        break;
      case 15:
        platform.reset(new Platform(world_, sec, line, PlatformType::kRaiseAndChange, 24));
        clean_special = true;
        break;
      case 20:
        platform.reset(new Platform(world_, sec, line, PlatformType::kRaiseToNearestAndChange));
        clean_special = true;
        break;
      case 21:
        platform.reset(new Platform(world_, sec, line, PlatformType::kDownWaitUpStay));
        clean_special = true;
        break;
      case 122:
        platform.reset(new Platform(world_, sec, line, PlatformType::kBlazeDWUS));
        clean_special = true;
        break;
      case 62:
        platform.reset(new Platform(world_, sec, line, PlatformType::kDownWaitUpStay, 1));
        break;
      case 66:
        platform.reset(new Platform(world_, sec, line, PlatformType::kRaiseAndChange, 24));
        break;
      case 67:
        platform.reset(new Platform(world_, sec, line, PlatformType::kRaiseAndChange, 32));
        break;
      case 68:
        platform.reset(new Platform(world_, sec, line, PlatformType::kRaiseToNearestAndChange));
        break;
      case 123:
        platform.reset(new Platform(world_, sec, line, PlatformType::kBlazeDWUS));
        break;

      default:
        #ifdef D_PRINT_UNPROCESSED_LINES
        std::cout << "UsePlatform(): unprocessed line->specials = " << line->specials << std::endl;
        #endif
        continue;
    }

    sobjs_.push_back(std::move(platform));
  }

  if (clean_special) {
    line->specials = 0;
  }
  if (is_ok && LineTextureSwitcher::IsSwitch(line)) {
    sobjs_.push_back(std::unique_ptr<LineTextureSwitcher>(new LineTextureSwitcher(line)));
  }
}

void SpecialLinesController::UseCeiling(world::Line* line, [[maybe_unused]] mobj::MapObject* mobj) {
  assert(tag_sectors_.count(line->tag));
  auto& sectors = tag_sectors_[line->tag];

  bool is_ok = false;
  bool clean_special = false;

  for (auto& ceiling : sobjs_) {
    // Reactivate some ceilings
    ceiling->ActivateInStasis(line->tag);
  }

  for (auto sec : sectors) {
    if (sec->has_sobj) {
      continue;
    }

    is_ok = true;

    std::unique_ptr<Ceiling> ceil;

    switch (line->specials)
    {
      case 41:
        ceil.reset(new Ceiling(world_, sec, line, CeilingType::kLowerToFloor));
        clean_special = true;
        break;
      case 49:
        ceil.reset(new Ceiling(world_, sec, line, CeilingType::kCrushAndRaise));
        clean_special = true;
        break;
      case 43:
        ceil.reset(new Ceiling(world_, sec, line, CeilingType::kLowerToFloor));
        break;

      default:
        #ifdef D_PRINT_UNPROCESSED_LINES
        std::cout << "UseCeiling(): unprocessed line->specials = " << line->specials << std::endl;
        #endif
        continue;
    }

    sobjs_.push_back(std::move(ceil));
  }

  if (clean_special) {
    line->specials = 0;
  }
  if (is_ok && LineTextureSwitcher::IsSwitch(line)) {
    sobjs_.push_back(std::unique_ptr<LineTextureSwitcher>(new LineTextureSwitcher(line)));
  }
}

void SpecialLinesController::TickTime() {
  for (auto it = std::begin(sobjs_); it != std::end(sobjs_); ) {
    bool ret = (*it)->TickTime();
    if (!ret) {
      it = sobjs_.erase(it);
    } else {
      ++it;
    }
  }
}

} // namespace sobj
