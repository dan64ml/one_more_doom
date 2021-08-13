#include "renderer.h"
//#include "wad/texture.h"

#include <limits>
#include <iostream>
#include <cassert>

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
  vp_.angle = DegreesToBam(player->angle);  // TODO!!!!!
  
  ClearRenderer();

  //CreateVissprites({});

  RenderWalls();
  RenderFlats();
  RenderMasked();

  auto finish = std::chrono::steady_clock::now();
  auto dur = finish - start;

  int msec = std::chrono::duration_cast<std::chrono::milliseconds>(dur).count();
  wnd_->PrintString("msec = " + std::to_string(msec) + ", fps = " + std::to_string(1000 / msec));

  wnd_->ShowFBRender();
}

void Renderer::RenderFlats() {
  for (const auto& vs : visplanes_) {
    CreateRanges(*(vs.get()));
    DrawPixelRange(*(vs.get()));
  }
}

void Renderer::RenderMasked() {
  // Hack to simplify DrawColumn() function
  ceiling_level_.fill(kScreenYResolution);
  floor_level_.fill(-1);

  for (const auto& msk : masked_) {
    DrawMaskedObject(msk);
  }
}

void Renderer::DrawMaskedObject(const MaskedObject& msk) {
  FillContext(msk);

  for (int x = ctx_.sx_leftmost; x <= ctx_.sx_rightmost; ++x) {
    if (msk.top_clip[x - ctx_.sx_leftmost] == -1) {
      continue;
    }

    // Screen top and bottom ends of current column
    int top = ctx_.mid_dy_top * (x - ctx_.sx_leftmost) + ctx_.mid_y_top;
    int bottom = ctx_.mid_dy_bottom * (x - ctx_.sx_leftmost) + ctx_.mid_y_bottom;

    DrawColumn(x, top, bottom, !(ctx_.line_def_flags & world::kLDFLowerUnpegged));
  }
}

void Renderer::FillContext(const MaskedObject& msk) {
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

  // Sprites and portals have different texture sources
  // Also the height of portal is not equal to texture size 
  if (msk.distance == -1) {
    ctx_.texture = gm_->GetTexture(msk.text);
    ctx_.pixel_height = msk.height;
  } else {
    ctx_.texture = gm_->GetSprite(msk.text);
    ctx_.pixel_height = ctx_.texture.GetYSize();
  }

  // TODO: ??? light_level and floor_height (for flying mobjs)

  // TODO: ??? Redundunt - name and Texture...
  ctx_.mid_texture = msk.text;

  // For texturing
  ctx_.segment_len = SegmentLength(ctx_.p1, ctx_.p2); //sqrt((ctx_.p1.x - ctx_.p2.x)*(ctx_.p1.x - ctx_.p2.x) + (ctx_.p1.y - ctx_.p2.y)*(ctx_.p1.y - ctx_.p2.y));

  std::tie(ctx_.mid_y_bottom, ctx_.mid_y_top, ctx_.mid_dy_bottom, ctx_.mid_dy_top) 
        = CreateCoefs(msk.z, msk.z + ctx_.pixel_height);

  ctx_.pixel_texture_y_shift = 0;
}


/*void Renderer::CreateVissprites(const Scene& scene) {
  vissprites_.clear();

  for (const thing::VisibleThing& item : scene.items_) {
    auto [sp_x, sp_y] = item.GetPosition();
    auto [width, height] = item.GetSpriteSize(vp_.angle);

    auto angle = CalcAngle(vp_.x, vp_.y, sp_x, sp_y);
//    int dx = (width / 2.0) * BamCos(kBamAngle90 - angle);
//    int dy = (width / 2.0) * BamSin(kBamAngle90 - angle);
//    auto [visible, p1, p2] = GetVisibleSegment(vp_, sp_x - dx, sp_y + dy, sp_x + dx, sp_y - dy);
    int dx = width * BamCos(kBamAngle90 - angle);
    int dy = width * BamSin(kBamAngle90 - angle);
    int lx = sp_x - dx / 2;
    int ly = sp_y + dy / 2;
    auto [visible, p1, p2] = GetVisibleSegment(vp_, lx, ly, lx + dx, ly - dy);
    if (!visible) {
      continue;
    }

//    double offset = sqrt((p1.x - (sp_x - dx))*(p1.x - (sp_x - dx)) + (p1.y - (sp_y + dy))*(p1.y - (sp_y + dy)));
    double offset = sqrt((p1.x - lx)*(p1.x - lx) + (p1.y - ly)*(p1.y - ly));
    double dist = sqrt((sp_x - vp_.x)*(sp_x - vp_.x) + (sp_y - vp_.y)*(sp_y - vp_.y));
    int left = bam_to_screen_x_table_[p1.angle];
    int right = bam_to_screen_x_table_[p2.angle];

    vissprites_.emplace_back(left, right, dist, item, p1, p2, DPoint{sp_x, sp_y, 0}, offset);
  }
}*/

