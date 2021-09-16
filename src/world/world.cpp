#include "world.h"

#include <fstream>
#include <exception>
#include <memory>
#include <algorithm>
#include <iostream>

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

  const auto bsp_lump = levels_[level].second.at("NODES");
  bsp_.LoadBsp(fin, bsp_lump.position, bsp_lump.size);
  bsp_.SetSubSectors(&sub_sectors_);

  const auto block_map_lump = levels_[level].second.at("BLOCKMAP");
  blocks_.Load(fin, block_map_lump.position, block_map_lump.size, lines_);

  CreateMapObjectList(fin);
}

void World::TickTime() {
  player_->TickTime();

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
  auto proj = std::unique_ptr<mobj::MapObject>(new mobj::Projectile(type, parent));
  PutMobjOnMap(std::move(proj), false);
}

void World::DoBlastDamage(int damage, int x, int y) {
  BBox bb {x - damage, x + damage, y + damage, y - damage};
  for (auto obj :blocks_.GetMapObjects(bb)) {
    if (!(obj->flags & mobj::MF_SHOOTABLE)) {
      continue;
    }
    // This monsters can't be hit by blast
    if (obj->type == id::MT_CYBORG || obj->type == id::MT_SPIDER) {
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

    if (IsMobjVisible(x, y, obj)) {
      obj->CauseDamage(damage - dist);
    }
  }
}

bool World::IsMobjVisible(int vp_x, int vp_y, const mobj::MapObject* obj) const {
  return true;
}

std::vector<IntersectedObject> World::CreateIntersectedObjList(int from_x, int from_y, 
                                                               rend::BamAngle angle, int distance) {
  std::vector<IntersectedObject> result;

  int to_x = from_x + distance * rend::BamCos(angle);
  int to_y = from_y + distance * rend::BamSin(angle);

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

  // FOV
  double coef_high_opening = 100.0 / 160;
  double coef_low_opening = 100.0 / 160;
  // If bullet doesn't hit any monster, but hit a line on its original height,
  // the bullet hole must be drawn on that first wall
  int first_line_distance = 0;

  int vp_z = player_->z + 42; // Weapon height, TODO!!!

  for (auto obj : crossed_objects) {
    int idx = obj.obj.index();

    if (idx == 0) {
      // Line
      auto l = std::get<0>(obj.obj);
      std::cout << "Hit line (" << l->x1 << ", " << l->y1 << ") -> (" << l->x2 << ", " << l->y2 << ")" << std::endl;

      if (!(l->flags & kLDFTwoSided)) {
        // wall

        if (first_line_distance) {
          break;
        }

        std::unique_ptr<mobj::MapObject> bullet( new mobj::MapObject(id::mobjinfo[id::MT_PUFF]));
        bullet->x = parent->x + (obj.distance - 3) * rend::BamCos(parent->angle + da);
        bullet->y = parent->y + (obj.distance - 3) * rend::BamSin(parent->angle + da);
        bullet->z = parent->z + 42;
        bullet->flags |= mobj::MF_NOGRAVITY;

        PutMobjOnMap(std::move(bullet), false);

        return;
      }

      int high_z = vp_z + obj.distance * coef_high_opening;
      int low_z = vp_z - obj.distance * coef_low_opening;

      int line_low = std::max(l->sides[0]->sector->floor_height, l->sides[1]->sector->floor_height);
      int line_high = std::min(l->sides[0]->sector->ceiling_height, l->sides[1]->sector->ceiling_height);

      if (!first_line_distance && (vp_z < line_low || vp_z > line_high)) {
        first_line_distance = obj.distance;
      }

      if (line_low > high_z || line_high < low_z) {
        // Portal closes view

        if (first_line_distance) {
          break;
        }

        std::unique_ptr<mobj::MapObject> bullet( new mobj::MapObject(id::mobjinfo[id::MT_PUFF]));
        bullet->x = parent->x + (obj.distance - 3) * rend::BamCos(parent->angle + da);
        bullet->y = parent->y + (obj.distance - 3) * rend::BamSin(parent->angle + da);
        bullet->z = parent->z + 42;
        bullet->flags |= mobj::MF_NOGRAVITY;

        PutMobjOnMap(std::move(bullet), false);

        return;
      } else {
        // Correct opening if it's necessary
        if (line_low > low_z) {
          coef_low_opening = static_cast<double>(vp_z - line_low) / obj.distance;
        }
        if (line_high < high_z) {
          coef_high_opening = static_cast<double>(line_high - vp_z) / obj.distance;
        }
      }
    } else if (idx == 1) {
      // MapObject
      auto m = std::get<1>(obj.obj);
      std::cout << "Hit mobj (" << m->x << ", " << m->y << ")" << std::endl;

      int high_z = vp_z + obj.distance * coef_high_opening;
      int low_z = vp_z - obj.distance * coef_low_opening;

      if (!((m->z > high_z) || (m->z + m->height < low_z))) {
        m->CauseDamage(damage);

        std::unique_ptr<mobj::MapObject> bullet( new mobj::MapObject(id::mobjinfo[id::MT_BLOOD]));
        bullet->x = parent->x + (obj.distance - 3) * rend::BamCos(parent->angle + da);
        bullet->y = parent->y + (obj.distance - 3) * rend::BamSin(parent->angle + da);
        bullet->z = m->z + m->height / 2;
        bullet->flags |= mobj::MF_NOGRAVITY;

        PutMobjOnMap(std::move(bullet), false);

        return;
      }
    }
  }

  if (first_line_distance) {
      std::unique_ptr<mobj::MapObject> bullet( new mobj::MapObject(id::mobjinfo[id::MT_PUFF]));
      bullet->x = parent->x + (first_line_distance - 3) * rend::BamCos(parent->angle + da);
      bullet->y = parent->y + (first_line_distance - 3) * rend::BamSin(parent->angle + da);
      bullet->z = parent->z + 42;
      bullet->flags |= mobj::MF_NOGRAVITY;

      PutMobjOnMap(std::move(bullet), false);
  }
}

void World::SpawnBulletPuff(int x, int y, int z) {
  std::unique_ptr<mobj::MapObject> bullet( new mobj::MapObject(id::mobjinfo[id::MT_PUFF]));

  bullet->x = x;
  bullet->y = y;
  bullet->z = z;
  bullet->flags |= mobj::MF_NOGRAVITY;

  PutMobjOnMap(std::move(bullet), false);
}

void World::SpawnBulletBlood(int x, int y, int z) {
  std::unique_ptr<mobj::MapObject> bullet( new mobj::MapObject(id::mobjinfo[id::MT_BLOOD]));

  bullet->x = x;
  bullet->y = y;
  bullet->z = z;
  bullet->flags |= mobj::MF_NOGRAVITY;

  PutMobjOnMap(std::move(bullet), false);
}

} // namespace world
