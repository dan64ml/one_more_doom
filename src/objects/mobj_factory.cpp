#include "mobj_factory.h"

#include "map_object.h"
#include "player.h"

#include "trooper.h"
#include "cacodemon.h"
#include "imp.h"
#include "mancubus.h"
#include "shotguy.h"
#include "heavy_dude.h"
#include "demon.h"
#include "revenant.h"
#include "archvile.h"

namespace id {

std::unique_ptr<mobj::MapObject> MobjFactory::Create(const wad::WadMapThing& thing) {
  if (mobjs_types_.count(thing.type) == 0) {
    return {};
  }

  std::unique_ptr<mobj::MapObject> obj;

  switch (id::mobjtype_t type = mobjs_types_.at(thing.type); type) {
    case id::MT_POSSESSED:
      //return {};
      obj.reset(new mobj::Trooper(type));
      break;
    case id::MT_HEAD:
      //return {};
      obj.reset(new mobj::Cacodemon(type));
      break;
    case id::MT_TROOP:
      //return {};
      obj.reset(new mobj::Imp(type));
      break;
    case id::MT_FATSO:
      //if (thing.x != -320) return {};
      obj.reset(new mobj::Mancubus(type));
      break;
    case id::MT_SHOTGUY:
      //if (thing.x != -320) return {};
      obj.reset(new mobj::ShotGuy(type));
      break;
    case id::MT_CHAINGUY:
      //if (thing.x != -320) return {};
      obj.reset(new mobj::HeavyDude(type));
      break;
    case id::MT_SERGEANT:
    case id::MT_SHADOWS:
      //if (thing.x != 1184) return {};
      obj.reset(new mobj::Demon(type));
      break;
    case id::MT_UNDEAD:
      //if (thing.x != 1184) return {};
      obj.reset(new mobj::Revenant(type));
      break;
    case id::MT_VILE:
      //if (thing.x != 1184) return {};
      obj.reset(new mobj::ArchVile(type));
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
  
  ret->x = thing.x;
  ret->y = thing.y;
//  ret->x = 1100;
//  ret->y = 1100;
  
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