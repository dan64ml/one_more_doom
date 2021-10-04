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

  void UseLine(world::Line* l, mobj::MapObject* mobj);
  void HitLine(world::Line* l, mobj::MapObject* mobj);
  void CrossLine(world::Line* l, mobj::MapObject* mobj);

 private:
  world::World* world_;

  std::unordered_map<int, std::vector<world::Sector*>> tag_sectors_;

  std::list<std::unique_ptr<StructureObject>> sobjs_;

  // For doors without TAG
  void UseManualDoor(world::Line* l, mobj::MapObject* mobj);
  // For doors with TAG, their control lines can have SWITCHes!
  void UseTagDoor(world::Line* l, mobj::MapObject* mobj);

  const int kNormalDoorSpeed = 2;
  const int kBlazeDoorSpeed = 4 * kNormalDoorSpeed;

  const int kNormalDoorWaitTime = 150;
  const int k30SecDoorWaitTime = 35 * 30;
};

} //  namespace sobj

#endif  // SPECIAL_LINES_CONTROLLER_H_