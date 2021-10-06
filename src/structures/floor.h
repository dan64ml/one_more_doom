#ifndef FLOOR_H_
#define FLOOR_H_

#include "structure_object.h"

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
  kLowerAndChange
};

class Floor : public StructureObject {
 public:
  Floor(world::World* w, world::Sector* s, FloorType type);
  ~Floor();

  bool TickTime() override;

  bool Trigger(mobj::MapObject*) override { return true; }

 private:
  const int kFloorSpeed = 1;
  const int kTurboFloorSpeed = 4 * kFloorSpeed;

  world::World* world_;
  world::Sector* sector_;
  FloorType type_;

  int speed_ = kFloorSpeed;
  int target_height_;
  int direction_;
  bool crush_ = false;  // most floors are harmless
};

} // namespace sobj

#endif  // FLOOR_H_
