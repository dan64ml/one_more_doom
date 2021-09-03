#include "mobj_factory.h"

#include "map_object.h"
#include "player.h"

namespace id {

std::optional<mobj::MapObject> MobjFactory::Create(int type) {
  if (mobjs_info_.count(type) == 0) {
    return {};
  }

  mobj::MapObject obj(mobjs_info_.at(type));

  return {obj};
}

std::unique_ptr<mobj::MapObject> MobjFactory::Create(const wad::WadMapThing& thing) {
  if (mobjs_info_.count(thing.type) == 0) {
    return {};
  }

  std::unique_ptr<mobj::MapObject> obj(new mobj::MapObject(mobjs_info_.at(thing.type)));
  obj->x = thing.x;
  obj->y = thing.y;

  obj->angle = rend::DegreesToBam(thing.angle);

  obj->type = thing.type;
  
  return obj;
}

std::unique_ptr<mobj::Player> MobjFactory::CreatePlayer(const wad::WadMapThing& thing) {
  std::unique_ptr<mobj::Player> ret(new mobj::Player(mobjinfo[id::MT_PLAYER]));
  
  ret->x = thing.x;
  ret->y = thing.y;
  
  ret->angle = rend::DegreesToBam(thing.angle);
  ret->type = thing.type;
  
  return ret;
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