#include "block_map.h"

#include <memory>
#include <algorithm>

#include "wad_raw_types.h"

namespace world {

void BlockMap::Load(std::ifstream& fin, int offset, int size, std::vector<Line>& lines) {
  std::unique_ptr<int16_t[]> buf(new int16_t[size / 2]);
  fin.seekg(offset);
  fin.read(reinterpret_cast<char*>(buf.get()), size);

  wad::WadBlockmapHeader* header = reinterpret_cast<wad::WadBlockmapHeader*>(buf.get());
  x_offset_ = header->x;
  y_offset_ = header->y;

  // Offsets area
  uint16_t* p_idx = reinterpret_cast<uint16_t*>(&buf[4]);
  blocks_.clear();
  blocks_.resize(header->rows);
  for (int r = 0; r < header->rows; ++r) {
    blocks_[r].resize(header->columns);

    for (int c = 0; c < header->columns; ++c) {
      int16_t* ldf = &buf[p_idx[c + r * header->columns]];
      for (int n = 1; ldf[n] != -1; ++n) {
        blocks_[r][c].lines.push_back(&lines[ldf[n]]);
      }
    }
  }
}

MobjRange BlockMap::GetMapObjects(const BBox& bbox) const {
  MobjRange result;
  int left_idx = (bbox.left - x_offset_) >> kBlockShift;
  int right_idx = (bbox.right - x_offset_) >> kBlockShift;
  int top_idx = (bbox.top - y_offset_) >> kBlockShift;
  int bottom_idx = (bbox.bottom - y_offset_) >> kBlockShift;
  
  if (left_idx < 0) left_idx = 0;
  if (right_idx >= (int)blocks_.front().size()) right_idx = blocks_.front().size() - 1;
  if (top_idx >= (int)blocks_.size()) top_idx = blocks_.size() - 1;
  if (bottom_idx < 0) bottom_idx = 0;

  for (int i = bottom_idx; i <= top_idx; ++i) {
    for (int j = left_idx; j <= right_idx; ++j) {
      result.insert(begin(blocks_[i][j].mobjs), end((blocks_[i][j].mobjs)));
    }
  }

  return result;
}

LineRange BlockMap::GetLines(const BBox& bbox) const {
  LineRange result;
  int left_idx = (bbox.left - x_offset_) >> kBlockShift;
  int right_idx = (bbox.right - x_offset_) >> kBlockShift;
  int top_idx = (bbox.top - y_offset_) >> kBlockShift;
  int bottom_idx = (bbox.bottom - y_offset_) >> kBlockShift;
  
  if (left_idx < 0) left_idx = 0;
  if (right_idx >= (int)blocks_.front().size()) right_idx = blocks_.front().size() - 1;
  if (top_idx >= (int)blocks_.size()) top_idx = blocks_.size() - 1;
  if (bottom_idx < 0) bottom_idx = 0;

  for (int i = bottom_idx; i <= top_idx; ++i) {
    for (int j = left_idx; j <= right_idx; ++j) {
      result.insert(begin(blocks_[i][j].lines), end((blocks_[i][j].lines)));
    }
  }

  return result;
}

void BlockMap::AddMapObject(mobj::MapObject* mobj) {
  int x_idx = (mobj->x - x_offset_) >> kBlockShift;
  int y_idx = (mobj->y - y_offset_) >> kBlockShift;
  blocks_[y_idx][x_idx].mobjs.push_back(mobj);
}

bool BlockMap::DeleteMapObject(mobj::MapObject* mobj) {
  int x_idx = (mobj->x - x_offset_) >> kBlockShift;
  int y_idx = (mobj->y - y_offset_) >> kBlockShift;

  auto& ml = blocks_[y_idx][x_idx].mobjs;
  auto it = std::find(begin(ml), end(ml), mobj);
  if (it == end(ml)) {
    return false;
  }

  ml.erase(it);

  return true;
}

bool BlockMap::MoveMapObject(mobj::MapObject* mobj, int new_x, int new_y) {
  if (!DeleteMapObject(mobj)) {
    return false;
  }

  int x_idx = (new_x - x_offset_) >> kBlockShift;
  int y_idx = (new_y - y_offset_) >> kBlockShift;
  blocks_[y_idx][x_idx].mobjs.push_back(mobj);

  return true;
}

} // namespace world