#ifndef PROJECTILE_H_
#define PROJECTILE_H_

#include <vector>

#include "map_object.h"
#include "weapon/projectile_params.h"

namespace mobj {

class Projectile : public MapObject {
 public:
  Projectile(const wpn::ProjectileParams& param, const MapObject* parent);

  bool TickTime() override;

 protected:
  void SlowDown() override {}
  void ZMove() override {}
  bool RunIntoAction() override;
  bool InfluenceObject(MapObject*) override;
  bool ProcessLine(const world::Line* line) override;

 private:
  int damage_;
  int blast_damage_;
  int speed_;
  
  bool met_obstacle = false;
};


} // namespace wpn

#endif  // PROJECTILE_H_
