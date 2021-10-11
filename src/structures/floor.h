#ifndef FLOOR_H_
#define FLOOR_H_

#include "structure_object.h"
#include "sobj_types.h"
#include "world/world_types.h"

namespace sobj {

enum class FloorType {
  kLowerFloor,
  kLowerFloorToLowest,
  kTurboLower,
  kRaiseFloorCrush,
  kRaiseFloor,
  kRaiseFloorTurbo,
  kRaiseFloorToNearest,
  kRaiseFloor24,
  kRaiseFloor512,
  kRaiseFloor24AndChange,
  kRaiseToTexture,
  kLowerAndChange,

  kDonutRaise // ???
};

class Floor : public StructureObject {
 public:
  Floor(world::World* w, world::Sector* s, world::Line* l, FloorType type);
  ~Floor();

  bool TickTime() override;

  bool Trigger(mobj::MapObject*) override { return true; }

 private:
  const double kFloorSpeed = 1.0;

  world::World* world_;
  world::Sector* sector_;
  FloorType type_;

  double speed_ = kFloorSpeed;
  double target_height_;
  MoveDirection direction_;
  bool crush_ = false;  // most floors are harmless

  int new_special_;
  std::string texture_;
};

} // namespace sobj

#endif  // FLOOR_H_
