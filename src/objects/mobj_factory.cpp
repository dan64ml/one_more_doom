#include "mobj_factory.h"

namespace id {

std::optional<mobj::MapObject> MobjFactory::Create(int type, int x, int y, int degree_angle) {
  if (mobjs_info_.count(type) == 0) {
    return {};
  }

  return {};
}

std::unordered_map<int, id::mobjinfo_t> Fill() {
  std::unordered_map<int, id::mobjinfo_t> ret;
  for (int i = 0; i < NUMMOBJTYPES; ++i) {
    if (mobjinfo[i].doomednum != -1) {
      ret[i] = mobjinfo[i];
    }
  }

  return ret;
}

std::unordered_map<int, id::mobjinfo_t> MobjFactory::mobjs_info_ = Fill();

} // namespace id