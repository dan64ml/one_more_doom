#include "world.h"

#include <fstream>
#include <exception>
#include <memory>
#include <algorithm>
#include <iostream>
#include <cassert>

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
      obj->ss->mobjs.remove(obj);
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
      player_->floor_z = player_->z = sub_sectors_[ss_idx].sector->floor_height;
      player_->ss = &sub_sectors_[ss_idx];

      player_->angle = rend::DegreesToBam(items[i].angle);
      player_->world_ = this;

      //sub_sectors_[ss_idx].mobjs.push_back(obj.get());
    } else {
      auto mobj = spawner_.Create(items[i]);
      if (mobj) {
        // TODO: move??
        PutMobjOnMap(std::move(mobj), true);
      }
    }
  }
}

void World::PutMobjOnMap(std::unique_ptr<mobj::MapObject> obj, bool put_on_floor) {
  int ss_idx = bsp_.GetSubSectorIdx(obj->x, obj->y);
  obj->ss = &sub_sectors_[ss_idx];

  obj->floor_z = sub_sectors_[ss_idx].sector->floor_height;
  if (put_on_floor) {
    // // Don't put projectiles on floor
    obj->z = obj->floor_z;
  }

  obj->world_ = this;

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

  auto proj = std::unique_ptr<mobj::MapObject>(new mobj::Projectile(type, parent));
  //proj-> SetVerticalAngle(angle);
  proj->mom_z = proj->speed * rend::BamSin(angle);
  PutMobjOnMap(std::move(proj), false);
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
    dist = std::max(0, dist - obj->radius);
    // The mobj too far
    if (dist >= damage) {
      continue;
    }

    if (IsMobjBlastVisible(x, y, obj)) {
      obj->CauseDamage(damage - dist);
    }
  }
}

bool World::IsMobjBlastVisible(int vp_x, int vp_y, const mobj::MapObject* obj) const {
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

std::vector<IntersectedObject> World::CreateIntersectedObjList(int from_x, int from_y, 
                                                               rend::BamAngle angle, int distance) const {
  int to_x = from_x + distance * rend::BamCos(angle);
  int to_y = from_y + distance * rend::BamSin(angle);

  return CreateIntersectedObjList(from_x, from_y, to_x, to_y);
}

std::vector<IntersectedObject> World::CreateIntersectedObjList(int from_x, int from_y, int to_x, int to_y) const {
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
      result.push_back(IntersectedObject {dist, static_cast<int>(cx), static_cast<int>(cy), obj});
    }
  }

  // Scan lines
  for (auto line : blocks_.GetLines(bb)) {
    auto [cross, cx, cy] = math::GetSegmentsIntersection(from_x, from_y, to_x, to_y, line);
    if (cross) {
      double dist = rend::SegmentLength(from_x, from_y, cx, cy);
      result.push_back(IntersectedObject {dist, static_cast<int>(cx), static_cast<int>(cy), line});
    }
  }

  std::sort(begin(result), end(result), [](const auto& lhs, const auto& rhs) { 
            return lhs.distance < rhs.distance; });

  return result;
}

void World::HitLineAttack(mobj::MapObject* parent, int damage, int distance, rend::BamAngle da) {
  auto crossed_objects = CreateIntersectedObjList(parent->x, parent->y, parent->angle + da, distance);

  Opening op;
  op.view_line_z = parent->z + 42; // Weapon height, TODO!!!

  // If bullet doesn't hit any monster, but hit a line on its original height,
  // the bullet hole must be drawn on that first wall (NOT on the last wall)
  bool save_first_wall = false;
  int fw_x = 0, fw_y = 0;

  for (auto elem : crossed_objects) {
    int idx = elem.obj.index();

    if (idx == 0) {
      auto l = std::get<0>(elem.obj);
      std::cout << "Hit line (" << l->x1 << ", " << l->y1 << ") -> (" << l->x2 << ", " << l->y2 << ")" << std::endl;

      // Solid wall
      if (!(l->flags & kLDFTwoSided)) {
        if (save_first_wall) {
          break;
        } else {
          auto [x, y] = math::ShiftToCenter(parent->x, parent->y, elem.x, elem.y, 2);
          SpawnBulletPuff(x, y, op.view_line_z);
          return;
        }
      }

      bool is_open = math::CorrectOpening(op, l, elem.distance);

      if (!save_first_wall && (op.view_line_z < op.low_z || op.view_line_z > op.high_z)) {
        save_first_wall = true;
        fw_x = elem.x;
        fw_y = elem.y;
      }

      if (!is_open) {
        if (save_first_wall) {
          break;
        } else {
          auto [x, y] = math::ShiftToCenter(parent->x, parent->y, elem.x, elem.y, 2);
          SpawnBulletPuff(x, y, op.view_line_z);
          return;
        }
      }
    } else if (idx == 1) {
      // MapObject
      auto mobj = std::get<1>(elem.obj);
      std::cout << "Hit mobj (" << mobj->x << ", " << mobj->y << ")" << std::endl;

      int high_z = op.view_line_z + elem.distance * op.coef_high_opening;
      int low_z = op.view_line_z - elem.distance * op.coef_low_opening;

      if (!((mobj->z > high_z) || (mobj->z + mobj->height < low_z))) {
        mobj->CauseDamage(damage);

        auto [x, y] = math::ShiftToCenter(parent->x, parent->y, elem.x, elem.y, 2);
        SpawnBulletBlood(x, y, mobj->z + mobj->height / 2);

        return;
      }
    }
  }

  if (save_first_wall) {
      auto [x, y] = math::ShiftToCenter(parent->x, parent->y, fw_x, fw_y, 2);
      SpawnBulletPuff(x, y, op.view_line_z);
  }
}

