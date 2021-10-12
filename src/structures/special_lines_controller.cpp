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

void SpecialLinesController::HitLine(world::Line* line, mobj::MapObject* mobj) {
  if (mobj->mobj_type != id::MT_PLAYER && line->specials != 46) {
    return;
  }

  switch (line->specials)
  {
    case 24:
      UseFloor(line, mobj);
      break;

    case 46:
      UseTagDoor(line, mobj);
      break;
    
    case 47:
      UsePlatform(line, mobj);
      break;

    default:
      break;
  }
}

void SpecialLinesController::CrossLine(world::Line* line, mobj::MapObject* mobj) {
  if (mobj->mobj_type != id::MT_PLAYER) {
    switch (mobj->mobj_type)
    {
      // Can't interact with such lines
      case id::MT_ROCKET:
      case id::MT_PLASMA:
      case id::MT_BFG:
      case id::MT_TROOPSHOT:
      case id::MT_HEADSHOT:
      case id::MT_BRUISERSHOT:
        return;
      
      default:
        break;
    }

    switch (line->specials)
    {
      case 39:
      case 97:
      case 125:
      case 126:
      case 4:
      case 10:
      case 88:
        break;
      
      default:
        return;
    }
  }

  switch (line->specials)
  {
    case 2:
    case 3:
    case 4:
    case 16:
    case 108:
    case 109:
    case 110:
    case 75:
    case 76:
    case 86:
    case 90:
    case 105:
    case 106:
    case 107:
      UseTagDoor(line, mobj);
      break;
    
    case 5:
    case 19:
    case 30:
    case 36:
    case 37:
    case 38:
    case 56:
    case 58:
    case 59:
    case 119:
    case 130:
    case 82:
    case 83:
    case 84:
    case 91:
    case 92:
    case 93:
    case 94:
    case 96:
    case 98:
    case 128:
    case 129:
      UseFloor(line, mobj);
      break;

    case 6:
    case 25:
    case 44:
    case 141:
    case 72:
    case 73:
    case 77:
      UseCeiling(line, mobj);
      break;

    case 10:
    case 22:
    case 53:
    case 121:
    case 87:
    case 88:
    case 95:
    case 120:
      UsePlatform(line, mobj);
      break;

    case 8:
    case 100:
      // TODO: Build stairs
      break;

    case 12:
    case 13:
    case 35:
    case 79:
    case 80:
    case 81:
      // TODO: Light turn on
      break;

    case 17:
      // TODO: Start light strobing
      break;
    
    case 39:
    case 125:
    case 97:
    case 126:
      // TODO: Teleport
      break;

    case 52:
      // TODO: Exit
      break;
    
    case 124:
      // TODO: Secret exit
      break;
    
    case 104:
      // TODO: Turn lights off in sector(tag)
      break;

    case 54:
    case 89:
      StopPlatform(line);
      break;

    case 57:
    case 74:
      // TODO: Ceiling crush stop
      break;

    case 40:
      // TODO: Move ceiling and floor
      break;

    default:
        #ifdef D_PRINT_UNPROCESSED_LINES
        std::cout << "CrossLine(): undispatched line->specials = " << line->specials << std::endl;
        #endif
      break;
  }
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

void SpecialLinesController::UseTagDoor(world::Line* line, [[maybe_unused]] mobj::MapObject* mobj) {
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

      // Cross line action
      case 2:
        door.reset(new Door(world_, sec, line, DoorType::kOpen));
        clear_special = true;
        is_ok = false; // Nothing to switch
        break;
      case 3:
        door.reset(new Door(world_, sec, line, DoorType::kClose));
        clear_special = true;
        is_ok = false;
        break;
      case 4:
        door.reset(new Door(world_, sec, line, DoorType::kNormal));
        clear_special = true;
        is_ok = false;
        break;
      case 16:
        door.reset(new Door(world_, sec, line, DoorType::kClose30ThenOpen));
        clear_special = true;
        is_ok = false;
        break;
      case 108:
        door.reset(new Door(world_, sec, line, DoorType::kBlazeRaise));
        clear_special = true;
        is_ok = false;
        break;
      case 109:
        door.reset(new Door(world_, sec, line, DoorType::kBlazeOpen));
        clear_special = true;
        is_ok = false;
        break;
      case 110:
        door.reset(new Door(world_, sec, line, DoorType::kBlazeClose));
        clear_special = true;
        is_ok = false;
        break;
      case 75:
        door.reset(new Door(world_, sec, line, DoorType::kClose));
        clear_special = false;
        is_ok = false;
        break;
      case 86:
        door.reset(new Door(world_, sec, line, DoorType::kOpen));
        clear_special = false;
        is_ok = false;
        break;
      case 90:
        door.reset(new Door(world_, sec, line, DoorType::kNormal));
        clear_special = false;
        is_ok = false;
        break;
      case 105:
        door.reset(new Door(world_, sec, line, DoorType::kBlazeRaise));
        clear_special = false;
        is_ok = false;
        break;
      case 106:
        door.reset(new Door(world_, sec, line, DoorType::kBlazeOpen));
        clear_special = false;
        is_ok = false;
        break;
      case 107:
        door.reset(new Door(world_, sec, line, DoorType::kBlazeClose));
        clear_special = false;
        is_ok = false;
        break;
      case 76:
        door.reset(new Door(world_, sec, line, DoorType::kClose30ThenOpen));
        clear_special = false;
        is_ok = false;
        break;

      // Hit line action
      case 46:
        door.reset(new Door(world_, sec, line, DoorType::kOpen));
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

void SpecialLinesController::UseFloor(world::Line* line, [[maybe_unused]] mobj::MapObject* mobj) {
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
      // Use line action
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

      // Cross line action
      case 5:
        floor.reset(new Floor(world_, sec, line, FloorType::kRaiseFloor));
        clear_special = true;
        is_ok = false;
        break;
      case 19:
        floor.reset(new Floor(world_, sec, line, FloorType::kLowerFloor));
        clear_special = true;
        is_ok = false;
        break;
      case 30:
        floor.reset(new Floor(world_, sec, line, FloorType::kRaiseToTexture));
        clear_special = true;
        is_ok = false;
        break;
      case 36:
        floor.reset(new Floor(world_, sec, line, FloorType::kTurboLower));
        clear_special = true;
        is_ok = false;
        break;
      case 37:
        floor.reset(new Floor(world_, sec, line, FloorType::kLowerAndChange));
        clear_special = true;
        is_ok = false;
        break;
      case 38:
        floor.reset(new Floor(world_, sec, line, FloorType::kLowerFloorToLowest));
        clear_special = true;
        is_ok = false;
        break;
      case 56:
        floor.reset(new Floor(world_, sec, line, FloorType::kRaiseFloorCrush));
        clear_special = true;
        is_ok = false;
        break;
      case 58:
        floor.reset(new Floor(world_, sec, line, FloorType::kRaiseFloor24));
        clear_special = true;
        is_ok = false;
        break;
      case 59:
        floor.reset(new Floor(world_, sec, line, FloorType::kRaiseFloor24AndChange));
        clear_special = true;
        is_ok = false;
        break;
      case 119:
        floor.reset(new Floor(world_, sec, line, FloorType::kRaiseFloorToNearest));
        clear_special = true;
        is_ok = false;
        break;
      case 130:
        floor.reset(new Floor(world_, sec, line, FloorType::kRaiseFloorTurbo));
        clear_special = true;
        is_ok = false;
        break;
      case 82:
        floor.reset(new Floor(world_, sec, line, FloorType::kLowerFloorToLowest));
        clear_special = false;
        is_ok = false;
        break;
      case 83:
        floor.reset(new Floor(world_, sec, line, FloorType::kLowerFloor));
        clear_special = false;
        is_ok = false;
        break;
      case 84:
        floor.reset(new Floor(world_, sec, line, FloorType::kLowerAndChange));
        clear_special = false;
        is_ok = false;
        break;
      case 91:
        floor.reset(new Floor(world_, sec, line, FloorType::kRaiseFloor));
        clear_special = false;
        is_ok = false;
        break;
      case 92:
        floor.reset(new Floor(world_, sec, line, FloorType::kRaiseFloor24));
        clear_special = false;
        is_ok = false;
        break;
      case 93:
        floor.reset(new Floor(world_, sec, line, FloorType::kRaiseFloor24AndChange));
        clear_special = false;
        is_ok = false;
        break;
      case 94:
        floor.reset(new Floor(world_, sec, line, FloorType::kRaiseFloorCrush));
        clear_special = false;
        is_ok = false;
        break;
      case 96:
        floor.reset(new Floor(world_, sec, line, FloorType::kRaiseToTexture));
        clear_special = false;
        is_ok = false;
        break;
      case 98:
        floor.reset(new Floor(world_, sec, line, FloorType::kTurboLower));
        clear_special = false;
        is_ok = false;
        break;
      case 128:
        floor.reset(new Floor(world_, sec, line, FloorType::kRaiseFloorToNearest));
        clear_special = false;
        is_ok = false;
        break;
      case 129:
        floor.reset(new Floor(world_, sec, line, FloorType::kRaiseFloorTurbo));
        clear_special = false;
        is_ok = false;
        break;

      // Hit line action
      case 24:
        floor.reset(new Floor(world_, sec, line, FloorType::kRaiseFloor));
        clear_special = true;
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
      // Use line action
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

      // Cross line action
      case 10:
        platform.reset(new Platform(world_, sec, line, PlatformType::kDownWaitUpStay));
        clean_special = true;
        is_ok = false;
        break;
      case 22:
        platform.reset(new Platform(world_, sec, line, PlatformType::kRaiseToNearestAndChange));
        clean_special = true;
        is_ok = false;
        break;
      case 53:
        platform.reset(new Platform(world_, sec, line, PlatformType::kPerpetualRaise));
        clean_special = true;
        is_ok = false;
        break;
      case 121:
        platform.reset(new Platform(world_, sec, line, PlatformType::kBlazeDWUS));
        clean_special = true;
        is_ok = false;
        break;
      case 87:
        platform.reset(new Platform(world_, sec, line, PlatformType::kPerpetualRaise));
        clean_special = false;
        is_ok = false;
        break;
      case 88:
        platform.reset(new Platform(world_, sec, line, PlatformType::kDownWaitUpStay));
        clean_special = false;
        is_ok = false;
        break;
      case 95:
        platform.reset(new Platform(world_, sec, line, PlatformType::kRaiseToNearestAndChange));
        clean_special = false;
        is_ok = false;
        break;
      case 120:
        platform.reset(new Platform(world_, sec, line, PlatformType::kBlazeDWUS));
        clean_special = false;
        is_ok = false;
        break;

      // Hit line action
      case 47:
        platform.reset(new Platform(world_, sec, line, PlatformType::kRaiseToNearestAndChange));
        clean_special = true;
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
      // Use line action
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

      // Cross line action
      case 6:
        ceil.reset(new Ceiling(world_, sec, line, CeilingType::kFastCrushAndRaise));
        clean_special = true;
        is_ok = false;
        break;
      case 25:
        ceil.reset(new Ceiling(world_, sec, line, CeilingType::kCrushAndRaise));
        clean_special = true;
        is_ok = false;
        break;
      case 44:
        ceil.reset(new Ceiling(world_, sec, line, CeilingType::kLowerAndCrush));
        clean_special = true;
        is_ok = false;
        break;
      case 141:
        ceil.reset(new Ceiling(world_, sec, line, CeilingType::kSilentCrushAndRaise));
        clean_special = true;
        is_ok = false;
        break;
      case 72:
        ceil.reset(new Ceiling(world_, sec, line, CeilingType::kLowerAndCrush));
        clean_special = false;
        is_ok = false;
        break;
      case 73:
        ceil.reset(new Ceiling(world_, sec, line, CeilingType::kCrushAndRaise));
        clean_special = false;
        is_ok = false;
        break;
//    case 74:
//        ceil.reset(new Ceiling(world_, sec, line, CeilingType::kLowerAndCrush));
//        clean_special = false;
//        is_ok = false;
//        break;
      case 77:
        ceil.reset(new Ceiling(world_, sec, line, CeilingType::kFastCrushAndRaise));
        clean_special = false;
        is_ok = false;
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

void SpecialLinesController::StopPlatform(world::Line* line) {
  for (auto& obj : sobjs_) {
    obj->StopObject(line->tag);
  }

  if (line->specials == 54) {
    line->specials = 0;
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
