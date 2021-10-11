#ifndef CEILING_H_
#define CEILING_H_

#include "structure_object.h"

#include "world/world_types.h"

namespace sobj {

enum class CeilingType {
  kFastCrushAndRaise,
  kSilentCrushAndRaise,
  kCrushAndRaise,
  kLowerAndCrush,
  kLowerToFloor,
  kRaiseToHighest
};

enum class MoveDirection {
  kUp,
  kDown,
  kWait,
  kSuspend  // Sobj can be suspended by using a line
};

class Ceiling : public StructureObject {
 public:
  Ceiling(world::World* w, world::Sector* s, world::Line* l, CeilingType type);
  ~Ceiling();

  bool TickTime() override;

  bool Trigger(mobj::MapObject*) override { return true; }

 private:
  const int kCeilingSpeed = 1;

  world::World* world_;
  world::Sector* sector_;
  CeilingType type_;

  int speed_;
  int high_pos_;
  int low_pos_;

  bool crush_ = false;

  MoveDirection direction_;

};

} // namespace sobj

#endif  // CEILING_H_
