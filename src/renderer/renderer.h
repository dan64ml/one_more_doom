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

#define DEBUG_CODE

#ifdef DEBUG_CODE
  #define DEBUG_VISPLANES
#endif

namespace rend {

//const int kScaleCoef = 8;
const int kScaleCoef = 6;
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

// Context for rendering world::Segments
struct SegmentRendContext {
  // Left and right ends of visible part of the segment
  DPoint p1;
  DPoint p2;

  // Screen coordinates of p1 and p2
  int sx_leftmost;
  int sx_rightmost;

  // Distances from view point to p1 and p2 with angle correction
  double left_distance;
  double right_distance;

  // world::Line::flags of the parent Line for the segment;
  uint32_t line_def_flags;

  // Distance from the left end of the parent Line to p1
  // NB! MUST includes SideDef::texture_offset and invisible 
  // part of the segment!
  int full_offset;
  // Instead of front SideDef::row_offset
  int row_offset;

  // Height of current element in map pixels. It can be height of
  // the wall, or any part of portal (middle, top...) or the height
  // of a masked.
  int pixel_height;

  // Front sector data
  int front_light_level;
  int front_ceiling_height;
  int front_floor_height;

  std::string front_ceiling_pic;
  std::string front_floor_pic;

  // Texture name for walls and middle part of portals
  // Also used for drawing Masked
  std::string mid_texture;

  // Lenght of segment (p1, p2). For speed up only.
  double segment_len;

  // Precalculated coefs for walls, middle part of portals and Masked
  int mid_y_top;
  int mid_y_bottom;
  double mid_dy_top;
  double mid_dy_bottom;

  // Vertical shift, used with lower unpegged bottom texture
  // TODO: looks like a candidate to combine with row_offset
  int pixel_texture_y_shift;


  // ! This part makes sense ONLY for portals !
  int back_ceiling_height;
  int back_floor_height;

  // Texture names for top and bottom parts of the portal
  std::string top_texture;
  std::string bottom_texture;

  std::string back_ceiling_pic;

  // Precalculated coefs for bottom part of portals
  int bottom_y_top;
  int bottom_y_bottom;
  double bottom_dy_top;
  double bottom_dy_bottom;
  // Precalculated coefs for top part of portals
  int top_y_top;
  int top_y_bottom;
  double top_dy_top;
  double top_dy_bottom;


  // Current texture (can be empty). Just cache.
  graph::Texture texture;
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


class Renderer {
 public:
  Renderer();

  //void RenderScene(const Scene& scene);
  void RenderScene(const wad::FastBsp* bsp, const graph::GraphicsManager* gm, const mobj::Player* player);

  void SetDrawWindow(sdl2::SdlWindow* wnd) { wnd_ = wnd; }

  void SetLevel(int level) { level_ = level; }

 private:
  void ClearRenderer();

  void RenderWalls();
  void RenderFlats();
  void RenderMasked();
  void RenderStatusBar(const mobj::Player* player);
  void RenderWeapon(const wpn::Weapon& w);

  void DrawTextureWithShifts(const graph::Texture& texture, int extra_x_shift, int extra_y_shift);
  // Tmp!!!
  void DrawSpriteInCenter(const graph::Texture& texture, int cx, int cy);
  void DrawSpriteTopCenter(const graph::Texture& texture, int cx, int top_y);
  
  // Segment can be partially closed by other elements. This foo creates a list
  // of visible fragments and some extra params. Returns false if all the segment
  // is clipped
  bool CreateUnclippedFragments(const DPoint& left, const DPoint& right);
  void UpdateClipList(int first, int last);
  void CheckOpening(int first, int last);

  void FillSegmentContext(const world::Segment* bsp_segment, const DPoint& left, const DPoint& right);
  void CreateVisplanes();

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
  // Current level. For sky texture only :(
  int level_ = 0;

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
  void DrawSky(const Visplane& vs);

  std::list<MaskedObject> masked_;
  // Clipping track for masked
  // top_clip[i] == -1 means fully closed column
  std::vector<int> top_clip_;
  std::vector<int> bottom_clip_;

  // Returns false if the object is invisible
  bool FillMobjMaskedObject(MaskedObject& msk, const mobj::MapObject* mobj);
  // Fills portal masked using current contex. Portal can be broken into several
  // segments by closer objects. Each segment (left, right) neads its own masked object.
  bool FillPortalMaskedObject(MaskedObject& msk, int left, int right);

  void DrawMaskedObject(const MaskedObject& msk);
  void FillContextFromMasked(const MaskedObject& msk);

  // Idea: Object list contains sprites and portals: [sprites from a sector][portals of a sector] ... [s][p]
  // We have to sort sprites by distance in each sector group, but we must NOT touch the order of groups.
  void SortMaskedObjects();

#ifdef DEBUG_VISPLANES
  void CheckVisplane(const Visplane& vs);
#endif
};

} // namespace rend

#endif  // RENDERER_H_
