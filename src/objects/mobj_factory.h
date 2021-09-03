#ifndef MOBJ_FACTORY_H_
#define MOBJ_FACTORY_H_

#include <unordered_map>
#include <optional>
#include <memory>

#include "info.h"

#include "map_object.h"
#include "player.h"
#include "world/world_types.h"
#include "world/wad_raw_types.h"

namespace id {

class MobjFactory {
 public:
  MobjFactory() = default;

  std::optional<mobj::MapObject> Create(int type);
  std::unique_ptr<mobj::MapObject> Create(const wad::WadMapThing& thing);
  std::unique_ptr<mobj::Player> CreatePlayer(const wad::WadMapThing& thing);

 private:
  static std::unordered_map<int, id::mobjinfo_t> mobjs_info_;

};

} // namespace id

#endif