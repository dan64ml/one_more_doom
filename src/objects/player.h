#ifndef PLAYER_H_
#define PLAYER_H_

#include "map_object.h"

namespace mobj {

class Player : public MapObject {
 public:
  Player(world::World* world) : MapObject(world) {}
  
};

} // namespace mobj

#endif  // PLAYER_H_