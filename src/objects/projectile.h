#ifndef PROJECTILE_H_
#define PROJECTILE_H_

#include <vector>

#include "map_object.h"
#include "moving_object.h"

namespace mobj {

class Projectile : public MovingObject {
 public:
  Projectile(id::mobjtype_t type, MapObject* parent);

  bool TickTime() override;

  void SetVerticalAngle(rend::BamAngle an);

  void TieToMap(world::World* world, world::SubSector* ss) override;

 protected:
  void SlowDown() override {}
  void ZMove() override;
  bool RunIntoAction(double new_x, double new_y) override;
  bool InfluenceObject(MapObject*) override;
  bool ProcessLine(world::Line* line) override;

  void CallStateFunction(id::FuncId foo_id) override;

 private:
  void BFGSpray();

  const int kRocketBlastDamage = 128;
  const int kBarrelHeight = 32; // original value

  int damage_;
  int blast_damage_;
  
  bool met_obstacle = false;
  bool hit_sky_ = false;
};


} // namespace wpn

#endif  // PROJECTILE_H_
