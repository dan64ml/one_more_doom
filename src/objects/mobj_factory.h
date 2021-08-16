#ifndef MOBJ_FACTORY_H_
#define MOBJ_FACTORY_H_

#include <unordered_map>
#include <optional>

#include "info.h"

#include "map_object.h"
#include "world/world_types.h"

namespace id {

class MobjFactory {
 public:
  MobjFactory() = default;

  std::optional<mobj::MapObject> Create(int type, int x, int y, int degree_angle);

 private:
  static std::unordered_map<int, id::mobjinfo_t> mobjs_info_;

};

} // namespace id

#endif