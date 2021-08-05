#include "block_map.h"

#include <memory>

#include "wad_raw_types.h"

namespace world {

void BlockMap::Load(std::ifstream& fin, int offset, int size, const std::vector<Line>& lines) {
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

} // namespace world