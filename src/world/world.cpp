#include "world.h"

#include <fstream>
#include <exception>
#include <memory>

#include "wad_utils.h"
#include "wad_raw_types.h"

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
  player_.TickTime();
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
    // TODO: create MapObject
    if (items[i].type == 1) {
      player_.x = items[i].x;
      player_.y = items[i].y;

      int ss_idx = bsp_.GetSubSectorIdx(player_.x, player_.y);
      player_.floor_z = player_.z = sub_sectors_[ss_idx].sector->floor_height;
      player_.ss = &sub_sectors_[ss_idx];
      player_.radius = 16;

      player_.flags = mobj::MF_SOLID | mobj::MF_DROPOFF;

      player_.angle = rend::DegreesToBam(items[i].angle);
    } else if (items[i].type == 45 || items[i].type == 46) {
      mobj::MapObject torch(this);
      torch.x = items[i].x;
      torch.y = items[i].y;

      //torch.height = 

      int ss_idx = bsp_.GetSubSectorIdx(torch.x, torch.y);
      torch.z = sub_sectors_[ss_idx].sector->floor_height;

      torch.texture = "TGRNA0";

      mobjs_.push_back(torch);
      sub_sectors_[ss_idx].mobjs.push_back(&mobjs_.back());
    }
  }
}

} // namespace world
