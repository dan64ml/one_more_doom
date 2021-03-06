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

#include "flat_wall_animator.h"
#include "structures/special_lines_controller.h"

namespace mobj {
  class MapObject;
}

namespace sobj {
  class SpecialLinesController;
}

namespace world {

class World {
 public:
  World();

  void OpenWad(std::string wad_file);
  void LoadLevel(size_t level);

  void TickTime();

  const wad::FastBsp* GetBsp() const { return &bsp_; }
  std::vector<Sector>& GetSectors() { return sectors_; }
  BlockMap& GetBlocks() { return blocks_; }

  mobj::Player* GetPlayer() { return reinterpret_cast<mobj::Player*>(player_.get()); };

  // Spawn plasma ball or missile.
  void SpawnProjectile(id::mobjtype_t type, mobj::MapObject* parent);
  // Spawn plasma ball or missile.
  void SpawnProjectile(id::mobjtype_t type, mobj::MapObject* source, mobj::MapObject* target);
  // Spawn BFG extra hit
  void SpawnBFGExplode(int x, int y, int z);
  // Spawn a mobj
  void SpawnMapObject(id::mobjtype_t type, int x, int y, uint32_t extra_flags, bool on_floor = true);

  // Line attack with horisontal (dir_angle) and vertical (vert_angle) angles.
  // Line attack has only two mobjtype_t: MT_BLOOD if hit and MT_PUFF if miss.
  // If the weapon in refire state, accuracy decreases and da != 0.
  void HitAngleLineAttack(mobj::MapObject* parent, int damage, int distance, rend::BamAngle dir_angle, rend::BamAngle vert_angle);

  // Looks for a target. Returns vertical angle to the target. Zero if target not found
  rend::BamAngle GetTargetAngle(int from_x, int from_y, int from_z, rend::BamAngle direction, int distance);
  // Returns the target
  mobj::MapObject* GetTarget(int from_x, int from_y, int from_z, rend::BamAngle direction, int distance);

  //void DoBlastDamage(int damage, int x, int y);
  void DoBlastDamage(int damage, mobj::MapObject* inflictor);

  void UseLine(world::Line* line, mobj::MapObject* mobj);
  void HitLine(world::Line* line, mobj::MapObject* mobj);
  void CrossLine(world::Line* line, mobj::MapObject* mobj);

  // Boss's death can be a trigger on some maps
  void BossDeath(id::mobjtype_t type);

   // Some game logic uses it...
  long long tick_counter_ = 0;

  //**************************************************************************
  // Searches the player. If around == true, 360 degrees, else - 180
  mobj::MapObject* LookForPlayer(mobj::MapObject* mobj, bool around);
  
  // Check if there is no obstacle between two mobjs. Optimize!!!
  bool CheckSight(mobj::MapObject* mobj1, mobj::MapObject* mobj2);

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

  std::list<std::unique_ptr<mobj::MapObject>> mobjs_;
  
  FlatWallAnimator flat_animator_;
  
  // Control doors, switches, moving floors etc...
  std::unique_ptr<sobj::SpecialLinesController> spec_lines_controller_;

 private:
  void ClearLevel();
  void LoadSectors(std::ifstream& fin);
  void LoadSideDefs(std::ifstream& fin);
  void LoadLines(std::ifstream& fin, RawVertex* vertexes);
  void LoadSegments(std::ifstream& fin, RawVertex* vertexes);
  void LoadSubSectors(std::ifstream& fin);
  // Loads things from .wad and creates list of MapObjects
  void CreateMapObjectList(std::ifstream& fin);
  // Fills Sector::lines and Sector::subsecs in all sectors.
  void FillSectorLines();

  void PutMobjOnMap(std::unique_ptr<mobj::MapObject> obj, bool keep_z);

  void SpawnBulletPuff(int x, int y, int z);
  void SpawnBulletBlood(int x, int y, int z);

  // Checks if the mobj is reachable for a blast from {vp_x, vp_y}
  bool IsMobjBlastVisible(double vp_x, double vp_y, const mobj::MapObject* obj) const;

 public:
  // Returns sorted list of objects (lines and mobjs) that are projected on the section {from_x, from_y, angle, distance}
  std::vector<IntersectedObject> CreateIntersectedObjList(double from_x, double from_y, rend::BamAngle angle, double distance) const;
  std::vector<IntersectedObject> CreateIntersectedObjList(double from_x, double from_y, double to_x, double to_y) const;

  friend class mobj::MapObject;
  friend class mobj::MovingObject;
};

} // namespace world

#endif  // WORLD_H_
