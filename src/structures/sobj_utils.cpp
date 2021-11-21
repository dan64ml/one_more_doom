#include "sobj_utils.h"

#include <cassert>

#include "world/world.h"

namespace sobj {

MoveResult MoveCeiling(world::Sector* sec, double speed, double dest, bool crush, MoveDirection dir) {
  double old_pos;

  switch (dir)
  {
    case MoveDirection::kDown:
      if (sec->ceiling_height - speed < dest) {
        old_pos = sec->ceiling_height;
        sec->ceiling_height = dest;
        if (ChangeSectorHeight(sec, crush)) {
          // Hit something, restore sector height
          sec->ceiling_height = old_pos;
          ChangeSectorHeight(sec, crush);
        }
        return MoveResult::kGotDest;
      } else {
        old_pos = sec->ceiling_height;
        sec->ceiling_height -= speed;
        if (ChangeSectorHeight(sec, crush)) {
          if (crush) {
            return MoveResult::kCrushed;
          } else {
            sec->ceiling_height = old_pos;
            ChangeSectorHeight(sec, crush);
            return MoveResult::kCrushed;
          }
        }
      }
      break;
    
    case MoveDirection::kUp:
      if (sec->ceiling_height + speed > dest) {
        old_pos = sec->ceiling_height;
        sec->ceiling_height = dest;
        if (ChangeSectorHeight(sec, crush)) {
          sec->ceiling_height = old_pos;
          ChangeSectorHeight(sec, crush);
        }
        return MoveResult::kGotDest;
      } else {
        old_pos = sec->ceiling_height;
        sec->ceiling_height += speed;
        ChangeSectorHeight(sec, crush);
      }
      break;

    default:
      assert(false);
      break;
  }

  return MoveResult::kOk;
}

MoveResult MoveFloor(world::Sector* sec, double speed, double dest, bool crush, MoveDirection dir) {
  double old_pos;

  switch (dir)
  {
    case MoveDirection::kDown:
      if (sec->floor_height - speed < dest) {
        old_pos = sec->floor_height;
        sec->floor_height = dest;
        if (ChangeSectorHeight(sec, crush)) {
          sec->floor_height = old_pos;
          ChangeSectorHeight(sec, crush);
        }
        return MoveResult::kGotDest;
      } else {
        old_pos = sec->floor_height;
        sec->floor_height -= speed;
        if (ChangeSectorHeight(sec, crush)) {
          sec->floor_height = old_pos;
          ChangeSectorHeight(sec, crush);
          return MoveResult::kCrushed;
        }
      }
      break;

    case MoveDirection::kUp:
      if (sec->floor_height + speed > dest) {
        old_pos = sec->floor_height;
        sec->floor_height = dest;
        if (ChangeSectorHeight(sec, crush)) {
          sec->floor_height = old_pos;
          ChangeSectorHeight(sec, crush);
        }
        return MoveResult::kGotDest;
      } else {
        old_pos = sec->floor_height;
        sec->floor_height += speed;
        if (ChangeSectorHeight(sec, crush)) {
          if (crush) {
            return MoveResult::kCrushed;
          } else {
            sec->floor_height = old_pos;
            ChangeSectorHeight(sec, crush);
            return MoveResult::kCrushed;
          }
        }
      }
      break;

    default:
      assert(false);
      break;
  }

  return MoveResult::kOk;
}

bool ChangeSectorHeight(world::Sector* sec, bool crush) {
  world::BBox bbox {sec->bbox};
  bbox.top += mobj::kMaxRadius;
  bbox.right += mobj::kMaxRadius;
  bbox.left -= mobj::kMaxRadius;
  bbox.bottom -= mobj::kMaxRadius;
  auto mobj_list = sec->world->GetBlocks().GetMapObjects(bbox);

  bool result = false;
  for (auto mobj : mobj_list) {
    if (mobj->AdjustMobjHeight()) {
      continue;
    }

    if (mobj->GetHealth() <= 0) {
      // Dead stuff should be destroy anyway
      mobj->DamageBySobj(10);
      continue;
    }

    if (mobj->flags & mobj::MF_DROPPED) {
      // Such things should be removed
      mobj->DamageBySobj(10);
      continue;
    }

    if (!(mobj->flags & mobj::MF_SHOOTABLE)) {
      // Uninteractable
      continue;
    }

    result = true;

    if (crush && !(sec->world->tick_counter_ & 3)) {
      mobj->CauseDamage(10, nullptr, nullptr);
      // TODO: Add blood spray
    }
  }

  return result;
}

} // namespace sobj
