#ifndef PROJECTILE_H_
#define PROJECTILE_H_

#include <vector>

#include "objects/map_object.h"

namespace wpn {

class Projectile : public mobj::MapObject {
 public:
  template<class T>
  Projectile(T param);

  bool TickTime();

 private:
  int sprite_idx_ = 0;
  int frame_count_ = 0;
  std::vector<std::pair<std::string, int>> sprites_;
};

template<class T>
Projectile::Projectile(T param) {

}

} // namespace wpn

#endif  // PROJECTILE_H_
