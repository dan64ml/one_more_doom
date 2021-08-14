#ifndef PLAYER_H_
#define PLAYER_H_

#include "map_object.h"
#include "renderer/bam.h"

namespace mobj {

class Player : public MapObject {
 public:
  Player(world::World* world) : MapObject(world) {}
  
  void Move(rend::BamAngle delta_angle, double forward_move, double side_move);
};

} // namespace mobj

#endif  // PLAYER_H_