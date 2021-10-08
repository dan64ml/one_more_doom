#ifndef PLATFORM_H_
#define PLATFORM_H_

#include "structure_object.h"
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

 private:
  enum class PlatformState {
    kUp,
    kDown,
    kWait,
    kSuspend  // Platform can be suspended by using a line
  };

  const int kPlatformSpeed = 1;
  const int kPlatformWait = 3;

  world::World* world_;
  world::Sector* sector_;
  PlatformType type_;

  PlatformState state_;

  int speed_;
  int high_pos_;
  int low_pos_;
  int wait_count_;

  int count_;
};

} // namespace sobj

#endif  // PLATFORM_H_