void World::HitAngleLineAttack(mobj::MapObject* parent, int damage, int distance,
                               rend::BamAngle dir_angle, rend::BamAngle vert_angle) {
  auto crossed_objects = CreateIntersectedObjList(parent->x, parent->y, dir_angle, distance);

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
        return;
      }
    } else if (idx == 1) {
      // MapObject
      auto mobj = std::get<1>(elem.obj);
      std::cout << "Hit mobj (" << mobj->x << ", " << mobj->y << ")" << std::endl;

      int hit_line_height = view_line_z + elem.distance * height_coef;

      if (!((mobj->z > hit_line_height) || (mobj->z + mobj->height < hit_line_height))) {
        mobj->CauseDamage(damage);

        auto [x, y] = math::ShiftToCenter(parent->x, parent->y, elem.x, elem.y, 2);
        SpawnBulletBlood(x, y, hit_line_height);

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
      //std::cout << "Hit mobj (" << mobj->x << ", " << mobj->y << ")" << std::endl;

      int high_z = op.view_line_z + elem.distance * op.coef_high_opening;
      int low_z = op.view_line_z - elem.distance * op.coef_low_opening;

      if (!((mobj->z > high_z) || (mobj->z + mobj->height < low_z))) {
        // Found the target
        int visible_top = std::min(mobj->z + mobj->height, high_z);
        int visible_bottom = std::max(mobj->z, low_z);

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
  std::unique_ptr<mobj::MapObject> bullet(new mobj::MapObject(id::mobjinfo[id::MT_PUFF]));

  bullet->x = x;
  bullet->y = y;
  bullet->z = z;
  bullet->flags |= mobj::MF_NOGRAVITY;

  PutMobjOnMap(std::move(bullet), false);
}

void World::SpawnBulletBlood(int x, int y, int z) {
  std::unique_ptr<mobj::MapObject> bullet(new mobj::MapObject(id::mobjinfo[id::MT_BLOOD]));

  bullet->x = x;
  bullet->y = y;
  bullet->z = z;
  bullet->flags |= mobj::MF_NOGRAVITY;

  PutMobjOnMap(std::move(bullet), false);
}

void World::SpawnBFGExplode(int x, int y, int z) {
  std::unique_ptr<mobj::MapObject> bfg(new mobj::MapObject(id::mobjinfo[id::MT_EXTRABFG]));

  bfg->x = x;
  bfg->y = y;
  bfg->z = z;
//  bfg->flags |= mobj::MF_NOGRAVITY;

  PutMobjOnMap(std::move(bfg), false);
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
}

bool World::TryToChangeSectorHeight(Sector* sec, int floor_h, int ceiling_h, bool cause_damage) {
  int max_height = ceiling_h - floor_h;

  bool is_obstacle = false;
  // Look for unfitted mobj and hit them
  for (const SubSector* ss : sec->subsecs) {
    for (auto mobj : ss->mobjs) {
      if (mobj->height > max_height) {
        is_obstacle = true;
        if (cause_damage) {
          // There can be many actions...
          // TODO: Put all them into CauseDamage() ???

          /*if (mobj->height <= 0) {
            // Get giblets
            mobj->flags &= ~mobj::MF_SOLID;
            mobj->height = 0;
            mobj->radius = 0;
            continue;
          }

          if (mobj->flags & mobj::MF_DROPPED) {
            // Delete such a stuff
            //mobj->
            continue;
          }

          if (!(mobj->flags & mobj::MF_SHOOTABLE)) {
            //Can't be hit
            continue;
          }*/

          if (!(tick_counter_ & 3)) {
            mobj->CauseDamage(10);
          }
        }
      }
    }
  }

  if (is_obstacle) {
    return false;
  }

  // Change mobjs' z
  for (const SubSector* ss : sec->subsecs) {
    for (auto mobj : ss->mobjs) {
      // floor_z and ceiling_z make opening for this mobj
      // These vars are used during moving
      mobj->floor_z = floor_h;
      mobj->ceiling_z = ceiling_h;

      if (mobj->z == sec->floor_height) {
        // The mobj is on floor
        mobj->z = floor_h;
      } else {
        // ? flying mobj ?
        if (mobj->z + mobj->height > ceiling_h) {
          mobj->z = ceiling_h - mobj->height;
        }
      }
    }
  }

  return true;
}

} // namespace world
