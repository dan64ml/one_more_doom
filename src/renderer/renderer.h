#ifndef RENDERER_H_
#define RENDERER_H_

#include <vector>
#include <list>

#include "app/sdl_window.h"
#include "world/fast_bsp.h"
#include "objects/player.h"
#include "objects/map_object.h"
#include "graphics/graphics_manager.h"
#include "graphics/texture.h"

#include "plane_utils.h"
#include "masked_object.h"

namespace rend {

const int kScaleCoef = 8;
const int kOriginXResolution = 320;
const int kOriginYResolution = 200;
const int kScreenXResolution = kOriginXResolution * kScaleCoef;
const int kScreenYResolution = kOriginYResolution * kScaleCoef;

const int kProjectPlaneDist = 160;  //160;
const int kProjectPlaneCenter = 132;

const int kPlayerHeight = 56;

const int kLightLevels = 32;
const int kLightShift = 4;
const double kDiminishFactor = 0.1;

struct RendererOptions {
  bool sector_light_enable = false;
  bool distance_light_enable = false;
};

struct SegmentRendContext {
  // Data related to the current segment
  //const world::Segment* bsp_segment;
  //const world::Line* line_def;
  uint32_t line_def_flags;

  //const world::SideDef* front_side_def;
  //const world::Sector* front_sector;
  int front_light_level;
  int front_ceiling_height;
  int front_floor_height;
  std::string front_ceiling_pic;
  std::string front_floor_pic;

  // Valid for portals only
  //const world::SideDef* back_side_def;
//  const world::Sector* back_sector;
  int back_ceiling_height;
  int back_floor_height;

  // Distances to visible ends of the segment
  double left_distance;
  double right_distance;

  // Distance from the start of linedef to the left end of the segment
  // NB! MUST includes SideDef::texture_offset
  int full_offset;
  // Instead of SideDef::row_offset
  int row_offset;

  std::string mid_texture;
  std::string top_texture;
  std::string bottom_texture;

  // And their coordinates
  DPoint p1;
  DPoint p2;

  // Height of current element in map pixels
  int pixel_height;
  // Vertical shift, used with lower unpegged bottom texture 
  int pixel_texture_y_shift;
  // Current texture (can be empty)
  graph::Texture texture;
  // Visible segment phys lenght. Necessary for texturing
  double segment_len;

  // Data to calculate screen coordinates (screen x of seg's ends)
  int sx_leftmost;
  int sx_rightmost;

  // For walls and middle part of portals
  int mid_y_top;
  int mid_y_bottom;
  double mid_dy_top;
  double mid_dy_bottom;
  // For bottom part of portals
  int bottom_y_top;
  int bottom_y_bottom;
  double bottom_dy_top;
  double bottom_dy_bottom;
  // For top part of portals
  int top_y_top;
  int top_y_bottom;
  double top_dy_top;
  double top_dy_bottom;
};

struct PixelRange {
  int first;
  int last;
};

struct Visplane {
  int height;
  int picnum;
  int light_level;
  int min_x = -1;
  int max_x = -1;
  std::string texture;

  std::array<int, kScreenXResolution> top;
  std::array<int, kScreenXResolution> bottom;
};

struct MidPortalVisplane {
  MidPortalVisplane(const SegmentRendContext& ctx, const std::array<int, kScreenXResolution>& floor_level,
    const std::array<int, kScreenXResolution>& ceiling_level, const std::vector<PixelRange>& visible_fragments) 
    : context(ctx), floor_level(floor_level), ceiling_level(ceiling_level), visible_fragments(visible_fragments) {
      mask.assign(ctx.sx_rightmost - ctx.sx_leftmost + 1, std::vector<bool>(kScreenYResolution, false));
    }

  SegmentRendContext context;
  std::array<int, kScreenXResolution> floor_level;
  std::array<int, kScreenXResolution> ceiling_level;
  std::vector<PixelRange> visible_fragments;

  std::vector<std::vector<bool>> mask;
};

/*struct Vissprite {
  Vissprite() = default;
  Vissprite(int left, int right, double d, const thing::VisibleThing& sp, DPoint p1, DPoint p2, DPoint p_center, double offset)
    : x_left(left), x_right(right), dist(d), sprite(&sp), p1(p1), p2(p2), p_center(p_center), offset(offset),
    top(right - left + 1, rend::kScreenYResolution), bottom(right - left + 1, -1) {}

  // Screen x coordinates of sprite location
  int x_left;
  int x_right;

  // Distance from view point to the sprite center
  double dist;

  const thing::VisibleThing* sprite;
  
  // Visible ends of the sprite
  DPoint p1, p2;
  // And the center
  DPoint p_center;
  // Distance from left end of the sprite to the left visible end
  double offset;

  std::vector<int> top; // == -1 if the column if fully masked
  std::vector<int> bottom;

  int masked_column_count = 0;
};*/

class Renderer {
 public:
  Renderer();

