#ifndef PROJECTILE_H_
#define PROJECTILE_H_

#include <vector>

#include "map_object.h"
#include "weapon/projectile_params.h"

namespace mobj {

class Projectile : public MapObject {
 public:
  Projectile(const wpn::ProjectileParams& param);
  Projectile(const wpn::ProjectileParams& param, const MapObject* parent);

  bool TickTime() override;

 protected:
  void SlowDown() override {}
  void ZMove() override {}
  bool RunIntoAction() override;

 private:
  int sprite_idx_ = 0;
  int frame_count_ = 0;
  //std::vector<std::pair<std::string, int>> sprites_;
};


} // namespace wpn

#endif  // PROJECTILE_H_
