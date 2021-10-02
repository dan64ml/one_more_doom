#ifndef DOOR_H_
#define DOOR_H_

#include "structure_object.h"

#include "world/world_types.h"

namespace sobj {

class Door : public StructureObject {
 public:
  Door();

  bool TickTime(world::World* w) override;

 private:
  world::Sector* sector_;

  // Door can have maximum three states: opening(closing)/wait/closing(opening).
  // Or only one, eg open and stay open.
  enum DoorState {
    kMove1 = 0,
    kMove2,
    kWait,
  };

  DoorState current_state_ = DoorState::kMove1;

  // Both moving have the same speed
  int move_speed_;
  // Initial direction. After kWait direction reverses. 1 == move up
  int move_direction_;
  
  // kMove1 and kMove2 should stop on this heights
  int move_stop_level_[2];
  // Pausa length in ticks. -1 means there is no kMove2.
  int wait_counter_;

  // Some door types should wait until obstacles disappear
  // But most doors just return back.
  // bool wait_obstacle = false;

  // Move ceiling. Returns true when ceiling reached stop level
  bool MoveCeiling();

  bool CheckObstacles(int ceiling_height);
};

} // sobj

#endif  // DOOR_H_
