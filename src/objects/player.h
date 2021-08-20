#ifndef PLAYER_H_
#define PLAYER_H_

#include "map_object.h"
#include "renderer/bam.h"

namespace mobj {

class Player : public MapObject {
 public:
  Player(const id::mobjinfo_t& info) : MapObject(info) {}
  
  void Move(rend::BamAngle delta_angle, double forward_move, double side_move);
 
 private:
  virtual bool RunIntoAction() override;
};

} // namespace mobj

#endif  // PLAYER_H_