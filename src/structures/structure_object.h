#ifndef STRUCTURE_OBJECT_H_
#define STRUCTURE_OBJECT_H_

namespace mobj {
  class MapObject;
}

namespace world {
  class World;
}

namespace sobj {

enum SObjStateName {
  kNoState = -1,
  kTopPosition,
  kBottomPosition,
  kMoveUp,
  kMoveDown
};

struct SObjState {
  SObjStateName state;
  int tics;
  SObjStateName next_state;
  SObjStateName trigger_state;
};

// Base class for most map's construction like doors, switches, moving surfaces etc..
// Most of them just do some actions (eg open-wait-close or close-stay close) and then
// must be destroyed. It happens when TickTime() returns false.
class StructureObject {
 public:
  virtual ~StructureObject();

  virtual bool TickTime(world::World* w) = 0;
  virtual bool Trigger(mobj::MapObject*) = 0;
};

} // sobj

#endif  // STRUCTURE_OBJECT_H_