/*void Renderer::UpdateVssMaskByWall(int left, int right) {
  for (auto& vs : vissprites_) {
    if (vs.x_left > right || vs.x_right < left) {
      continue;
    }
    if (DefineVpSide(ctx_.p1.x, ctx_.p1.y, ctx_.p2.x, ctx_.p2.y, vs.p_center.x, vs.p_center.y) == 0) {
      continue;
    }

    const auto [from, to] = CreateIntersection(left, right, vs.x_left, vs.x_right);
    for (int i = from; i <= to; ++i) {
      if (vs.top[i - vs.x_left] != -1) {
        vs.top[i - vs.x_left] = -1;
        ++vs.masked_column_count;
      }
    }
  }

  for (auto it = begin(vissprites_); it != end(vissprites_);) {
    if (it->masked_column_count == (it->x_right - it->x_left + 1)) {
      it = vissprites_.erase(it);
    } else {
      ++it;
    }
  }
}*/

/*void Renderer::UpdateVssMaskByPortal(int left, int right) {
  for (auto& vs : vissprites_) {
    if (vs.x_left > right || vs.x_right < left) {
      continue;
    }
    if (DefineVpSide(ctx_.p1.x, ctx_.p1.y, ctx_.p2.x, ctx_.p2.y, vs.p_center.x, vs.p_center.y) == 0) {
      continue;
    }

    const auto [from, to] = CreateIntersection(left, right, vs.x_left, vs.x_right);
    for (int i = from; i <= to; ++i) {
      if (vs.top[i - vs.x_left] == -1) {
        continue;
      }

      int top = ctx_.mid_y_top + ctx_.mid_dy_top * (i - ctx_.sx_leftmost);
      int bottom = ctx_.mid_y_bottom + ctx_.mid_dy_bottom * (i - ctx_.sx_leftmost);

      vs.top[i - vs.x_left] = std::min(vs.top[i - vs.x_left], top);
      vs.bottom[i - vs.x_left] = std::max(vs.bottom[i - vs.x_left], bottom);
      if (vs.bottom[i - vs.x_left] + 1 >= vs.top[i - vs.x_left]) {
        vs.top[i - vs.x_left] = -1;
        ++vs.masked_column_count;
      }
    }
  }

  for (auto it = begin(vissprites_); it != end(vissprites_);) {
    if (it->masked_column_count == (it->x_right - it->x_left + 1)) {
      it = vissprites_.erase(it);
    } else {
      ++it;
    }
  }
}*/

/*void Renderer::SortVissprites() {
  auto comp = [](const Vissprite& l, const Vissprite& r) -> bool {
      return l.dist > r.dist;
  };

  vissprites_.sort(comp);
}*/

