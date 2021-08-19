#ifndef GRAPHICS_MANAGER_H_
#define GRAPHICS_MANAGER_H_

#include "graphics_types.h"

#include <unordered_map>
#include <string>
#include <vector>
#include <fstream>
#include <array>
#include <memory>

#include "world/wad_raw_types.h"

namespace graph {

template<class T> class PaletteImage;
using Flat = PaletteImage<FlatPicture>;
using Texture = PaletteImage<PixelPicture>;

const int kPaletteSize = 256;
const int kPalettesNumber = 14;
const int kLightMapsNumber = 34;

using Color = uint32_t;
using Palette = std::array<Color, kPaletteSize>;
using LightMap = std::array<uint8_t, kPaletteSize>;


class GraphicsManager {
 public:
  GraphicsManager() = default;

  void Load(const std::string& wad_file);

  const Palette& GetPalette(int idx) const { return palette_[idx]; }
  const LightMap* GetLightMap() const { return light_maps_; }

  Texture GetTexture(std::string texture_name) const;
  Texture GetSprite(std::string sprite_name) const;
  // Trys to load sprite with name sprite_name. If such texture not found,
  // trys to load texture with another last number...
  Texture GetSpriteEx(std::string sprite_name) const;
  Flat GetFlat(std::string flat_name) const;

 private:
  // Set of all MapTextureRaw. MapTextureRaw has variable size, so we need
  // offsets to the starts of the structures.
  std::unique_ptr<char[]> textures_;
  // Our offsets
  std::unordered_map<std::string, int> texture_offset_;

  // List of names of Patch's. Index in this vector is the index of patch in
  // MapPatchRaw::patch. It gives the name of PatchRaw.
  std::vector<std::string> pnames_;
  // Maps pnames_ indexes=>names to raw pointer to PatchRaw.
  std::unordered_map<std::string, std::unique_ptr<char[]>> patches_by_name_;

  std::unordered_map<std::string, PixelPicture> textures2_;
  std::unordered_map<std::string, FlatPicture> flats_;
  std::unordered_map<std::string, PixelPicture> sprites_;

  // Set VGA palettes (256 x RGB). The game uses 14 palettes.
  // #0 is default. #1 - #8 are used to indicate damage.
  // #9 - #12 are used to show item picking up. Both intervals are used
  // for fading effects. #13 is used for radiation suit effect.
  // Stored in PLAYPAL lump in raw format.
  Palette palette_[kPalettesNumber];

  // A great idea. Shades of every color in palette are emulated with
  // another colors from the same palette. LightMap #0 correspondes to
  // max bright, #31 - the darkest.
  // Colormap #32 is used when the player has the invulnerability powerup.
  //
  // We have the next color pipeline:
  // value in texture -> light_maps_[bright level][the value] -> 
  // -> palette_[current palette][the value] -> RGB
  // 
  // So, we can change the illumination of scene and emulate game events
  // independently without changing textures.
  // Stored in COLORMAP lump
  LightMap light_maps_[kLightMapsNumber];

  void LoadPNames(std::ifstream& fin, const wad::WadDirectoryEntry& entry);
  void LoadPStartEntry(std::ifstream& fin, const wad::WadDirectoryEntry& entry);
  void LoadTexture(std::ifstream& fin, const wad::WadDirectoryEntry& entry);
  void LoadPlayPal(std::ifstream& fin, const wad::WadDirectoryEntry& entry);
  void LoadColorMap(std::ifstream& fin, const wad::WadDirectoryEntry& entry);

  void LoadFlatEntry(std::ifstream& fin, const wad::WadDirectoryEntry& entry);
  void LoadSpriteEntry(std::ifstream& fin, const wad::WadDirectoryEntry& entry);

  void MirrorSprite(PixelPicture& sp);

  const PatchRaw* GetRawPatchByIdx(int idx) const;

  PixelPicture LoadWallTexture(std::string name);

  template<class T> friend class PaletteImage;
};

} // namespace graph

#endif  // GRAPHICS_MANAGER_H_
