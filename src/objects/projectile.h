#ifndef PROJECTILE_H_
#define PROJECTILE_H_

#include <vector>

#include "map_object.h"
#include "moving_object.h"

namespace mobj {

class Projectile : public MovingObject {
 public:
  Projectile(id::mobjtype_t type, MapObject* parent, rend::BamAngle vert_angle);

  bool TickTime() override;

 protected:
  void SlowDown() override {}
  void ZMove() override;
  bool RunIntoAction(double new_x, double new_y) override;
  bool InfluenceObject(MapObject*) override;
  bool ProcessLine(world::Line* line) override;

  void CallStateFunction(id::FuncId foo_id) override;

 private:
  void BFGSpray();

  int damage_;
  int blast_damage_;
  
  bool met_obstacle = false;
  bool hit_sky_ = false;
};

} // namespace wpn

#endif  // PROJECTILE_H_
