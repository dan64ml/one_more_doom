#include "graphics_manager.h"

#include <cstring>
#include <cassert>
#include <memory>
#include <algorithm>
#include <iostream>

#include "texture.h"
#include "world/wad_raw_types.h"
#include "world/wad_utils.h"

namespace graph {

void GraphicsManager::Load(const std::string& wad_file) {
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

  const wad::WadDirectoryEntry* directory = reinterpret_cast<wad::WadDirectoryEntry*>(buf.get());
  for (int i = 0; i < header.num_directories; ++i) {
//    const std::string name = wad::to_string<8>(directory[i].name);
//    std::cout << name << "     " << directory[i].size << std::endl;
    if (wad::strcmp<8>(directory[i].name, "P_START")) {
      // Zero-sized lump marking the beginning of wall patches
      while (!wad::strcmp<8>(directory[++i].name, "P_END")) {
        LoadPStartEntry(fin, directory[i]);
      }
    } else if (wad::strcmp<8>(directory[i].name, "PNAMES")){
      LoadPNames(fin, directory[i]);
    } else if (wad::strcmp<8>(directory[i].name, "TEXTURE1")) {
      LoadTexture(fin, directory[i]);
      std::string entry_name = wad::to_string<8>(directory[i].name);
    } else if (wad::strcmp<8>(directory[i].name, "PLAYPAL")) {
      // Lump with 14 palettes. Each palette contains 256 colors of RGB
      LoadPlayPal(fin, directory[i]);
    } else if (wad::strcmp<8>(directory[i].name, "COLORMAP")) {
      LoadColorMap(fin, directory[i]);
    } else if (wad::strcmp<8>(directory[i].name, "F_START")) {
      // Zero-sized lump marking the beginning of flats
      while (!wad::strcmp<8>(directory[++i].name, "F_END")) {
        LoadFlatEntry(fin, directory[i]);
      }
    } else if (wad::strcmp<8>(directory[i].name, "S_START")) {
      // Zero-sized lump marking the beginning of flats
      while (!wad::strcmp<8>(directory[++i].name, "S_END")) {
        LoadSpriteEntry(fin, directory[i]);
      }
    } else if (wad::strcmp<8>(directory[i].name, "STBAR")) {
      LoadSpriteEntry(fin, directory[i]);
    } else if (wad::strcmp<2>(directory[i].name, "ST")) {
      LoadSTBarEntry(fin, directory[i]);
      //const std::string name = wad::to_string<8>(directory[i].name);
      //std::cout << name << std::endl;
    } else {
      const std::string name = wad::to_string<8>(directory[i].name);
//      std::cout << name << std::endl;
    }
  }

  for (auto [name, _] : texture_offset_) {
    textures2_.insert({name, LoadWallTexture(name)});
  }
}

void GraphicsManager::LoadSTBarEntry(std::ifstream& fin, const wad::WadDirectoryEntry& entry) {
  const std::string name = wad::to_string<8>(entry.name);

  auto buf = std::unique_ptr<char[]>(new char[entry.size]);
  fin.seekg(entry.offset);
  fin.read(buf.get(), entry.size);

  auto texture = CreatePixelPicture(buf.get());

  //if (texture) {
    stbar_[name] = std::move(texture);
  //}
}

PixelPicture GraphicsManager::CreatePixelPicture(const char* buf) {
  const PatchRaw* patch = reinterpret_cast<const PatchRaw*>(buf);
  
  PixelPicture ret;
  ret.height = patch->height;
  ret.width = patch->width;
  ret.pixels.assign(patch->width * patch->height, kTransparentColor);

  for (int x = 0; x < patch->width; ++x) {
    int column_offset = patch->columnofs[x];
    const uint8_t* posts_data = reinterpret_cast<const uint8_t*>(patch) + column_offset;

    while (*posts_data != 0xff) {
      int offset = *posts_data;
      int len = *(posts_data + 1);

      posts_data += 3;
      for (int i = 0; i < len; ++i) {
        int idx = x + (offset + i ) * patch->width;
        ret.pixels[idx] = *posts_data;
        ++posts_data;
      }
      ++posts_data;
    }
  }

  return ret;
}

void GraphicsManager::LoadSpriteEntry(std::ifstream& fin, const wad::WadDirectoryEntry& entry) {
  if (entry.size == 0) {
    return;
  }

  const std::string name = wad::to_string<8>(entry.name);
  //if (name == "SHT2I0") {
  if (name == "SHT2I0") {
    std::cout << std::endl;
  }

  auto buf = std::unique_ptr<char[]>(new char[entry.size]);
  fin.seekg(entry.offset);
  fin.read(buf.get(), entry.size);

  const PatchRaw* patch = reinterpret_cast<PatchRaw*>(buf.get());
  
  PixelPicture ret;
  ret.height = patch->height;
  ret.width = patch->width;
  ret.left_offset = patch->left_offset;
  ret.top_offset = patch->top_offset;
  ret.pixels.assign(patch->width * patch->height, kTransparentColor);

  for (int x = 0; x < patch->width; ++x) {
    int column_offset = patch->columnofs[x];
    const uint8_t* posts_data = reinterpret_cast<const uint8_t*>(patch) + column_offset;

    while (*posts_data != 0xff) {
      int offset = *posts_data;
      int len = *(posts_data + 1);

      posts_data += 3;
      for (int i = 0; i < len; ++i) {
        int idx = x + (offset + i ) * patch->width;
        ret.pixels[idx] = *posts_data;
        ++posts_data;
      }
      ++posts_data;
    }
  }

  if (name.size() == 8) {
    //std::cout << name << std::endl;
    sprites_.insert({name.substr(0, 6), ret});
    MirrorSprite(ret);
    sprites_.insert({name.substr(0, 4) + name.substr(6, 2), ret});
  } else {
    sprites_.insert({name, ret});
  }
}

void GraphicsManager::MirrorSprite(PixelPicture& sp) {
  for (int i = 0; i < sp.width / 2; ++i) {
    for (int j = 0; j < sp.height; ++j) {
      int left_idx = i + j * sp.width;
      int right_idx = (sp.width - i - 1) + j * sp.width;

      std::swap(sp.pixels[left_idx], sp.pixels[right_idx]);
    }
  }
}

void GraphicsManager::LoadFlatEntry(std::ifstream& fin, const wad::WadDirectoryEntry& entry) {
  if (entry.size == 0) {
    return;
  }

  const std::string name = wad::to_string<8>(entry.name);

  FlatPicture pic;
  pic.height = 64;
  pic.width = 64;
  pic.pixels.resize(64 * 64);
  assert(pic.pixels.size() == entry.size);

  fin.seekg(entry.offset);
  fin.read(reinterpret_cast<char*>(pic.pixels.data()), entry.size);

  flats_.insert({name, pic});
}

void GraphicsManager::LoadColorMap(std::ifstream& fin, const wad::WadDirectoryEntry& entry) {
  fin.seekg(entry.offset);
  fin.read(reinterpret_cast<char*>(light_maps_), entry.size);
}

void GraphicsManager::LoadPlayPal(std::ifstream& fin, const wad::WadDirectoryEntry& entry) {
  std::unique_ptr<uint8_t[]> buf(new uint8_t[entry.size]);

  fin.seekg(entry.offset);
  fin.read(reinterpret_cast<char*>(buf.get()), entry.size);

  int idx = 0;
  for (int p = 0; p < kPalettesNumber; ++p) {
    for (int c = 0; c < kPaletteSize; ++c) {
      palette_[p][c] = buf[idx++];
      palette_[p][c] |= buf[idx++] << 8;
      palette_[p][c] |= buf[idx++] << 16;
      palette_[p][c] |= 0xFF000000;   // fully opaque by default
    }
  }
}

void GraphicsManager::LoadTexture(std::ifstream& fin, const wad::WadDirectoryEntry& entry) {
  fin.seekg(entry.offset);
  
  textures_.reset(new char[entry.size]);
  fin.read(textures_.get(), entry.size);

  int32_t num_textures = *reinterpret_cast<int32_t*>(textures_.get());
  int32_t* offsets = reinterpret_cast<int32_t*>(textures_.get() + 4);

  for (int i = 0; i < num_textures; ++i) {
    MapTextureRaw* texture = reinterpret_cast<MapTextureRaw*>(textures_.get() + offsets[i]);
    texture_offset_[wad::to_string<8>(texture->name)] = offsets[i];
  }
}

void GraphicsManager::LoadPStartEntry(std::ifstream& fin, const wad::WadDirectoryEntry& entry) {
  if (entry.size == 0) {
    return;
  }

  const std::string name = wad::to_string<8>(entry.name);
  auto [_, result] = patches_by_name_.try_emplace(name, std::unique_ptr<char[]>(new char[entry.size]));
  if (!result) {
    // Copies can be met
    //std::cout << "Can't insert " + name + " entry" << std::endl;
    return;
  }
  
  fin.seekg(entry.offset);
  fin.read(patches_by_name_[name].get(), entry.size);
}

// PNAMES lump starts with a header of 4 bytes, followed by all of the patch names.
void GraphicsManager::LoadPNames(std::ifstream& fin, const wad::WadDirectoryEntry& entry) {
  fin.seekg(entry.offset);

  int number;
  fin.read(reinterpret_cast<char*>(&number), 4);

  std::unique_ptr<char[]> buf(new char[8 * number]);
  fin.read(buf.get(), 8 * number);

  pnames_.reserve(number);
  for (int i = 0; i < number; ++i) {
    //std::cout << to_string8(buf.get() + 8 * i) << std::endl;
    pnames_.push_back(wad::to_string<8>(buf.get() + 8 * i));
  }
}

Texture GraphicsManager::GetTexture(std::string texture_name) const {
  if (texture_name == "-") {
    return {};
  }

  if (textures2_.count(texture_name) == 0) {
    //std::cout << "Texture " + texture_name + " not found!" << std::endl;
    return {};
  } else {
    return {this, &textures2_.at(texture_name)};
  }
}

Texture GraphicsManager::GetSprite(std::string sprite_name) const {
  if (sprites_.count(sprite_name) == 0) {
    //std::cout << "Sprite " + sprite_name + " not found!" << std::endl;
    return {};
  } else {
    return {this, &sprites_.at(sprite_name)};
  }
}

Texture GraphicsManager::GetSTBarElement(std::string element_name) const {
  if (stbar_.count(element_name) == 0) {
    //std::cout << "Sprite " + sprite_name + " not found!" << std::endl;
    return {};
  } else {
    return {this, &stbar_.at(element_name)};
  }
}

Texture GraphicsManager::GetSpriteEx(std::string sprite_name) const {
   if (sprites_.count(sprite_name) == 0) {
     sprite_name.back() = '0';
     return GetSprite(sprite_name);
   } else {
    return {this, &sprites_.at(sprite_name)};
  }
}

Flat GraphicsManager::GetFlat(std::string flat_name) const {
  if (flats_.count(flat_name) == 0) {
    //std::cout << "Flat " + flat_name + " not found!" << std::endl;
    return {};
  } else {
    return {this, &flats_.at(flat_name)};
  }
}

const PatchRaw* GraphicsManager::GetRawPatchByIdx(int idx) const {
  assert(idx < pnames_.size());
  const std::string name = pnames_[idx];
  assert(patches_by_name_.count(name) != 0);

  return reinterpret_cast<const PatchRaw*>(patches_by_name_.at(name).get());
}

PixelPicture GraphicsManager::LoadWallTexture(std::string name) {
  int offset = texture_offset_[name]; 
  MapTextureRaw* texture = reinterpret_cast<MapTextureRaw*>(textures_.get() + offset);

  PixelPicture ret;
  ret.height = texture->height;
  ret.width = texture->width;
  ret.pixels.assign(texture->width * texture->height, kTransparentColor);

  const PatchRaw* patch = nullptr;
  for (int p = 0; p < texture->patch_count; ++p) {
    const MapPatchRaw& mp = texture->patches[p];
    patch = GetRawPatchByIdx(mp.patch);

    int patch_x;
    int texture_x;
    if (mp.origin_x >= 0) {
      patch_x = 0;
      texture_x = mp.origin_x;
    } else {
      patch_x = -mp.origin_x;
      texture_x = 0;
    }

    for (; patch_x < patch->width && texture_x < texture->width; ++patch_x, ++texture_x) {
      int column_offset = patch->columnofs[patch_x];
      const uint8_t* posts_data = reinterpret_cast<const uint8_t*>(patch) + column_offset;

      bool end_column = false;
      while (*posts_data != 0xff && !end_column) {
        int offset = *posts_data;
        int len = *(posts_data + 1);

        posts_data += 3;
        for (int i = 0; i < len; ++i) {
          if (offset + i + mp.origin_y >= texture->height) {
            end_column = true;
            break;
          }
          if (offset + i + mp.origin_y < 0) {
            ++posts_data;
            continue;
          }

          int idx = texture_x + (offset + i + mp.origin_y) * texture->width;
          ret.pixels[idx] = *posts_data;
          ++posts_data;
        }
        ++posts_data;
      }
    }
  }

  return ret;
}

} // namespace graph