#include "renderer.h"

#include <limits>
#include <iostream>
#include <cassert>
#include <algorithm>

#include <chrono>

namespace rend {

Renderer::Renderer() {
  for (BamAngle a = kBamAngle0; a <= kBamAngle90; ++a) {
    bam_to_screen_x_table_[a] = kScreenXResolution / 2 + (kScreenXResolution / 2) * BamTan(static_cast<BamAngle>(kBamAngle45 - a));
  }
  for (BamAngle a = kBamAngle90; a < kBamAngle180; ++a) { 
    bam_to_screen_x_table_[a] = 0;
  }

  top_clip_.resize(kScreenXResolution);
  bottom_clip_.resize(kScreenXResolution);
}

void Renderer::RenderScene(const wad::FastBsp* bsp, const graph::GraphicsManager* gm, const mobj::Player* player) {
  auto start = std::chrono::steady_clock::now();
  
  gm_ = gm;
  bsp_ = bsp;
  vp_.x = player->x;
  vp_.y = player->y;
  player_feet_height_ = player->z;
  vp_.angle = player->angle;
  
  ClearRenderer();

  RenderWalls();
  RenderFlats();
  RenderMasked();
  RenderStatusBar(player);

  auto finish = std::chrono::steady_clock::now();
  auto dur = finish - start;

  int msec = std::chrono::duration_cast<std::chrono::milliseconds>(dur).count();
  wnd_->PrintString("msec = " + std::to_string(msec) + ", fps = " + std::to_string(1000 / msec));

  wnd_->ShowFBRender();
}

void Renderer::RenderStatusBar(const mobj::Player* player) {
  auto bar = gm_->GetSprite("STBAR");

//  auto arms_name = player->GetWeaponSprite();
//  auto ext = gm_->GetSprite(arms_name);
  
  //auto ext = gm_->GetSprite("PISFA0");
  //auto ext = gm_->GetSprite("SHTGA0");
  //auto ext = gm_->GetSprite("SHTFA0");
  
  //auto ext = gm_->GetSprite("PLSGA0");
  //auto ext = gm_->GetSprite("PLSFB0");
  //auto ext = gm_->GetSprite("PLSSB0");

  //auto ext = gm_->GetSTBarElement("STFTL00");
  
//  auto name = player->GetSpriteName(vp_.x, vp_.y);
//  auto ext = gm_->GetSprite(name);

//  int ext_y_size = ext.GetYSize() * rend::kScaleCoef;
//
//  int x_shift = (160 - ext.GetXSize() / 2) * rend::kScaleCoef;
//  //int y_shift = 32 * rend::kScaleCoef;
//  int y_shift = 16 * rend::kScaleCoef;
//
//  for (int i = 0; i < ext_y_size; ++i) {
//    for (int j = 0; j < ext.GetXSize() * rend::kScaleCoef; ++j) {
//      uint32_t color = ext.GetPixel(j / rend::kScaleCoef, i / rend::kScaleCoef);
//      wnd_->RenderFBPointAlpha(j + x_shift, ext_y_size - i - 1 + y_shift, color);
//    }
//  }

  RenderWeapon(player->GetWeapon());

  int bar_y_size = bar.GetYSize() * rend::kScaleCoef;
  for (int i = 0; i < bar_y_size; ++i) {
    for (int j = 0; j < bar.GetXSize() * rend::kScaleCoef; ++j) {
      uint32_t color = bar.GetPixel(j / rend::kScaleCoef, i / rend::kScaleCoef);
      wnd_->RenderFBPointAlpha(j, bar_y_size - i - 1, color);
    }
  }
}

void Renderer::RenderWeapon(const wpn::Weapon& w) {
  auto arms_name = w.GetSprite();
  //arms_name = "SHT2A0";
  //arms_name = "SHTGA0";
  //arms_name = "PISGB0";
  auto texture = gm_->GetSprite(arms_name);
  
  int top = 200 + texture.GetTopOffset() - 16;

  //std::cout << arms_name << " weapon: ";
  //int top_y = 50 - texture.GetTopOffset();
  
  //DrawSpriteTopCenter(texture, 160, top);
  DrawTextureWithShifts(texture, 0, 16);
  //DrawSpriteInCenter(texture, 160, top);


  /*auto ext = gm_->GetSprite(arms_name);

  int top = w.GetWeaponTopPosition() * rend::kScaleCoef;

  int ext_y_size = ext.GetYSize() * rend::kScaleCoef;

  int x_shift = (160 - ext.GetXSize() / 2) * rend::kScaleCoef;
  int y_shift = 32 * rend::kScaleCoef;
  //int y_shift = 16 * rend::kScaleCoef;

  for (int i = 0; i < top; ++i) {
    for (int j = 0; j < ext.GetXSize() * rend::kScaleCoef; ++j) {
      uint32_t color = ext.GetPixel(j / rend::kScaleCoef, (top - i - 1) / rend::kScaleCoef);
      wnd_->RenderFBPointAlpha(j + x_shift, i + y_shift, color);
    }
  }*/

  auto effect_name = w.GetEffectSprite();
  //effect_name = "MISFA0";
  //effect_name = "SHT2I0";
  //effect_name = "SHTFA0";
  //effect_name = "PISFA0";
  if (effect_name.empty()) {
    return;
  }
  //top = 200 - (32 - texture.GetTopOffset());
  texture = gm_->GetSprite(effect_name);
  top = 200 + texture.GetTopOffset() - 16;
  //std::cout << effect_name << " effect: ";
  //DrawSpriteTopCenter(texture, 160, top);
  DrawTextureWithShifts(texture, 0, 16);

  /*ext = gm_->GetSprite(effect_name);

  top = (10 + w.GetWeaponTopPosition()) * rend::kScaleCoef;

  ext_y_size = ext.GetYSize() * rend::kScaleCoef;

  x_shift = (160 - ext.GetXSize() / 2) * rend::kScaleCoef;
  y_shift = 32 * rend::kScaleCoef;
  //int y_shift = 16 * rend::kScaleCoef;

  for (int i = 0; i < top; ++i) {
    for (int j = 0; j < ext.GetXSize() * rend::kScaleCoef; ++j) {
      uint32_t color = ext.GetPixel(j / rend::kScaleCoef, (top - i - 1) / rend::kScaleCoef);
      wnd_->RenderFBPointAlpha(j + x_shift, i + y_shift, color);
    }
  }*/
}

// extra_y_shift - positive value means shift down, extra_x_shift - shift right
void Renderer::DrawTextureWithShifts(const graph::Texture& texture, int extra_x_shift, int extra_y_shift) {
  int x_size = texture.GetXSize();
  int y_size = texture.GetYSize();

  int left_offset = texture.GetLeftOffset();
  int top_offset = texture.GetTopOffset();

  int sx_from = (-left_offset + extra_x_shift) * kScaleCoef;
  int sx_to = sx_from + x_size * kScaleCoef;
  int sx_offset = sx_from; // -
  sx_from = std::max(0, sx_from);
  sx_to = std::min(kScreenXResolution, sx_to);

  int sy_from = (kOriginYResolution + top_offset - y_size - extra_y_shift) * kScaleCoef;
  int sy_to = (kOriginYResolution + top_offset - extra_y_shift) * kScaleCoef;
  int sy_offset = sy_from;  // -
  sy_from = std::max(0, sy_from);
  sy_to = std::min(kScreenYResolution, sy_to);

  for (int x = sx_from; x < sx_to; ++x) {
    for (int y = sy_from; y < sy_to; ++y) {
      uint32_t color = texture.GetPixel((x - sx_offset) / kScaleCoef, y_size - 1 - (y - sy_offset) / kScaleCoef);
      wnd_->RenderFBPointAlpha(x, y, color);
    }
  }
}


void Renderer::DrawSpriteInCenter(const graph::Texture& texture, int cx, int cy) {
  //auto texture = gm_->GetSprite(name);
  int x_size = texture.GetXSize();
  int y_size = texture.GetYSize();

  int sx_from = (cx - x_size / 2) * kScaleCoef;
  int sx_to = sx_from + x_size * kScaleCoef;
  int sx_offset = -sx_from;
  sx_from = std::max(0, sx_from);
  sx_to = std::min(kScreenXResolution, sx_to);

  int sy_from = (cy - y_size / 2) * kScaleCoef;
  int sy_to = sy_from + y_size * kScaleCoef;
  int sy_offset = -sy_from;
  sy_from = std::max(0, sy_from);
  sy_to = std::min(kScreenYResolution, sy_to);

  for (int x = sx_from; x < sx_to; ++x) {
    for (int y = sy_from; y < sy_to; ++y) {
      uint32_t color = texture.GetPixel((x + sx_offset) / kScaleCoef, y_size - 1 - (y + sy_offset) / kScaleCoef);
      wnd_->RenderFBPointAlpha(x, y, color);
    }
  }
}

// cx - center of the sprite, top_y - screen top
void Renderer::DrawSpriteTopCenter(const graph::Texture& texture, int cx, int top_y) {
  //auto texture = gm_->GetSprite(name);
  int x_size = texture.GetXSize();
  int y_size = texture.GetYSize();

  //std::cout << texture.GetLeftOffset() << ", " << texture.GetTopOffset() << std::endl;
  //top_y = top_y - texture.GetTopOffset();
  
  int sx_from = (cx - x_size / 2) * kScaleCoef;
  int sx_to = sx_from + x_size * kScaleCoef;
  int sx_offset = -sx_from;
  sx_from = std::max(0, sx_from);
  sx_to = std::min(kScreenXResolution, sx_to);

  int sy_from = (top_y - y_size) * kScaleCoef;
  int sy_to = top_y * kScaleCoef;
  int sy_offset = -sy_from;
  sy_from = std::max(0, sy_from);
  sy_to = std::min(kScreenYResolution, sy_to);

  for (int x = sx_from; x < sx_to; ++x) {
    for (int y = sy_from; y < sy_to; ++y) {
      uint32_t color = texture.GetPixel((x + sx_offset) / kScaleCoef, y_size - 1 - (y + sy_offset) / kScaleCoef);
      wnd_->RenderFBPointAlpha(x, y, color);
    }
  }
}

void Renderer::RenderFlats() {
  for (const auto& vs : visplanes_) {
    if (vs->texture == "F_SKY1") {
      DrawSky(*(vs.get()));
    } else {
      CreateRanges(*(vs.get()));
      DrawPixelRange(*(vs.get()));
    }
  }
}

void Renderer::RenderMasked() {
  SortMaskedObjects();

  for (const auto& msk : masked_) {
    DrawMaskedObject(msk);
  }
}

void Renderer::DrawMaskedObject(const MaskedObject& msk) {
  FillContextFromMasked(msk);

  for (int x = msk.left; x <= msk.right; ++x) {
    if (msk.top_clip[x - ctx_.sx_leftmost] == -1) {
      continue;
    }

    // Screen top and bottom ends of current column
    int top = ctx_.mid_dy_top * (x - ctx_.sx_leftmost) + ctx_.mid_y_top;
    int bottom = ctx_.mid_dy_bottom * (x - ctx_.sx_leftmost) + ctx_.mid_y_bottom;

    DrawColumn(x, top, bottom, !(ctx_.line_def_flags & world::kLDFLowerUnpegged));
  }
}

void Renderer::FillContextFromMasked(const MaskedObject& msk) {
  // Ends of segment
  ctx_.p1 = msk.p1;
  ctx_.p2 = msk.p2;

  // Data to calculate screen coordinates
  ctx_.sx_leftmost = msk.sx_leftmost;
  ctx_.sx_rightmost = msk.sx_rightmost;

  // Distances
  ctx_.left_distance = SegmentLength(vp_, ctx_.p1) * rend::BamCos(ctx_.p1.angle - rend::kBamAngle45);
  ctx_.right_distance = SegmentLength(vp_, ctx_.p2) * rend::BamCos(ctx_.p2.angle - rend::kBamAngle45);

  ctx_.line_def_flags = 0;
  ctx_.full_offset = msk.full_offset;
  ctx_.row_offset = 0;

  ctx_.texture = msk.texture;
  ctx_.pixel_height = msk.height;

  // TODO: ??? light_level and floor_height (for flying mobjs)

  // TODO: ??? Redundant - name and Texture...
  //ctx_.mid_texture = msk.texture_name;

  // For texturing
  ctx_.segment_len = SegmentLength(ctx_.p1, ctx_.p2);

  std::tie(ctx_.mid_y_bottom, ctx_.mid_y_top, ctx_.mid_dy_bottom, ctx_.mid_dy_top) 
        = CreateCoefs(msk.z, msk.z + ctx_.pixel_height);

  ctx_.pixel_texture_y_shift = 0;

  // DrawColumn() uses floor_level_ and ceiling_level_ for clipping
  for (int i = msk.sx_leftmost; i <= msk.sx_rightmost; ++i) {
    floor_level_[i] = msk.bottom_clip[i - msk.sx_leftmost];
    ceiling_level_[i] = msk.top_clip[i - msk.sx_leftmost];
  }
}

void Renderer::DrawSky(const Visplane& vs) {
  std::string texture_name;
  if (level_ < 11) {
    texture_name = "SKY1";
  } else if (level_ < 20) {
    texture_name = "SKY2";
  } else {
    texture_name = "SKY3";
  }

  auto texture = gm_->GetTexture(texture_name);
  texture.SetLightLevel(0);

  for (int i = vs.min_x; i <= vs.max_x; ++i) {
    if (vs.bottom[i] == -1) {
      continue;
    }
    BamAngle da = kBamAngle90 - vp_.angle;
    int shift = (kScreenXResolution / static_cast<double>(kBamAngle90)) * da;

    int u = ((i + shift) / kScaleCoef) % 256;

    for (int j = vs.bottom[i]; j <= vs.top[i]; ++j) {
      assert(j >= 0);
      assert(j < kScreenYResolution);
      int v = (kScreenYResolution - j - 1) / kScaleCoef;
      uint32_t color = texture.GetPixel(u, v);
      wnd_->RenderFBPointAlpha(i, j, color);
    }
  }
}

void Renderer::DrawPixelRange(const Visplane& vs) {
  auto texture = gm_->GetFlat(vs.texture);
  int horizon = player_feet_height_ + kPlayerHeight;

  for (int i = min_vsp_y_; i <= max_vsp_y_; ++i) {
    double z = (horizon - vs.height) * kProjectPlaneDist 
      / (kProjectPlaneCenter - i / static_cast<double>(kScaleCoef));
    double dz_x = z * BamCos(vp_.angle);
    double dz_y = z * BamSin(vp_.angle);

    double step = z / kProjectPlaneDist;
    double ds_x = step * BamSin(static_cast<BamAngle>(vp_.angle));
    double ds_y = -step * BamCos(static_cast<BamAngle>(vp_.angle));

    int light = 0;
    if (opt_.sector_light_enable) {
      light = (255 - vs.light_level);
    }
    if (opt_.distance_light_enable) {
      light += kDiminishFactor * z;
    }
    light >>= kLightShift;
    if (light < 0) {
      light = 0;
    }
    if (light >= kLightLevels) {
      light = kLightLevels - 1;
    }
    texture.SetLightLevel(light);

    for (const auto& r : vsp_ranges_[i]) {
      for (int c = r.first; c <= r.last; ++c) {
        int map_x = vp_.x + dz_x + (c - kScreenXResolution / 2) * ds_x / kScaleCoef;
        int map_y = vp_.y + dz_y + (c - kScreenXResolution / 2) * ds_y / kScaleCoef;

        int mx = (map_x >= 0) ? map_x % 64 : 63 + map_x % 64;
        int my = (map_y >= 0) ? map_y % 64 : 63 + map_y % 64;

        uint32_t color = texture.GetPixel(mx, my);
        wnd_->RenderFBPointAlpha(c, i, color);
      }
    }
  }
}

void Renderer::CreateRanges(const Visplane& vs) {
  // TODO: Optimize!!!
  vsp_ranges_.clear();
  min_vsp_y_ = std::numeric_limits<int>::max();
  max_vsp_y_ = std::numeric_limits<int>::min();

  for (int i = vs.min_x; i <= vs.max_x; ++i) {
    if (vs.bottom[i] == -1) {
      continue;
    }

    min_vsp_y_ = std::min(min_vsp_y_, vs.bottom[i]);
    max_vsp_y_ = std::max(max_vsp_y_, vs.top[i]);
  }

  vsp_ranges_.resize(kScreenYResolution);
  std::vector<int> begines(kScreenYResolution);

  int c_top;
  int c_bottom = -1; // empty previous column

  int i;
  for (i = vs.min_x; i <= vs.max_x; ++i) {
    if (vs.bottom[i] == -1 && c_bottom == -1) {
      continue;
    } else if (vs.bottom[i] == -1) {
      for (int j = c_bottom; j <= c_top; ++j) {
        vsp_ranges_[j].push_back({begines[j], i - 1});
      }
      c_bottom = -1;
    } else if (c_bottom == -1) {
      for (int j = vs.bottom[i]; j <= vs.top[i]; ++j) {
        begines[j] = i;
      }
      c_bottom = vs.bottom[i];
      c_top = vs.top[i];
    } else {
      for (int j = std::min(c_bottom, vs.bottom[i]); j <= std::max(c_top, vs.top[i]); ++j) {
        if (j >= c_bottom && j <= c_top && j >= vs.bottom[i] && j <= vs.top[i]) {
          continue;
        } else if (j >= c_bottom && j <= c_top) {
          vsp_ranges_[j].push_back({begines[j], i - 1});
        } else if (j >= vs.bottom[i] && j <= vs.top[i]) {
          begines[j] = i;
        }
      }
      c_bottom = vs.bottom[i];
      c_top = vs.top[i];
    }
  }

  for (int j = c_bottom; j <= c_top; ++j) {
    vsp_ranges_[j].push_back({begines[j], i - 1});
  }
}

void Renderer::RenderWalls() {
  bsp_->SetViewPoint(vp_.x, vp_.y);

  int count = 0;
  for (const auto ss : *bsp_) {
    ++count;
    if (wall_clipping_.size() == 1) {
      // All columnes are drawn
      break;
    }

    // deal with MapObjects
    for (const auto* mobj : ss->mobjs) {
      MaskedObject masked;
      if (!FillMobjMaskedObject(masked, mobj)) {
        continue;
      }
      if (masked.Clip(top_clip_, bottom_clip_)) {
        masked_.push_front(std::move(masked));
      }
    }

    // Segments
    for (const auto seg : ss->segs) {
      auto [visible, p1, p2] = GetVisibleSegment(vp_, seg->x1, seg->y1, seg->x2, seg->y2);
      if (!visible) {
        continue;
      }
  
      // Perfoms crude clipping
      if (!CreateUnclippedFragments(p1, p2)) {
        continue;
      }
  
      FillSegmentContext(seg, p1, p2);
      CreateVisplanes();
  
      if (seg->linedef->sides[1] == nullptr) {
        TexurizeWall();
      } else {
        TexurizePortal();
      }
  
      AddVisplane(top_visplane_);
      AddVisplane(bottom_visplane_);
    }
  }
}

#ifdef DEBUG_VISPLANES
  void Renderer::CheckVisplane(const Visplane& vs) {
    for (int i = vs.min_x; i <= vs.max_x; ++i) {
      //assert(vs.bottom[i] )
    }
  }
#endif

void Renderer::FillSegmentContext(const world::Segment* bsp_segment, const DPoint& left, const DPoint& right) {
  ctx_.p1 = left;
  ctx_.p2 = right;

  ctx_.sx_leftmost = bam_to_screen_x_table_[left.angle];
  ctx_.sx_rightmost = bam_to_screen_x_table_[right.angle];

  ctx_.left_distance = SegmentLength(vp_, left) * BamCos(left.angle - rend::kBamAngle45);
  ctx_.right_distance = SegmentLength(vp_, right) * BamCos(right.angle - rend::kBamAngle45);

  ctx_.line_def_flags = bsp_segment->linedef->flags;

  ctx_.full_offset = bsp_segment->offset + SegmentLength(ctx_.p1.x, ctx_.p1.y, bsp_segment->x1, bsp_segment->y1)
    + bsp_segment->linedef->sides[bsp_segment->side]->texture_offset;
  ctx_.row_offset = bsp_segment->linedef->sides[bsp_segment->side]->row_offset;

  ctx_.front_light_level = bsp_segment->linedef->sides[bsp_segment->side]->sector->light_level;
  ctx_.front_ceiling_height = bsp_segment->linedef->sides[bsp_segment->side]->sector->ceiling_height;
  ctx_.front_floor_height = bsp_segment->linedef->sides[bsp_segment->side]->sector->floor_height;

  ctx_.front_ceiling_pic = bsp_segment->linedef->sides[bsp_segment->side]->sector->ceiling_pic;
  ctx_.front_floor_pic = bsp_segment->linedef->sides[bsp_segment->side]->sector->floor_pic;

  // NB! Such a value is valid only for walls. It must be changed for each part of portals!
  ctx_.pixel_height = ctx_.front_ceiling_height - ctx_.front_floor_height;

  ctx_.segment_len = SegmentLength(ctx_.p1, ctx_.p2);

  ctx_.pixel_texture_y_shift = 0;

  // This part varies depending on the type of the secment

  ctx_.mid_texture = bsp_segment->linedef->sides[bsp_segment->side]->mid_texture;
  if (ctx_.line_def_flags & world::kLDFTwoSided) {
    ctx_.top_texture = bsp_segment->linedef->sides[bsp_segment->side]->top_texture;
    ctx_.bottom_texture = bsp_segment->linedef->sides[bsp_segment->side]->bottom_texture;
  }

  if (!(ctx_.line_def_flags & world::kLDFTwoSided)) {
    std::tie(ctx_.mid_y_bottom, ctx_.mid_y_top, ctx_.mid_dy_bottom, ctx_.mid_dy_top) 
          = CreateCoefs(ctx_.front_floor_height, ctx_.front_ceiling_height);
  } else {
    ctx_.back_ceiling_height = bsp_segment->linedef->sides[bsp_segment->side ^ 1]->sector->ceiling_height;
    ctx_.back_floor_height = bsp_segment->linedef->sides[bsp_segment->side ^ 1]->sector->floor_height;
    ctx_.back_ceiling_pic = bsp_segment->linedef->sides[bsp_segment->side ^ 1]->sector->ceiling_pic;

    // Data to calculate screen coordinates 
    if (ctx_.front_ceiling_height > ctx_.back_ceiling_height) {
      std::tie(ctx_.top_y_bottom, ctx_.top_y_top, ctx_.top_dy_bottom, ctx_.top_dy_top) 
            = CreateCoefs(ctx_.back_ceiling_height, ctx_.front_ceiling_height);
    } else {
      std::tie(ctx_.top_y_bottom, ctx_.top_y_top, ctx_.top_dy_bottom, ctx_.top_dy_top) 
            = CreateCoefs(ctx_.front_ceiling_height, ctx_.back_ceiling_height);
    }
  
    if (ctx_.front_floor_height < ctx_.back_floor_height) {
      std::tie(ctx_.bottom_y_bottom, ctx_.bottom_y_top, ctx_.bottom_dy_bottom, ctx_.bottom_dy_top)
            = CreateCoefs(ctx_.front_floor_height, ctx_.back_floor_height);
    } else {
      std::tie(ctx_.bottom_y_bottom, ctx_.bottom_y_top, ctx_.bottom_dy_bottom, ctx_.bottom_dy_top)
            = CreateCoefs(ctx_.back_floor_height, ctx_.front_floor_height);
    }
  
    ctx_.mid_y_bottom = ctx_.bottom_y_top + 1;
    ctx_.mid_y_top = ctx_.top_y_bottom - 1;
    ctx_.mid_dy_bottom = ctx_.bottom_dy_top;
    ctx_.mid_dy_top = ctx_.top_dy_bottom;
  }
}

void Renderer::CreateVisplanes() {
  // Create visplanes
  if (!top_visplane_) {
    top_visplane_.reset(new Visplane);
    top_visplane_->bottom.fill(-1);
  }
  top_visplane_->min_x = -1;
  top_visplane_->max_x = -1;
  top_visplane_->light_level = ctx_.front_light_level;
  top_visplane_->height = ctx_.front_ceiling_height;
  top_visplane_->texture = ctx_.front_ceiling_pic;

  if (!bottom_visplane_) {
    bottom_visplane_.reset(new  Visplane);
    bottom_visplane_->bottom.fill(-1);
  }
  bottom_visplane_->min_x = -1;
  bottom_visplane_->max_x = -1;
  bottom_visplane_->light_level = ctx_.front_light_level;
  bottom_visplane_->height = ctx_.front_floor_height;
  bottom_visplane_->texture = ctx_.front_floor_pic;
}

std::tuple<int, int, double, double> Renderer::CreateCoefs(int h_low, int h_high) {
  int horizon = player_feet_height_ + kPlayerHeight;

  double delta_down = static_cast<double>(horizon - h_low) * kProjectPlaneDist;
  double delta_up = static_cast<double>(h_high - horizon) * kProjectPlaneDist;

  int left_bottom = kScaleCoef * (kProjectPlaneCenter - delta_down / ctx_.left_distance);
  int left_top = kScaleCoef * (kProjectPlaneCenter + delta_up / ctx_.left_distance);

  int right_bottom = kScaleCoef * (kProjectPlaneCenter - delta_down / ctx_.right_distance);
  int right_top = kScaleCoef * (kProjectPlaneCenter + delta_up / ctx_.right_distance);

  double dy_top = static_cast<double>(right_top - left_top) / (ctx_.sx_rightmost - ctx_.sx_leftmost + 1);
  double dy_bottom = static_cast<double>(right_bottom - left_bottom) / (ctx_.sx_rightmost - ctx_.sx_leftmost + 1);

  return {left_bottom, left_top, dy_bottom, dy_top};
}

void Renderer::TexurizeWall() {
  ctx_.texture = gm_->GetTexture(ctx_.mid_texture);

  for (auto [left, right] : visible_fragments_) {
    TexurizeWallFragment(left, right);
    UpdateClipList(left, right);
  }
}

void Renderer::UpdateTopVisplane(int screen_x, int from_y) {
  if (ceiling_level_[screen_x] <= from_y || 
      ceiling_level_[screen_x] <= floor_level_[screen_x] + 1) {
    if (top_visplane_->min_x != -1) {
      top_visplane_->bottom[screen_x] = -1;
    }
    return;
  }

  int bottom = std::max(from_y, floor_level_[screen_x] + 1);

  top_visplane_->top[screen_x] = ceiling_level_[screen_x] - 1;
  top_visplane_->bottom[screen_x] = bottom;

  if (top_visplane_->min_x == -1) {
    top_visplane_->min_x = screen_x;
  }
  top_visplane_->max_x = screen_x;
}

void Renderer::UpdateBottomVisplane(int screen_x, int from_y) {
  if (floor_level_[screen_x] >= from_y ||
      ceiling_level_[screen_x] <= floor_level_[screen_x] + 1) {
    if (bottom_visplane_->min_x != -1) {
      bottom_visplane_->bottom[screen_x] = -1;
    }
    return;
  }

  int top = std::min(from_y, ceiling_level_[screen_x] - 1);
  bottom_visplane_->top[screen_x] = top;
  bottom_visplane_->bottom[screen_x] = floor_level_[screen_x] + 1;

  if (bottom_visplane_->min_x == -1) {
    bottom_visplane_->min_x = screen_x;
  }
  bottom_visplane_->max_x = screen_x;
}
  
void Renderer::UpdateFloor(int screen_x, int from_y) {
  // Closed columns must be deleted from drawing range
  assert(ceiling_level_[screen_x] != -1);
  if (from_y >= ceiling_level_[screen_x] - 1) {
    ceiling_level_[screen_x] = -1;
  } else {
    floor_level_[screen_x] = std::max(floor_level_[screen_x], from_y);
  }
}

void Renderer::UpdateCeiling(int screen_x, int from_y) {
  // Closed columns must be deleted from drawing range
  assert(ceiling_level_[screen_x] != -1);
  if (from_y <= floor_level_[screen_x] + 1) {
    ceiling_level_[screen_x] = -1;
  } else {
    ceiling_level_[screen_x] = std::min(ceiling_level_[screen_x], from_y);
  }
}

void Renderer::CheckOpening(int first, int last) {
  int start = -1;
  for (int i = first; i <= last; ++i) {
    if (ceiling_level_[i] == -1) {
      if (start == -1) {
        start = i;
      }
    } else {
      if (start != -1) {
        UpdateClipList(start, i - 1);
        start = -1;
      }
    }
  }

  if (start != -1) {
    UpdateClipList(start, last);
  }
}

void Renderer::SortMaskedObjects() {
  auto start_it = begin(masked_);
  while (start_it != end(masked_)) {
    auto end_it = std::find_if(start_it, end(masked_), [](const auto elem){ return elem.distance == -1; });
    std::list<MaskedObject> tmp;
    tmp.splice(end(tmp), masked_, start_it, end_it);
    tmp.sort([](const auto& lhs, const auto& rhs ){ return lhs.distance > rhs.distance; });
    masked_.splice(end_it, tmp);
    start_it = std::find_if(end_it, end(masked_), [](const auto elem){ return elem.distance != -1; });
  }
}

void Renderer::TexurizePortal() {
  ctx_.texture = gm_->GetTexture(ctx_.bottom_texture);
  ctx_.pixel_height = abs(ctx_.front_floor_height - ctx_.back_floor_height);
  ctx_.pixel_texture_y_shift = 0;
  for (auto [left, right] : visible_fragments_) {
    TexurizeBottomFragment(left, right);
    CheckOpening(left, right);
  }

  CreateUnclippedFragments(ctx_.p1, ctx_.p2);

  ctx_.texture = gm_->GetTexture(ctx_.top_texture);
  ctx_.pixel_height = abs(ctx_.front_ceiling_height - ctx_.back_ceiling_height);
  ctx_.pixel_texture_y_shift = 0;
  for (auto [left, right] : visible_fragments_) {
    // hack for sky
    if (ctx_.front_ceiling_pic == "F_SKY1" && ctx_.back_ceiling_pic == "F_SKY1") {
      continue;
    }
    TexurizeTopFragment(left, right);
    CheckOpening(left, right);
  }

  CreateUnclippedFragments(ctx_.p1, ctx_.p2);

  ctx_.texture = gm_->GetTexture(ctx_.mid_texture);
  if (!ctx_.texture) {
    return;
  }

  for (auto [left, right] : visible_fragments_) {
    MaskedObject msk;
    if (!FillPortalMaskedObject(msk, left, right)) {
      return;
    }

    masked_.push_front(std::move(msk));
  }
}

void Renderer::TexurizeWallFragment(int left, int right) {
  for (int x = left; x <= right; ++x) {
    // wall closes the segment
    top_clip_[x] = -1;

    if (floor_level_[x] >= ceiling_level_[x]) {
      continue;
    }

    // Screen top and bottom ends of current column
    int top = ctx_.mid_dy_top * (x - ctx_.sx_leftmost) + ctx_.mid_y_top;
    int bottom = ctx_.mid_dy_bottom * (x - ctx_.sx_leftmost) + ctx_.mid_y_bottom;

    DrawColumn(x, top, bottom, !(ctx_.line_def_flags & world::kLDFLowerUnpegged));

    UpdateTopVisplane(x, top + 1);
    UpdateBottomVisplane(x, bottom - 1);
  }
}

void Renderer::DrawColumn(int screen_x, int screen_top_y, int screen_bottom_y, bool up_to_down) {
  // No texture, e.g. portal just changes sectors but is invisible itself
  if (!ctx_.texture) {
    return;
  }
  // Below the floor
  if (screen_top_y <= floor_level_[screen_x]) {
    return;
  }
  // Above the ceiling
  if (screen_bottom_y >= ceiling_level_[screen_x]) {
    return;
  }
  // Visible part of the column
  int high = std::min(screen_top_y, ceiling_level_[screen_x] - 1);
  int low = std::max(screen_bottom_y, floor_level_[screen_x] + 1);

  // Light
  int light = 0;
  if (opt_.sector_light_enable) {
    light = (255 - ctx_.front_light_level);
  }
  if (opt_.distance_light_enable) {
    light += kDiminishFactor * screen_bottom_y / kScaleCoef;
  }
  light >>= kLightShift;
  if (light < 0) {
    light = 0;
  }
  if (light >= kLightLevels) {
    light = kLightLevels - 1;
  }
  ctx_.texture.SetLightLevel(light);

  // Texture map coords (u, v)
  // Common texture offset + offset from the start of line to bsp segment + column offset
  int u = (ctx_.full_offset + ScreenXtoTextureU(screen_x)) % ctx_.texture.GetXSize();

  double delta_v = static_cast<double>(ctx_.pixel_height) / (screen_top_y - screen_bottom_y + 1);

  int texture_y_size = ctx_.texture.GetYSize();
  if (up_to_down) {
    for (int y = high; y >= low; --y) {
      int v = static_cast<int>((screen_top_y - y) * delta_v + ctx_.pixel_texture_y_shift + ctx_.row_offset) // TODO:!!!
              % texture_y_size;

      uint32_t c = ctx_.texture.GetPixel(u, v);
      wnd_->RenderFBPointAlpha(screen_x, y, c);
    }
  } else {
    for (int y = low; y <= high; ++y) {
      int v = static_cast<int>((y - screen_bottom_y) * delta_v + ctx_.pixel_texture_y_shift + ctx_.row_offset) % texture_y_size;

      uint32_t c = ctx_.texture.GetPixel(u, texture_y_size - v - 1);
      wnd_->RenderFBPointAlpha(screen_x, y, c);
    }
  }
}

void Renderer::TexurizeBottomFragment(int left, int right) {
  for (int x = left; x <= right; ++x) {
    // Screen top and bottom ends of current column
    int top = ctx_.bottom_dy_top * (x - ctx_.sx_leftmost) + ctx_.bottom_y_top;
    int bottom = ctx_.bottom_dy_bottom * (x - ctx_.sx_leftmost) + ctx_.bottom_y_bottom;

    ctx_.pixel_texture_y_shift = (ctx_.line_def_flags & world::kLDFLowerUnpegged) 
      ? (ctx_.front_ceiling_height - ctx_.back_floor_height) : 0;

    bottom_clip_[x] = std::max(top, bottom_clip_[x]);

    if (ctx_.front_floor_height < ctx_.back_floor_height) {
      DrawColumn(x, top, bottom, true);
      UpdateBottomVisplane(x, bottom - 1);
    } else {
      UpdateBottomVisplane(x, top);
    }

    UpdateFloor(x, top);
  }
}

void Renderer::TexurizeTopFragment(int left, int right) {
  for (int x = left; x <= right; ++x) {
    // Screen top and bottom ends of current column
    int top = ctx_.top_dy_top * (x - ctx_.sx_leftmost) + ctx_.top_y_top;
    int bottom = ctx_.top_dy_bottom * (x - ctx_.sx_leftmost) + ctx_.top_y_bottom;

    top_clip_[x] = std::min(bottom, top_clip_[x]);

    if (ctx_.front_ceiling_height > ctx_.back_ceiling_height) {
      DrawColumn(x, top, bottom, (ctx_.line_def_flags & world::kLDFUpperUnpegged));
      UpdateTopVisplane(x, top + 1);
    } else {
      UpdateTopVisplane(x, bottom);
    }

    UpdateCeiling(x, bottom);
  }
}

bool Renderer::CreateUnclippedFragments(const DPoint& left, const DPoint& right) {
  visible_fragments_.resize(0);

  int sx_left = bam_to_screen_x_table_[left.angle];
  int sx_right = bam_to_screen_x_table_[right.angle];

  for (auto it = begin(wall_clipping_); it != end(wall_clipping_); ++it) {
    if (it->last < sx_left) {
      continue;
    } else if (sx_right < it->first) {
      visible_fragments_.push_back({sx_left, sx_right});
      break;
    } else if (it->first <= sx_left && it->first <= sx_right && 
              it->last >= sx_left && it->last >= sx_right) {
      break;
    } else if (it->first <= sx_right && sx_right <= it->last) {
      visible_fragments_.push_back({sx_left, it->first - 1});
      break;
    } else if (it->first <= sx_left && sx_left <= it->last) {
      sx_left = it->last + 1;
    } else {
      visible_fragments_.push_back({sx_left, it->first - 1});
      sx_left = it->last + 1;
    }
  }

  return !visible_fragments_.empty();
}

void Renderer::UpdateClipList(int first, int last) {
  for (auto it = begin(wall_clipping_); it != end(wall_clipping_); ++it) {
    if (it->first > last + 1) {
      wall_clipping_.insert(it, {first, last});
      return;
    } else if (it->first == last + 1) {
      it->first = first;
      return;
    } else if (it->last + 1 == first) {
      it->last = last;

      auto next_elem_it = next(it);
      if (next_elem_it != end(wall_clipping_)) {
        if (it->last + 1 == next_elem_it->first) {
          it->last = next_elem_it->last;
          wall_clipping_.erase(next_elem_it);
        }
      }
      return;
    }
  }
}

void Renderer::AddVisplane(std::unique_ptr<Visplane>& vs) {
  if (vs->min_x == -1) {
    return;
  }

  visplanes_.push_back(std::move(vs));
}

int Renderer::ScreenXtoTextureU(int sx) {
  double a = (sx - ctx_.sx_leftmost) / (ctx_.sx_rightmost - ctx_.sx_leftmost + 1.0);

  int ret = (a * ctx_.segment_len / ctx_.right_distance) / ((1 - a) / ctx_.left_distance + a / ctx_.right_distance);
  return ret;
}

void Renderer::ClearRenderer() {
//  wnd_->ClearRender(0, 0, 0, 0);
  wnd_->ClearFBRender(0, 0, 0, 255);

  wall_clipping_.clear();
  wall_clipping_.push_back({std::numeric_limits<int>::min(), -1});
  wall_clipping_.push_back({kScreenXResolution, std::numeric_limits<int>::max()});

  floor_level_.fill(-1);
  ceiling_level_.fill(kScreenYResolution);

  // visible_fragments_
  visplanes_.clear();

  top_clip_.assign(top_clip_.size(), kScreenYResolution);
  bottom_clip_.assign(bottom_clip_.size(), -1);
  masked_.clear();
}

bool Renderer::FillMobjMaskedObject(MaskedObject& msk, const mobj::MapObject* mobj) {
    auto texture_name = mobj->GetSpriteName(vp_.x, vp_.y);
    msk.texture = gm_->GetSpriteEx(texture_name);
    // TODO: handle with strange texture frame indexes
    if (!msk.texture) {
      return false;
    }
    
    msk.width = msk.texture.GetXSize();
    msk.height = msk.texture.GetYSize();

    // TODO: might be it would be better to place this piece in spawn code but
    // it requires getting texture size...
    if (mobj->flags & mobj::MF_SPAWNCEILING) {
      msk.z = mobj->ss->sector->ceiling_height - msk.height;
    } else {
      msk.z = mobj->z;
    }

    auto angle = CalcAngle(vp_.x, vp_.y, mobj->x, mobj->y);
    int dx = msk.width * BamCos(kBamAngle90 - angle);
    int dy = msk.width * BamSin(kBamAngle90 - angle);
    
    int x1 = mobj->x - dx / 2;
    int y1 = mobj->y + dy / 2;
    int x2 = x1 + dx;
    int y2 = y1 - dy;

    auto [visible, p1, p2] = GetVisibleSegment(vp_, x1, y1, x2, y2);
    if (!visible) {
      return false;
    }

    msk.p1 = p1;
    msk.p2 = p2;
    msk.full_offset = SegmentLength(x1, y1, p1.x, p1.y);

    msk.distance = sqrt((mobj->x - vp_.x)*(mobj->x - vp_.x) + (mobj->y - vp_.y)*(mobj->y - vp_.y));
    msk.sx_leftmost = bam_to_screen_x_table_[p1.angle];
    msk.sx_rightmost = bam_to_screen_x_table_[p2.angle];

    msk.left = msk.sx_leftmost;
    msk.right = msk.sx_rightmost;

    msk.top_clip.assign(msk.sx_rightmost - msk.sx_leftmost + 1, kScreenYResolution);
    msk.bottom_clip.assign(msk.sx_rightmost - msk.sx_leftmost + 1, -1);

    return true;
}

bool Renderer::FillPortalMaskedObject(MaskedObject& msk, int left, int right) {
  msk.texture = gm_->GetTexture(ctx_.mid_texture);

  msk.z = std::max(ctx_.back_floor_height, ctx_.front_floor_height);
  msk.height = std::min(ctx_.front_ceiling_height, ctx_.back_ceiling_height) - msk.z;
  msk.pixel_texture_y_shift = 0;

  msk.p1 = ctx_.p1;
  msk.p2 = ctx_.p2;

  // Data to calculate screen coordinates (screen x of seg's ends)
  msk.sx_leftmost = ctx_.sx_leftmost;
  msk.sx_rightmost = ctx_.sx_rightmost;

  msk.left = left;
  msk.right = right;

  msk.full_offset = ctx_.full_offset;

  // -1 for portals!!! 
  msk.distance = -1;

  msk.top_clip.resize(msk.sx_rightmost - msk.sx_leftmost + 1);
  msk.bottom_clip.resize(msk.sx_rightmost - msk.sx_leftmost + 1);

  int count = 0;
  for (int i = msk.sx_leftmost; i <= msk.sx_rightmost; ++i) {
    if (ceiling_level_[i] == -1 || (ceiling_level_[i] <= floor_level_[i] + 1)) {
      msk.top_clip[i - msk.sx_leftmost] = -1;
      ++count;
    } else {
      msk.top_clip[i - msk.sx_leftmost] = ceiling_level_[i];
      msk.bottom_clip[i - msk.sx_leftmost] = floor_level_[i];
    }
  }

  return count != (msk.sx_rightmost - msk.sx_leftmost + 1);
}

} // namespace rend