/*void Renderer::DrawSprite(const Vissprite& vs) {
  auto [sp_widht, sp_height] = vs.sprite->GetSpriteSize(vp_.angle);

  double const dist_left = sqrt((vs.p1.x - vp_.x)*(vs.p1.x - vp_.x) + (vs.p1.y - vp_.y)*(vs.p1.y - vp_.y))
      * BamCos(vs.p1.angle - kBamAngle45);
  double const dist_right = sqrt((vs.p2.x - vp_.x)*(vs.p2.x - vp_.x) + (vs.p2.y - vp_.y)*(vs.p2.y - vp_.y))
      * BamCos(vs.p2.angle - kBamAngle45);

  int top_y_left = (vs.sprite->GetHeight() + sp_height - (kPlayerHeight + player_feet_height_)) 
                    * (static_cast<double>(kProjectPlaneDist) / dist_left) + kProjectPlaneCenter;
  int top_y_right = (vs.sprite->GetHeight() + sp_height - (kPlayerHeight + player_feet_height_)) 
                    * (static_cast<double>(kProjectPlaneDist) / dist_right) + kProjectPlaneCenter;


  int bottom_y_left = kProjectPlaneCenter - (kPlayerHeight + player_feet_height_ - vs.sprite->GetHeight())
                      * (static_cast<double>(kProjectPlaneDist) / dist_left);
  int bottom_y_right = kProjectPlaneCenter - (kPlayerHeight + player_feet_height_ - vs.sprite->GetHeight())
                      * (static_cast<double>(kProjectPlaneDist) / dist_right);

  top_y_left *= kScaleCoef;
  top_y_right *= kScaleCoef;
  bottom_y_left *= kScaleCoef;
  bottom_y_right *= kScaleCoef;
  double dy_top = (top_y_right - top_y_left) / (vs.x_right - vs.x_left + 1.0);
  double dy_bottom = (bottom_y_right - bottom_y_left) / (vs.x_right - vs.x_left + 1.0);

  const auto& texture = vs.sprite->GetSprite(vp_.angle);
  double len = sqrt((vs.p1.x - vs.p2.x)*(vs.p1.x - vs.p2.x) + (vs.p1.y - vs.p2.y)*(vs.p1.y - vs.p2.y));

  for (int x = vs.x_left; x <= vs.x_right; ++x) {
    if (vs.top[x - vs.x_left] == -1) {
      continue;
    }

    double a = (x - vs.x_left) / (vs.x_right - vs.x_left + 1.0);
    int u = (a * len / dist_right) / ((1 - a) / dist_left + a / dist_right) + vs.offset;

    int top_y = top_y_left + (x - vs.x_left) * dy_top;
    int bottom_y = bottom_y_left + (x - vs.x_left) * dy_bottom;
    double texture_y_coef = sp_height / (top_y - bottom_y + 1.0);

    for (int y = std::max(vs.bottom[x - vs.x_left] + 1, bottom_y);
         y <= std::min(vs.top[x - vs.x_left] - 1, top_y); ++y) {
      int v = (top_y - y) * texture_y_coef;

      int r, g, b;
      if (texture.GetPixel(u, v, r, g, b)) {
        wnd_->RenderFBPoint(x, y, r, g, b);


        for (auto& mp : mid_portals_) {
          if (x < mp.context.sx_leftmost || x > mp.context.sx_rightmost) {
            continue;
          }
          if(DefineVpSide(mp.context.p1.x, mp.context.p1.y, mp.context.p2.x, mp.context.p2.y, vs.p_center.x, vs.p_center.y) == 1) {
            continue;
          }

          mp.mask[x - mp.context.sx_leftmost][y] = true;
        }
    
      }
    }
  }

}*/

/*void Renderer::RenderMasked() {
  SortVissprites();

  for (const auto& vs : vissprites_) {
    DrawSprite(vs);
  }

  for (auto it = std::rbegin(mid_portals_); it != std::rend(mid_portals_); ++it) {
    TexurizeMidFragment(*it);
  }

  // Temporary!!! Draw status bar
  auto texture = map_->GetGraphicsManager().GetSprite("STBAR");
  for (int x = 0; x < kScreenXResolution; ++x) {
    for (int y = 0; y < 32 * kScaleCoef; ++y) {
      int r, g, b;
      texture.GetPixel(x / kScaleCoef, (32 * kScaleCoef - y - 1) / kScaleCoef, r, g, b);
      wnd_->RenderFBPoint(x, y, r, g, b);
    }
  }
}*/

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
      if (!FillMaskedObject(masked, mobj)) {
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

    //UpdateVssMaskByWall(left, right);
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
  floor_level_[screen_x] = std::max(floor_level_[screen_x], 
    std::min(from_y, ceiling_level_[screen_x] - 1));
}

void Renderer::UpdateCeiling(int screen_x, int from_y) {
  ceiling_level_[screen_x] = std::min(ceiling_level_[screen_x],
    std::max(from_y, floor_level_[screen_x] + 1));
}

