#include "special_lines_controller.h"

#include <iostream>

#include "world/world.h"
#include "door.h"

#define DEBUG_CODE

#ifdef DEBUG_CODE
  #define D_PRINT_UNPROCESSED_LINES
#endif

namespace sobj {

SpecialLinesController::SpecialLinesController(world::World* w) : world_(w) {
  for (auto sec :world_->GetSectors()) {
    if (sec.tag != 0) {
      tag_sectors_[sec.tag].push_back(&sec);
    }
  }
}

void SpecialLinesController::UseLine(world::Line* line, mobj::MapObject* mobj) {
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

    default:
      break;
  }
}

void SpecialLinesController::HitLine(world::Line* l, mobj::MapObject* mobj) {

}

void SpecialLinesController::CrossLine(world::Line* l, mobj::MapObject* mobj) {
  
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
      door.reset(new Door(world_, sec, DoorType::kOpenThenClose, kNormalDoorSpeed, kNormalDoorWaitTime));
      break;

    case 31:
    case 32:
    case 33:
    case 34:
      door.reset(new Door(world_, sec, DoorType::kOpen, kNormalDoorSpeed, 0));
      line->specials = 0;  // one time action
      break;

    case 117:
      door.reset(new Door(world_, sec, DoorType::kOpenThenClose, kBlazeDoorSpeed, kNormalDoorWaitTime));
      break;
    case 118:
      door.reset(new Door(world_, sec, DoorType::kOpen, kBlazeDoorSpeed, 0));
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

void SpecialLinesController::UseTagDoor(world::Line* l, mobj::MapObject* mobj) {

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
