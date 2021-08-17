#include "mobj_factory.h"

#include "map_object.h"

namespace id {

std::optional<mobj::MapObject> MobjFactory::Create(int type) {
  if (mobjs_info_.count(type) == 0) {
    return {};
  }

  mobj::MapObject obj(mobjs_info_.at(type));

  return {obj};
}

std::unordered_map<int, id::mobjinfo_t> Fill() {
  std::unordered_map<int, id::mobjinfo_t> ret;
  for (int i = 0; i < NUMMOBJTYPES; ++i) {
    if (mobjinfo[i].doomednum != -1) {
      ret[mobjinfo[i].doomednum] = mobjinfo[i];
    }
  }

  return ret;
}

std::unordered_map<int, id::mobjinfo_t> MobjFactory::mobjs_info_ = Fill();

} // namespace id