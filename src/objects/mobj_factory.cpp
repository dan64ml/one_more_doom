#include "mobj_factory.h"

#include "map_object.h"
#include "player.h"

#include "trooper.h"
#include "cacodemon.h"
#include "imp.h"

namespace id {

std::unique_ptr<mobj::MapObject> MobjFactory::Create(const wad::WadMapThing& thing) {
  if (mobjs_types_.count(thing.type) == 0) {
    return {};
  }

  std::unique_ptr<mobj::MapObject> obj;

  switch (id::mobjtype_t type = mobjs_types_.at(thing.type); type) {
    case id::MT_POSSESSED:
      return {};
      //if (thing.x != -112) return {};
      obj.reset(new mobj::Trooper(type));
      break;
    case id::MT_HEAD:
      //if (thing.x != 1504) return {};
      obj.reset(new mobj::Cacodemon(type));
      break;
    case id::MT_TROOP:
      //if (thing.x != 1504) return {};
      obj.reset(new mobj::Imp(type));
      break;

    default:
      return {};
      obj.reset(new mobj::MapObject(type));
      break;
  }

  obj->x = thing.x;
  obj->y = thing.y;
  obj->angle = rend::DegreesToBam(thing.angle);

  return obj;
}

std::unique_ptr<mobj::Player> MobjFactory::CreatePlayer(const wad::WadMapThing& thing) {
  std::unique_ptr<mobj::Player> ret(new mobj::Player());
  
//  ret->x = thing.x;
//  ret->y = thing.y;
  ret->x = 1100;
  ret->y = 1100;
  
  ret->angle = rend::DegreesToBam(thing.angle);
  
  return ret;
}

std::unordered_map<int, id::mobjtype_t> Fill() {
  std::unordered_map<int, id::mobjtype_t> ret;
  for (int i = 0; i < NUMMOBJTYPES; ++i) {
    if (mobjinfo[i].doomednum != -1) {
      ret[mobjinfo[i].doomednum] = static_cast<id::mobjtype_t>(i);
    }
  }

  return ret;
}

const std::unordered_map<int, id::mobjtype_t> MobjFactory::mobjs_types_ = Fill();

} // namespace id