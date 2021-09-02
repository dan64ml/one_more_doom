#ifndef PROJECTILE_PARAMS_H_
#define PROJECTILE_PARAMS_H_

#include "objects/info.h"

namespace wpn {

struct ProjectileParams {
  int damage;
  int speed;

  id::mobjtype_t type;
};

} // namespace wpn

#endif  // PROJECTILE_PARAMS_H_
