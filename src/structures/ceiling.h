#ifndef CEILING_H_
#define CEILING_H_

#include "structure_object.h"

#include "sobj_types.h"

#include "world/world_types.h"

namespace sobj {

enum class CeilingType {
  kLowerToFloor,
  kRaiseToHighest,
  kLowerAndCrush,
  kCrushAndRaise,
  kFastCrushAndRaise,
  kSilentCrushAndRaise
};

class Ceiling : public StructureObject {
 public:
  Ceiling(world::World* w, world::Sector* s, world::Line* l, CeilingType type);
  ~Ceiling();

  bool TickTime() override;

  bool Trigger(mobj::MapObject*) override { return true; }

  void StopObject(int tag) override;
  void ActivateInStasis(int tag) override;

private:
  const double kCeilingSpeed = 1.0;

  world::World* world_;
  world::Sector* sector_;
  CeilingType type_;

  double speed_;
  double high_pos_;
  double low_pos_;

  bool crush_ = false;

  MoveDirection direction_;
  MoveDirection old_direction_;

};

} // namespace sobj

#endif  // CEILING_H_
