#include "texture.h"
#include "graphics_manager.h"

#include <cassert>

namespace wad {

/*template<class TImage>
bool PaletteImage<TImage>::GetPixel(int u, int v, int& r, int& g, int& b) const {
  return false;
}

template<>
bool PaletteImage<PixelPicture>::GetPixel(int u, int v, int& r, int& g, int& b) const {
  int pixel_idx = picture_->pixels[u + v * picture_->width];
  if (pixel_idx == kTransparentColor) {
    return false;
  }

  pixel_idx = gm_->light_maps_[light_level_][pixel_idx];
  r = gm_->palette_[palette_idx_][pixel_idx][0];
  g = gm_->palette_[palette_idx_][pixel_idx][1];
  b = gm_->palette_[palette_idx_][pixel_idx][2];

  return true;
}

template<>
bool PaletteImage<FlatPicture>::GetPixel(int u, int v, int& r, int& g, int& b) const {
  int pixel_idx = picture_->pixels[u + v * picture_->width];
  pixel_idx = gm_->light_maps_[light_level_][pixel_idx];

  r = gm_->palette_[palette_idx_][pixel_idx][0];
  g = gm_->palette_[palette_idx_][pixel_idx][1];
  b = gm_->palette_[palette_idx_][pixel_idx][2];

  return true;
}

template class PaletteImage<PixelPicture>;
template class PaletteImage<FlatPicture>;*/

} // namespace wad