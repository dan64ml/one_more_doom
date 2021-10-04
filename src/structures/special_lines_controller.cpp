#include "special_lines_controller.h"

#include "world/world.h"
#include "door.h"

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
  auto door = std::unique_ptr<Door>(new Door(world_, sec, DoorType::kOpenThenClose, 4));
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