  //void RenderScene(const Scene& scene);
  void RenderScene(const wad::FastBsp* bsp, const graph::GraphicsManager* gm, const mobj::Player* player);

  void SetDrawWindow(sdl2::SdlWindow* wnd) { wnd_ = wnd; }

 private:
  void ClearRenderer();
  void RenderWalls();
  void RenderFlats();
  void RenderMasked();

  // Segment can be partially closed by other elements. This foo creates a list
  // of visible fragments and some extra params. Returns false if all the segment
  // is clipped
  bool CreateUnclippedFragments(const DPoint& left, const DPoint& right);
  void UpdateClipList(int first, int last);

  //void FillCommonContext(const DPoint& left, const DPoint& right);
  void FillCommonContext(const world::Segment* bsp_segment, const DPoint& left, const DPoint& right);
  void FillWallContext(const DPoint& left, const DPoint& right);
  //void FillPortalContext();
  void FillPortalContext(const world::Segment* bsp_segment);

  void TexurizeWall();
  void TexurizeWallFragment(int left, int right);

  void TexurizePortal();
  void TexurizeBottomFragment(int left, int right);
  void TexurizeTopFragment(int left, int right);

  void DrawColumn(int screen_x, int screen_top_y, int screen_bottom_y, bool up_to_down);

  void UpdateTopVisplane(int screen_x, int from_y);
  void UpdateBottomVisplane(int screen_x, int from_y);
  void AddVisplane(std::unique_ptr<Visplane>& vs);

  void UpdateFloor(int screen_x, int from_y);
  void UpdateCeiling(int screen_x, int from_y);

  std::tuple<int, int, double, double> CreateCoefs(int h_low, int h_high);

  // Converts screen x coordinate to texture u coordinate
  int ScreenXtoTextureU(int sx);

  sdl2::SdlWindow* wnd_;
  const graph::GraphicsManager* gm_;
  const wad::FastBsp* bsp_;

  RendererOptions opt_;

  // Player position
  DPoint vp_;
  // May differ from floor level, eg during a jump
  int player_feet_height_;

  // Lookup table to convert BamAngle to screen x coord
  std::array<int, kBamAngle180 + 1> bam_to_screen_x_table_;
  // Contains fully clipped fragments of screen
  std::list<PixelRange> wall_clipping_;

  // Contain floor and ceiling level for every column of screen
  std::array<int, kScreenXResolution> floor_level_;
  std::array<int, kScreenXResolution> ceiling_level_;

  // Some data for segment rendering
  SegmentRendContext ctx_;

  // Visible fragments of the current segment. Filled by CreateUnclippedFragments().
  std::vector<PixelRange> visible_fragments_;

  std::vector<std::unique_ptr<Visplane>> visplanes_;

  std::unique_ptr<Visplane> top_visplane_;
  std::unique_ptr<Visplane> bottom_visplane_;

  // TODO: Optimize!!!
  std::vector<std::vector<PixelRange>> vsp_ranges_;
  int min_vsp_y_;
  int max_vsp_y_;
  void CreateRanges(const Visplane& vs);
  void DrawPixelRange(const Visplane& vs);

  // Vissprites
  //std::list<Vissprite> vissprites_;
  // Creates list of visible sprites
  //void CreateVissprites(const Scene& scene);
  // Using current fragment data update masks of vissprites.
  // If vissprite is fully masked, delete it from the list 
  //void UpdateVssMaskByWall(int left, int right);
  //void UpdateVssMaskByPortal(int left, int right);
  // Sorts vissprites_ by distance (from farthest to nearest)
  //void SortVissprites();
  // Draw the sprite
  //void DrawSprite(const Vissprite& vs);


  // TOD0: there is lots of very dirty code!
  void TexurizeMidFragment(MidPortalVisplane& mpv);
  void DrawMaskedColumn(int screen_x, int screen_top_y, int screen_bottom_y, bool up_to_down, 
    MidPortalVisplane& mpv);

  std::vector<MidPortalVisplane> mid_portals_;

  std::list<MaskedObject> masked_;
  // top_clip[i] == -1 means fully closed column
  std::vector<int> top_clip_;
  std::vector<int> bottom_clip_;

  // Returns false if the object is invisible
  bool FillMaskedObject(MaskedObject& msk, const mobj::MapObject* mobj);
  // Fills using contex
  bool FillMaskedObject(MaskedObject& msk);

  void DrawMaskedObject(const MaskedObject& msk);
  void FillContext(const MaskedObject& msk);
};

} // namespace rend

#endif  // RENDERER_H_
