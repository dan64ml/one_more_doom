#ifndef PLATFORM_H_
#define PLATFORM_H_

#include "structure_object.h"
#include "sobj_types.h"
#include "world/world_types.h"

namespace sobj {

enum class PlatformType {
  kRaiseToNearestAndChange,
  kRaiseAndChange,
  kDownWaitUpStay,
  kBlazeDWUS,
  kPerpetualRaise
};

class Platform : public StructureObject {
 public:
  Platform(world::World* w, world::Sector* s, world::Line* l, PlatformType type, int extra_height = 0);
  ~Platform();

  bool TickTime() override;

  bool Trigger(mobj::MapObject*) override { return true; }

  void ActivateInStasis(int tag) override;
  void StopObject(int tag) override;
  
 private:
  const double kPlatformSpeed = 1.0;
  const int kPlatformWait = 3;

  world::World* world_;
  world::Sector* sector_;
  PlatformType type_;

  MoveDirection state_;
  MoveDirection old_state_;

  double speed_;
  double high_pos_;
  double low_pos_;
  
  int wait_;
  int count_;

  bool crush_ = false;
};

} // namespace sobj

#endif  // PLATFORM_H_
