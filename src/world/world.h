#ifndef WORLD_H_
#define WORLD_H_

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>

#include "world_types.h"
#include "fast_bsp.h"
#include "block_map.h"

#include "objects/player.h"
#include "objects/mobj_factory.h"
#include "objects/projectile.h"

namespace mobj {
  class MapObject;
}

namespace world {

class World {
 public:
  World();

  void OpenWad(std::string wad_file);
  void LoadLevel(size_t level);

  void TickTime();

  const wad::FastBsp* GetBsp() const { return &bsp_; };

  mobj::Player* GetPlayer() { return reinterpret_cast<mobj::Player*>(player_.get()); };

  // Spawn plasma ball or missile.
  void SpawnProjectile(id::mobjtype_t type, mobj::MapObject* parent);

  void DoBlastDamage(int damage, int x, int y) {}

 private:
  std::string wad_file_name_;
  // Level names and lumps
  std::vector<std::pair<std::string, std::unordered_map<std::string, LumpPos>>> levels_;
  size_t current_level_;

  std::vector<Sector> sectors_;
  std::vector<SideDef> sides_;
  std::vector<Line> lines_;
  std::vector<Segment> segs_;
  std::vector<SubSector> sub_sectors_;

  wad::FastBsp bsp_;
  
  BlockMap blocks_;

  // Player is a special mobj
  std::unique_ptr<mobj::Player> player_;

  id::MobjFactory spawner_;

  // TMP!!!!
  //std::list<mobj::MapObject> mobjs_;
  std::list<std::unique_ptr<mobj::MapObject>> mobjs_;
  
 private:
  void ClearLevel();
  void LoadSectors(std::ifstream& fin);
  void LoadSideDefs(std::ifstream& fin);
  void LoadLines(std::ifstream& fin, RawVertex* vertexes);
  void LoadSegments(std::ifstream& fin, RawVertex* vertexes);
  void LoadSubSectors(std::ifstream& fin);
  // Loads things from .wad and creates list of MapObjects
  void CreateMapObjectList(std::ifstream& fin);

  void PutMobjOnMap(std::unique_ptr<mobj::MapObject> obj);
  //void DeleteMobj(const mobj::MapObject* obj);

  friend class mobj::MapObject;
};

} // namespace world

#endif  // WORLD_H_
