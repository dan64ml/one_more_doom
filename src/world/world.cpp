#include "world.h"

#include <fstream>
#include <exception>
#include <memory>
#include <algorithm>
#include <iostream>
#include <cassert>
#include <limits>

#include "wad_utils.h"
#include "wad_raw_types.h"
#include "renderer/bam.h"
#include "utils/world_utils.h"
#include "renderer/plane_utils.h"

namespace world {

World::World() {

}

void World::OpenWad(std::string wad_file) {
  // Throw exceptions if error
  std::ifstream fin;
  fin.exceptions(fin.exceptions() | std::ios_base::badbit 
      | std::ios_base::failbit | std::ios_base::eofbit);

  fin.open(wad_file, std::ios::in | std::ios::binary);

  wad::WadHeader header;
  fin.read(reinterpret_cast<char*>(&header), sizeof(wad::WadHeader));
  if (!wad::strcmp<4>(header.magic, "IWAD")) {
    throw std::runtime_error("Unknown file format");
  }

  int dir_size = header.num_directories * sizeof(wad::WadDirectoryEntry);
  std::unique_ptr<char[]> buf(new char[dir_size]);

  fin.seekg(header.directory_offset);
  fin.read(buf.get(), dir_size);

  wad::WadDirectoryEntry* raw_dir = reinterpret_cast<wad::WadDirectoryEntry*>(buf.get());
  for (int i = 0; i < header.num_directories; ++i) {
    if (wad::strcmp<3>(raw_dir[i].name, "MAP")) {
      levels_.push_back({wad::to_string<8>(raw_dir[i].name), {}});

      for (int lump = 1; lump <= 10; ++lump) {
        levels_.back().second[wad::to_string<8>(raw_dir[i + lump].name)] 
            = {raw_dir[i + lump].offset, raw_dir[i + lump].size};
      }

      i += 10;
    }
  }

  if (levels_.size() == 0) {
    throw std::runtime_error("Map level lumps not found!");
  }

  wad_file_name_ = wad_file;
}

void World::LoadLevel(size_t level) {
  if (level >= levels_.size()) {
    throw std::runtime_error("Level #" + std::to_string(level) + " not found!");
  }

  current_level_ = level;

  ClearLevel();

  std::ifstream fin;
  fin.exceptions(fin.exceptions() | std::ios_base::badbit 
      | std::ios_base::failbit | std::ios_base::eofbit);

  fin.open(wad_file_name_, std::ios::in | std::ios::binary);

  const auto vertex_lump = levels_[level].second.at("VERTEXES");
  auto vertexes = wad::LoadLump<RawVertex>(fin, vertex_lump.position, vertex_lump.size);

  // The order is important!
  LoadSectors(fin);
  LoadSideDefs(fin);
  LoadLines(fin, vertexes.get());
  LoadSegments(fin, vertexes.get());
  LoadSubSectors(fin);

  FillSectorLines();

  const auto bsp_lump = levels_[level].second.at("NODES");
  bsp_.LoadBsp(fin, bsp_lump.position, bsp_lump.size);
  bsp_.SetSubSectors(&sub_sectors_);

  const auto block_map_lump = levels_[level].second.at("BLOCKMAP");
  blocks_.Load(fin, block_map_lump.position, block_map_lump.size, lines_);

  CreateMapObjectList(fin);

  flat_animator_.CreateFlatsList(sectors_);
  flat_animator_.CreateWallsList(sides_);
  flat_animator_.CreateScrollingWallsList(lines_);

  spec_lines_controller_.reset(new sobj::SpecialLinesController(this));
}

void World::TickTime() {
  ++tick_counter_;

  player_->TickTime();

  flat_animator_.TickTime();
  spec_lines_controller_->TickTime();
  
  for (auto it = begin(mobjs_); it != end(mobjs_);) {
    if ((*it)->TickTime()) {
      ++it;
    } else {
      // delete the mobj
      auto obj = it->get();
      obj->ss_->mobjs.remove(obj);
      blocks_.DeleteMapObject(obj);
      it = mobjs_.erase(it);
    }
  }
}

void World::ClearLevel() {
  sectors_.clear();
  sides_.clear();
  lines_.clear();
  segs_.clear();
  sub_sectors_.clear();
}

void World::LoadSectors(std::ifstream& fin) {
  auto lump = levels_[current_level_].second.at("SECTORS");
  size_t len = lump.size / sizeof(wad::WadSector);

  auto sectors = wad::LoadLump<wad::WadSector>(fin, lump.position, lump.size);
  sectors_.reserve(len);
  for (size_t i = 0; i < len; ++i) {
    Sector sec;
    sec.floor_height = sectors[i].floor_height;
    sec.ceiling_height = sectors[i].ceiling_height;
  
    sec.floor_pic = wad::to_string<8>(sectors[i].floor_pic);
    sec.ceiling_pic = wad::to_string<8>(sectors[i].ceiling_pic);
  
    sec.light_level = sectors[i].light_level;
    sec.special = sectors[i].special;
    sec.tag = sectors[i].tag;

    sec.has_sobj = false;

    sectors_.push_back(std::move(sec));
  }
}

void World::LoadSideDefs(std::ifstream& fin) {
  auto lump = levels_[current_level_].second.at("SIDEDEFS");
  size_t len = lump.size / sizeof(wad::WadSideDef);

  auto sides = wad::LoadLump<wad::WadSideDef>(fin, lump.position, lump.size);
  sides_.reserve(len);
  for (size_t i = 0; i < len; ++i) {
    SideDef side;
    side.texture_offset = sides[i].texture_offset;
    side.row_offset = sides[i].row_offset;

    side.top_texture = wad::to_string<8>(sides[i].top_texture);
    side.bottom_texture = wad::to_string<8>(sides[i].bottom_texture);
    side.mid_texture = wad::to_string<8>(sides[i].mid_texture);

    side.sector = &sectors_[sides[i].sector];

    sides_.push_back(std::move(side));
  }
}

void World::LoadLines(std::ifstream& fin, RawVertex* vertexes) {
  auto lump = levels_[current_level_].second.at("LINEDEFS");
  size_t len = lump.size / sizeof(wad::WadLineDef);
  
  auto lines = wad::LoadLump<wad::WadLineDef>(fin, lump.position, lump.size);
  lines_.reserve(len);
  for (size_t i = 0; i < len; ++i) {
    Line line;
    line.x1 = vertexes[lines[i].v1].x;
    line.y1 = vertexes[lines[i].v1].y;
    line.x2 = vertexes[lines[i].v2].x;
    line.y2 = vertexes[lines[i].v2].y;

    line.flags = lines[i].flags;
    line.specials = lines[i].specials;
    line.tag = lines[i].tag;

    line.sides[0] = &sides_[lines[i].sidenum[0]];
    line.sides[1] = (lines[i].sidenum[1] == -1) ? nullptr : &sides_[lines[i].sidenum[1]];

    if (line.x1 > line.x2) {
      line.bbox.left = line.x2;
      line.bbox.right = line.x1;
    } else {
      line.bbox.left = line.x1;
      line.bbox.right = line.x2;
    }
    if (line.y1 > line.y2) {
      line.bbox.bottom = line.y2;
      line.bbox.top = line.y1;
    } else {
      line.bbox.bottom = line.y1;
      line.bbox.top = line.y2;
    }

    if (line.specials) {
      std::cout << "Specials = " << line.specials << ", tag = " << line.tag << std::endl;
    }

    lines_.push_back(std::move(line));
  }
}

void World::LoadSegments(std::ifstream& fin, RawVertex* vertexes) {
  auto lump = levels_[current_level_].second.at("SEGS");
  size_t len = lump.size / sizeof(wad::WadBspSegment);
  
  auto segments = wad::LoadLump<wad::WadBspSegment>(fin, lump.position, lump.size);
  segs_.reserve(len);
  for (size_t i = 0; i < len; ++i) {
    Segment seg;
    seg.x1 = vertexes[segments[i].v1].x;
    seg.y1 = vertexes[segments[i].v1].y;
    seg.x2 = vertexes[segments[i].v2].x;
    seg.y2 = vertexes[segments[i].v2].y;

    seg.angle = segments[i].angle;
    seg.offset = segments[i].offset;

    seg.linedef = &lines_[segments[i].linedef];
    //seg.side = seg.linedef->sides[segments[i].side];
    seg.side = segments[i].side;

    segs_.push_back(std::move(seg));
  }
}

void World::LoadSubSectors(std::ifstream& fin) {
  auto lump = levels_[current_level_].second.at("SSECTORS");
  size_t len = lump.size / sizeof(wad::WadBspSubSector);
  
  auto ss = wad::LoadLump<wad::WadBspSubSector>(fin, lump.position, lump.size);
  sub_sectors_.reserve(len);
  for (size_t i = 0; i < len; ++i) {
    SubSector subsec;
    subsec.segs.reserve(ss[i].numsegs);
    for (int j = 0; j < ss[i].numsegs; j++) {
      subsec.segs.push_back(&segs_[ss[i].firstseg + j]);
    }

    const Segment* seg = subsec.segs.front(); 
    subsec.sector = seg->linedef->sides[seg->side]->sector;

    sub_sectors_.push_back(std::move(subsec));
  }
}

void World::CreateMapObjectList(std::ifstream& fin) {
  auto lump = levels_[current_level_].second.at("THINGS");
  size_t len = lump.size / sizeof(wad::WadMapThing);

  auto items = wad::LoadLump<wad::WadMapThing>(fin, lump.position, lump.size);
  for (size_t i = 0; i < len; ++i) {
    if (items[i].type == 1) {
      // create player
      player_ = spawner_.CreatePlayer(items[i]);

      int ss_idx = bsp_.GetSubSectorIdx(player_->x, player_->y);
      player_->TieToMap(this, &sub_sectors_[ss_idx]);

      sub_sectors_[ss_idx].mobjs.push_back(player_.get());
      blocks_.AddMapObject(player_.get());
    } else {
      auto mobj = spawner_.Create(items[i]);
      if (mobj) {
        PutMobjOnMap(std::move(mobj), false);
      }
    }
  }
}

void World::PutMobjOnMap(std::unique_ptr<mobj::MapObject> obj, bool keep_z) {
  int ss_idx = bsp_.GetSubSectorIdx(obj->x, obj->y);
  obj->TieToMap(this, &sub_sectors_[ss_idx], keep_z);

  if (!(obj->flags & mobj::MF_NOSECTOR)) {
    sub_sectors_[ss_idx].mobjs.push_back(obj.get());
  }

  if (!(obj->flags & mobj::MF_NOBLOCKMAP)) {
    blocks_.AddMapObject(obj.get());
  }

  mobjs_.push_back(std::move(obj));
}

void World::SpawnProjectile(id::mobjtype_t type, mobj::MapObject* parent) {
  rend::BamAngle angle = GetTargetAngle(parent->x, parent->y, parent->z + mobj::kWeaponHeight, parent->angle, 1024);
  auto proj = std::unique_ptr<mobj::MapObject>(new mobj::Projectile(type, parent, angle));
  PutMobjOnMap(std::move(proj), true);
}

void World::SpawnMapObject(id::mobjtype_t type, int x, int y, uint32_t extra_flags, bool on_floor) {
  auto new_mobj = std::unique_ptr<mobj::MapObject>(new mobj::MapObject(type));
  new_mobj->x = x;
  new_mobj->y = y;
  new_mobj->flags |= extra_flags;
  PutMobjOnMap(std::move(new_mobj), !on_floor);
}

void World::DoBlastDamage(int damage, int x, int y) {
  BBox bb {x - damage, x + damage, y + damage, y - damage};
  for (auto obj :blocks_.GetMapObjects(bb)) {
    if (!(obj->flags & mobj::MF_SHOOTABLE)) {
      continue;
    }
    // This monsters can't be hit by blast
    if (obj->mobj_type == id::MT_CYBORG || obj->mobj_type == id::MT_SPIDER) {
      continue;
    }

    int dx = abs(x - obj->x);
    int dy = abs(y - obj->y);
    // Original algorithm uses max projection instead real distance. Keep it.
    int dist = std::max(dx, dy);
    dist = std::max<int>(0, dist - obj->radius);
    // The mobj too far
    if (dist >= damage) {
      continue;
    }

    if (IsMobjBlastVisible(x, y, obj)) {
      obj->CauseDamage(damage - dist);
    }
  }
}

bool World::IsMobjBlastVisible(double vp_x, double vp_y, const mobj::MapObject* obj) const {
  auto crossed_objects = CreateIntersectedObjList(vp_x, vp_y, obj->x, obj->y);

  for (const auto item : crossed_objects) {
    int idx = item.obj.index();

    if (idx == 0) {
      const auto line = std::get<0>(item.obj);
      if (!(line->flags & kLDFTwoSided)) {
        return false;
      }

      if (line->sides[1]->sector->ceiling_height <= line->sides[1]->sector->floor_height) {
        // Portal is closed
        return false;
      }
    } else {
      const auto mobj = std::get<1>(item.obj);
      if (mobj == obj) {
        return true;
      } else {
        return false;
      }
    }
  }

  return true;
}

std::vector<IntersectedObject> World::CreateIntersectedObjList(double from_x, double from_y, 
                                                               rend::BamAngle angle, double distance) const {
  double to_x = from_x + distance * rend::BamCos(angle);
  double to_y = from_y + distance * rend::BamSin(angle);

  return CreateIntersectedObjList(from_x, from_y, to_x, to_y);
}

std::vector<IntersectedObject> World::CreateIntersectedObjList(double from_x, double from_y, double to_x, double to_y) const {
  std::vector<IntersectedObject> result;

  BBox bb {from_x, to_x, to_y, from_y};
  if (bb.left > bb.right) {
    std::swap(bb.left, bb.right);
  }
  if (bb.top < bb.bottom) {
    std::swap(bb.top, bb.bottom);
  }

  // Scan objects
  for (auto obj : blocks_.GetMapObjects(bb)) {
    if (!(obj->flags & mobj::MF_SHOOTABLE)) {
      continue;
    }

    auto [cross, cx, cy] = math::GetMobjIntersection(from_x, from_y, to_x, to_y, obj);
    if (cross) {
      double dist = rend::SegmentLength(from_x, from_y, cx, cy);
      result.push_back(IntersectedObject {dist, cx, cy, obj});
    }
  }

  // Scan lines
  for (auto line : blocks_.GetLines(bb)) {
    auto [cross, cx, cy] = math::GetSegmentsIntersection(from_x, from_y, to_x, to_y, line);
    if (cross) {
      double dist = rend::SegmentLength(from_x, from_y, cx, cy);
      result.push_back(IntersectedObject {dist, cx, cy, line});
    }
  }

  std::sort(begin(result), end(result), [](const auto& lhs, const auto& rhs) { 
            return lhs.distance < rhs.distance; });

  return result;
}

void World::HitAngleLineAttack(mobj::MapObject* parent, int damage, int distance,
                               rend::BamAngle dir_angle, rend::BamAngle vert_angle) {
  auto crossed_objects = CreateIntersectedObjList((int)parent->x, (int)parent->y, dir_angle, (int)distance);

  double height_coef = rend::BamSin(vert_angle);
  int view_line_z = parent->z + mobj::kWeaponHeight; // Weapon height, TODO!!!

  for (auto elem : crossed_objects) {
    int idx = elem.obj.index();

    if (idx == 0) {
      auto line = std::get<0>(elem.obj);
      //std::cout << "Hit line (" << line->x1 << ", " << line->y1 << ") -> (" << line->x2 << ", " << line->y2 << ")" << std::endl;

      int hit_line_height = view_line_z + elem.distance * height_coef;
      // Check for hitting the sky
      if (hit_line_height > line->sides[0]->sector->ceiling_height && 
          line->sides[0]->sector->ceiling_pic == "F_SKY1") {
        return;
      }

      // Solid wall
      if (!(line->flags & kLDFTwoSided)) {
        auto [x, y] = math::ShiftToCenter(parent->x, parent->y, elem.x, elem.y, 2);
        SpawnBulletPuff(x, y, hit_line_height);
        if (line->specials) {
          HitLine(line, parent);
        }
        return;
      }

      int opening_bottom = std::max(line->sides[0]->sector->floor_height, line->sides[1]->sector->floor_height);
      int opening_top = std::min(line->sides[0]->sector->ceiling_height, line->sides[1]->sector->ceiling_height);

      if (hit_line_height < opening_bottom || hit_line_height > opening_top) {
        if (line->sides[0]->sector->ceiling_pic == "F_SKY1" &&
            line->sides[1]->sector->ceiling_pic == "F_SKY1" &&
            line->sides[0]->sector->ceiling_height > line->sides[1]->sector->ceiling_height) {
          // It's the sky
          return;
        }

        auto [x, y] = math::ShiftToCenter(parent->x, parent->y, elem.x, elem.y, 2);
        SpawnBulletPuff(x, y, hit_line_height);
        if (line->specials) {
          HitLine(line, parent);
        }
        return;
      }
    } else if (idx == 1) {
      // MapObject
      auto mobj = std::get<1>(elem.obj);
      std::cout << "Hit mobj (" << mobj->x << ", " << mobj->y << ")" << std::endl;
      if (mobj == parent) {
        // Don't hit yourself
        continue;
      }

      int hit_line_height = view_line_z + elem.distance * height_coef;

      if (!((mobj->z > hit_line_height) || (mobj->z + mobj->height < hit_line_height))) {
        mobj->CauseDamage(damage);

        auto [x, y] = math::ShiftToCenter(parent->x, parent->y, elem.x, elem.y, 2);
        if (mobj->flags & mobj::MF_NOBLOOD) {
          SpawnBulletPuff(x, y, hit_line_height);
        } else {
          SpawnBulletBlood(x, y, hit_line_height);
        }

        return;
      }
    }
  }
}

rend::BamAngle World::GetTargetAngle(int from_x, int from_y, int from_z, rend::BamAngle direction, int distance) {
  auto crossed_objects = CreateIntersectedObjList(from_x, from_y, direction, distance);

  Opening op;
  op.view_line_z = from_z;

  for (auto elem : crossed_objects) {
    int idx = elem.obj.index();

    if (idx == 0) {
      auto line = std::get<0>(elem.obj);
      //std::cout << "Hit line (" << line->x1 << ", " << line->y1 << ") -> (" << line->x2 << ", " << line->y2 << ")" << std::endl;

      // Solid wall
      if (!(line->flags & kLDFTwoSided)) {
        return 0;
      }

      bool is_open = math::CorrectOpening(op, line, elem.distance);
      if (!is_open) {
        return 0;
      }
    } else if (idx == 1) {
      // MapObject
      auto mobj = std::get<1>(elem.obj);
      if (mobj->mobj_type == id::MT_PLAYER) {
        // Ignore yourself
        continue;
      }
      //std::cout << "Hit mobj (" << mobj->x << ", " << mobj->y << ")" << std::endl;

      int high_z = op.view_line_z + elem.distance * op.coef_high_opening;
      int low_z = op.view_line_z - elem.distance * op.coef_low_opening;

      if (!((mobj->z > high_z) || (mobj->z + mobj->height < low_z))) {
        // Found the target
        int visible_top = std::min<int>(mobj->z + mobj->height, high_z);
        int visible_bottom = std::max<int>(mobj->z, low_z);

        double target_center = (visible_top + visible_bottom) / 2;

        return rend::BamArcSin((target_center - op.view_line_z) / elem.distance);
      }
    }
  }

  return 0;
}

mobj::MapObject* World::GetTarget(int from_x, int from_y, int from_z, rend::BamAngle direction, int distance) {
  auto crossed_objects = CreateIntersectedObjList(from_x, from_y, direction, distance);

  Opening op;
  op.view_line_z = from_z;

  for (auto elem : crossed_objects) {
    int idx = elem.obj.index();

    if (idx == 0) {
      auto line = std::get<0>(elem.obj);

      // Solid wall
      if (!(line->flags & kLDFTwoSided)) {
        return nullptr;
      }

      bool is_open = math::CorrectOpening(op, line, elem.distance);
      if (!is_open) {
        return nullptr;
      }
    } else if (idx == 1) {
      // MapObject
      auto mobj = std::get<1>(elem.obj);
      if (mobj->mobj_type == id::MT_PLAYER) {
        // Ignore yourself
        continue;
      }

      int high_z = op.view_line_z + elem.distance * op.coef_high_opening;
      int low_z = op.view_line_z - elem.distance * op.coef_low_opening;

      if (!((mobj->z > high_z) || (mobj->z + mobj->height < low_z))) {
        // Found the target
        return mobj;
      }
    }
  }

  return nullptr;
}

void World::SpawnBulletPuff(int x, int y, int z) {
  std::unique_ptr<mobj::MapObject> bullet(new mobj::MapObject(id::MT_PUFF));

  bullet->x = x;
  bullet->y = y;
  bullet->z = z;
  bullet->flags |= mobj::MF_NOGRAVITY;

  PutMobjOnMap(std::move(bullet), true);
}

void World::SpawnBulletBlood(int x, int y, int z) {
  std::unique_ptr<mobj::MapObject> bullet(new mobj::MapObject(id::MT_BLOOD));

  bullet->x = x;
  bullet->y = y;
  bullet->z = z;
  bullet->flags |= mobj::MF_NOGRAVITY;

  PutMobjOnMap(std::move(bullet), true);
}

void World::SpawnBFGExplode(int x, int y, int z) {
  std::unique_ptr<mobj::MapObject> bfg(new mobj::MapObject(id::MT_EXTRABFG));

  bfg->x = x;
  bfg->y = y;
  bfg->z = z;
//  bfg->flags |= mobj::MF_NOGRAVITY;

  PutMobjOnMap(std::move(bfg), true);
}

void World::UseLine(world::Line* line, mobj::MapObject* mobj) {
  spec_lines_controller_->UseLine(line, mobj);
}

void World::HitLine(world::Line* line, mobj::MapObject* mobj) {
  spec_lines_controller_->HitLine(line, mobj);
}

void World::CrossLine(world::Line* line, mobj::MapObject* mobj) {
  spec_lines_controller_->CrossLine(line, mobj);
}

void World::FillSectorLines() {
  std::unordered_map<Sector*, int> ptr_to_idx;
  for (size_t i = 0; i < sectors_.size(); ++i) {
    ptr_to_idx[&sectors_[i]] = i;
  }

  for (auto& line : lines_) {
    assert(ptr_to_idx.count(line.sides[0]->sector));
    int sec_idx = ptr_to_idx[line.sides[0]->sector];
    sectors_[sec_idx].lines.push_back(&line);
    if (line.sides[1]) {
      sec_idx = ptr_to_idx[line.sides[1]->sector];
      sectors_[sec_idx].lines.push_back(&line);
    }
  }

  for (auto& ssec : sub_sectors_) {
    assert(ptr_to_idx.count(ssec.sector));
    int sec_idx = ptr_to_idx[ssec.sector];
    sectors_[sec_idx].subsecs.push_back(&ssec);
  }

  // Add BBox data
  for (auto& sec : sectors_) {
    sec.world = this;
    sec.bbox.top = sec.bbox.right = std::numeric_limits<int>::min();
    sec.bbox.bottom = sec.bbox.left = std::numeric_limits<int>::max();
    for (const auto line : sec.lines) {
      sec.bbox.AddPoint(line->x1, line->y1);
      sec.bbox.AddPoint(line->x2, line->y2);
    }
  }
}

mobj::MapObject* World::LookForPlayer(mobj::MapObject* mobj, bool around) {
  if (!player_ || player_->GetHealth() <= 0) {
    return nullptr;
  }

  // Check angle
  if (!around) {
    double dist = rend::SegmentLength(mobj->x, mobj->y, player_->x, player_->y);
    
    auto dir_angle = rend::CalcAngle(mobj->x, mobj->y, player_->x, player_->y);
    auto da = dir_angle - mobj->angle;
    // If the player is behind the monster but on distance of melee attack, attack anyway
    if (da > rend::kBamAngle90 && da < rend::kBamAngle270 && dist > mobj::kMeleeRange) {
      return nullptr;
    }
  }

  if (CheckSight(mobj, player_.get())) {
    return player_.get();
  } else {
    return nullptr;
  }
}

bool World::CheckSight(mobj::MapObject* mobj1, mobj::MapObject* mobj2) {
  if (!mobj1 || !mobj2 || (mobj1 == mobj2)) {
    return false;
  }

  auto items = CreateIntersectedObjList(mobj1->x, mobj1->y, mobj2->x, mobj2->y);

  double dist = rend::SegmentLength(mobj1->x, mobj1->y, mobj2->x, mobj2->y);
  double eye_pos = mobj1->z + 0.75 * mobj1->height;

  for (auto item : items) {
    int idx = item.obj.index();

    if (idx == 0) {
      auto line = std::get<0>(item.obj);
      if (!line->sides[1]) {
        // Wall
        return false;
      } else {
        // Check opening
        int low_los = eye_pos + item.distance * (mobj2->z - eye_pos) / dist;
        int high_los = eye_pos + item.distance * (mobj2->z + mobj2->height - eye_pos) / dist;

        if (low_los > std::min(line->sides[0]->sector->ceiling_height, 
                               line->sides[1]->sector->ceiling_height)) {
          return false;
        }
        if (high_los < std::max(line->sides[0]->sector->floor_height, 
                               line->sides[1]->sector->floor_height)) {
          return false;
        }
      }
    } else if (idx == 1) {
      // Can a monster see throgh another mobjs?!
      auto mobj = std::get<1>(item.obj);
      if (mobj == mobj2) {
        return true;
      }
    }
  }

  return false;
}

} // namespace world