void Renderer::TexurizePortal() {
  ctx_.texture = gm_->GetTexture(ctx_.bottom_texture);
  ctx_.pixel_height = abs(ctx_.front_floor_height - ctx_.back_floor_height);
  ctx_.pixel_texture_y_shift = 0;
  for (auto [left, right] : visible_fragments_) {
    TexurizeBottomFragment(left, right);

    //UpdateVssMaskByPortal(left, right);
  }

  ctx_.texture = gm_->GetTexture(ctx_.top_texture);
  ctx_.pixel_height = abs(ctx_.front_ceiling_height - ctx_.back_ceiling_height);
  ctx_.pixel_texture_y_shift = 0;
  for (auto [left, right] : visible_fragments_) {
    TexurizeTopFragment(left, right);
  }


  ctx_.texture = gm_->GetTexture(ctx_.mid_texture);
  if (!ctx_.texture) {
    return;
  }
//  ctx_.pixel_height = abs(ctx_.front_ceiling_height - ctx_.front_floor_height);
//  ctx_.pixel_texture_y_shift = 0;

  // Create pseudo-vissprite
//  MidPortalVisplane mpv(ctx_, floor_level_, ceiling_level_, visible_fragments_);
//  mid_portals_.push_back(std::move(mpv));

  MaskedObject msk;
  if (!FillPortalMaskedObject(msk)) {
    return;
  }
  if (msk.Clip(top_clip_, bottom_clip_)) {
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

void Renderer::DrawMaskedColumn(int screen_x, int screen_top_y, int screen_bottom_y, bool up_to_down, MidPortalVisplane& mpv) {
  // No texture
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
  //int high = std::min(screen_top_y, kScreenYResolution - 1);
  //int low = std::max(screen_bottom_y, 0);

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
//  int u = (ctx_.front_side_def->texture_offset + ctx_.full_offset 
//          + ScreenXtoTextureU(screen_x)) % ctx_.texture.GetXSize();
  int u = (ctx_.full_offset + ScreenXtoTextureU(screen_x)) % ctx_.texture.GetXSize();

  double delta_v = static_cast<double>(ctx_.pixel_height) / (screen_top_y - screen_bottom_y + 1);

  int texture_y_size = ctx_.texture.GetYSize();
  int r, g, b;
  if (up_to_down) {
    for (int y = high; y >= low; --y) {
      if (mpv.mask[screen_x - ctx_.sx_leftmost][y]) {
        continue;
      }
      int v = static_cast<int>((screen_top_y - y) * delta_v + ctx_.pixel_texture_y_shift + ctx_.row_offset) // TODO:!!!
              % texture_y_size;

      //if (ctx_.texture.GetPixel(u, v, r, g, b)) {
      //  wnd_->RenderFBPoint(screen_x, y, r, g, b);
      //}
      uint32_t c = ctx_.texture.GetPixel(u, v);
      wnd_->RenderFBPointAlpha(screen_x, y, c);
    }
  } else {
    for (int y = low; y <= high; ++y) {
      if (mpv.mask[screen_x - ctx_.sx_leftmost][y]) {
        continue;
      }
      int v = static_cast<int>((y - screen_bottom_y) * delta_v + ctx_.pixel_texture_y_shift + ctx_.row_offset) % texture_y_size;

      //if (ctx_.texture.GetPixel(u, texture_y_size - v - 1, r, g, b)) {
      //  wnd_->RenderFBPoint(screen_x, y, r, g, b);
      //}
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

void Renderer::TexurizeMidFragment(MidPortalVisplane& mpv) {
  ctx_ = mpv.context;
  floor_level_ = mpv.floor_level;
  ceiling_level_ = mpv.ceiling_level;

  for (auto [left, right] : mpv.visible_fragments) {
    for (int x = left; x <= right; ++x) {
      // Screen top and bottom ends of current column
      int top = ctx_.mid_dy_top * (x - ctx_.sx_leftmost) + ctx_.mid_y_top;
      int bottom = ctx_.mid_dy_bottom * (x - ctx_.sx_leftmost) + ctx_.mid_y_bottom;
  
      DrawMaskedColumn(x, top, bottom, !(ctx_.line_def_flags & world::kLDFLowerUnpegged), 
        mpv);
    }
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
  // TODO: add length of line part out of FOV!!!!
  //return ctx_.delta_u * (sx - ctx_.sx_leftmost) /*+ ctx_.bsp_segment->offset*/;

  double a = (sx - ctx_.sx_leftmost) / (ctx_.sx_rightmost - ctx_.sx_leftmost + 1.0);

  int ret = (a * ctx_.segment_len / ctx_.right_distance) / ((1 - a) / ctx_.left_distance + a / ctx_.right_distance);
  return ret;
}

void Renderer::ClearRenderer() {
  wnd_->ClearRender(0, 0, 0, 0);

  wall_clipping_.clear();
  wall_clipping_.push_back({std::numeric_limits<int>::min(), -1});
  wall_clipping_.push_back({kScreenXResolution, std::numeric_limits<int>::max()});

  floor_level_.fill(-1);
  ceiling_level_.fill(kScreenYResolution);

  mid_portals_.clear();
  //vissprites_.clear();

  // visible_fragments_
   visplanes_.clear();
  // top_visplane_
  // bottom_visplane_
  // vsp_ranges_

  top_clip_.assign(top_clip_.size(), kScreenYResolution);
  bottom_clip_.assign(bottom_clip_.size(), -1);
  masked_.clear();
}

bool Renderer::FillMaskedObject(MaskedObject& msk, const mobj::MapObject* mobj) {
    //int width = mobj->radius;
    msk.height = mobj->height;
    msk.z = mobj->z;

    msk.text = mobj->texture;

    //auto [width, height] = item.GetSpriteSize(vp_.angle);

    auto angle = CalcAngle(vp_.x, vp_.y, mobj->x, mobj->y);
//    int dx = mobj->radius * BamCos(kBamAngle90 - angle);
//    int dy = mobj->radius * BamSin(kBamAngle90 - angle);
    int dx = 26 * BamCos(kBamAngle90 - angle);
    int dy = 26 * BamSin(kBamAngle90 - angle);

//    int lx = mobj->x - dx / 2;
//    int ly = mobj->y + dy / 2;
    
    msk.x1 = mobj->x - dx / 2;
    msk.y1 = mobj->y + dy / 2;
    msk.x2 = msk.x1 + dx;
    msk.y2 = msk.y1 - dy;

    auto [visible, p1, p2] = GetVisibleSegment(vp_, msk.x1, msk.y1, msk.x2, msk.y2);
    if (!visible) {
      return false;
    }

    msk.p1 = p1;
    msk.p2 = p2;
    msk.full_offset = SegmentLength(msk.x1, msk.y1, p1.x, p1.y);

    msk.distance = sqrt((mobj->x - vp_.x)*(mobj->x - vp_.x) + (mobj->y - vp_.y)*(mobj->y - vp_.y));
    msk.sx_leftmost = bam_to_screen_x_table_[p1.angle];
    msk.sx_rightmost = bam_to_screen_x_table_[p2.angle];

    msk.top_clip.assign(msk.sx_rightmost - msk.sx_leftmost + 1, kScreenYResolution);
    msk.bottom_clip.assign(msk.sx_rightmost - msk.sx_leftmost + 1, -1);

    return true;
}

bool Renderer::FillPortalMaskedObject(MaskedObject& msk) {
  // Don't use
  msk.x1;
  msk.y1;
  msk.x2;
  msk.y2;
  
  msk.text = ctx_.mid_texture;

  msk.z = std::max(ctx_.back_floor_height, ctx_.front_floor_height);
  msk.height = std::min(ctx_.front_ceiling_height, ctx_.back_ceiling_height) - msk.z;
  msk.pixel_texture_y_shift = 0;

  msk.p1 = ctx_.p1;
  msk.p2 = ctx_.p2;

  // Data to calculate screen coordinates (screen x of seg's ends)
  msk.sx_leftmost = ctx_.sx_leftmost;
  msk.sx_rightmost = ctx_.sx_rightmost;

  msk.full_offset = ctx_.full_offset;

  // -1 for portals!!! 
  msk.distance = -1;

  msk.top_clip.resize(ctx_.sx_rightmost - ctx_.sx_leftmost + 1);
  msk.bottom_clip.resize(ctx_.sx_rightmost - ctx_.sx_leftmost + 1);

  int count = 0;
  for (int i = ctx_.sx_leftmost; i <= ctx_.sx_rightmost; ++i) {
    if (ceiling_level_[i] == -1 || (ceiling_level_[i] <= floor_level_[i] + 1)) {
      msk.top_clip[i - ctx_.sx_leftmost] = -1;
      ++count;
    } else {
      msk.top_clip[i - ctx_.sx_leftmost] = ceiling_level_[i];
      msk.bottom_clip[i - ctx_.sx_leftmost] = floor_level_[i];
    }
  }

  return !(count == (ctx_.sx_rightmost - ctx_.sx_leftmost + 1));
}

} // namespace rend
