#ifndef TEXTURE_H_
#define TEXTURE_H_

#include "graphics_types.h"
#include "graphics_manager.h"

namespace graph {

template<class TImage>
class PaletteImage {
 public:
  PaletteImage() {}
  PaletteImage(const GraphicsManager* gm, const TImage* pp)
    : gm_(gm), picture_(pp) {}

  //bool GetPixel(int u, int v, int& r, int& g, int& b) const;
  Color GetPixel(int u, int v) const;

  int GetXSize() const { return picture_->width; }
  int GetYSize() const { return picture_->height; }
  int GetLeftOffset() const { return picture_->left_offset; }
  int GetTopOffset() const { return picture_->top_offset; }

  void SetPalette(int idx) { palette_idx_ = idx; }
  void SetLightLevel(int level) { light_level_ = level; }

  operator bool() const { return picture_; }
 private:
  const GraphicsManager* gm_ = nullptr;
  const TImage* picture_ = nullptr;

  int palette_idx_ = 0;
  int light_level_ = 0;
};

template<class TImage>
Color PaletteImage<TImage>::GetPixel(int u, int v) const {
  int pixel_idx = picture_->pixels[u + v * picture_->width];
  
  if constexpr (std::is_same_v<TImage, PixelPicture>) {
    if (pixel_idx == kTransparentColor) {
      return 0x00;
    }
  }

  pixel_idx = gm_->light_maps_[light_level_][pixel_idx];

  return gm_->palette_[palette_idx_][pixel_idx];
}

/*template<class TImage>
bool PaletteImage<TImage>::GetPixel(int u, int v, int& r, int& g, int& b) const {
  int pixel_idx = picture_->pixels[u + v * picture_->width];
  
  if constexpr (std::is_same_v<TImage, PixelPicture>) {
    if (pixel_idx == kTransparentColor) {
      return false;
    }
  }

  pixel_idx = gm_->light_maps_[light_level_][pixel_idx];
  r = gm_->palette_[palette_idx_][pixel_idx][0];
  g = gm_->palette_[palette_idx_][pixel_idx][1];
  b = gm_->palette_[palette_idx_][pixel_idx][2];

  return true;
}*/

} // namespace graph

#endif  // TEXTURE_H_
