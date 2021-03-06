#ifndef SPECIAL_LINES_CONTROLLER_H_
#define SPECIAL_LINES_CONTROLLER_H_

#include <unordered_map>
#include <vector>
#include <list>
#include <memory>

#include "world/world_types.h"
#include "structure_object.h"

namespace sobj {

class SpecialLinesController {
 public:
  SpecialLinesController(world::World* w);

  void TickTime();

  // Main foo for "using" line (use button)
  void UseLine(world::Line* l, mobj::MapObject* mobj);
  // For crossing line
  void HitLine(world::Line* l, mobj::MapObject* mobj);
  // For "hitting" line (can be also shooted by some weapon)
  void CrossLine(world::Line* l, mobj::MapObject* mobj);

 private:
  world::World* world_;

  std::unordered_map<int, std::vector<world::Sector*>> tag_sectors_;

  std::list<std::unique_ptr<StructureObject>> sobjs_;

  // For doors without TAG
  void UseManualDoor(world::Line* line, mobj::MapObject* mobj);
  // For doors with TAG, their control lines can have SWITCHes!
  void UseTagDoor(world::Line* line, mobj::MapObject* mobj);
  // For LOCKED doors with TAG
  void UseLockedTagDoor(world::Line* line, mobj::MapObject* mobj);
  // Activate moving floors
  void UseFloor(world::Line* line, [[maybe_unused]] mobj::MapObject* mobj);
  // Activate platforms
  void UsePlatform(world::Line* line, [[maybe_unused]] mobj::MapObject* mobj);
  // Activate ceiling
  void UseCeiling(world::Line* line, [[maybe_unused]] mobj::MapObject* mobj);
  // Stops the platform
  void StopPlatform(world::Line* line);
};

} //  namespace sobj

#endif  // SPECIAL_LINES_CONTROLLER_H_