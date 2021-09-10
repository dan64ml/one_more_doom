#ifndef STATUS_BAR_RENDERER_H_
#define STATUS_BAR_RENDERER_H_

#include "graphics/graphics_manager.h"
#include "renderer/renderer.h"

namespace sdl2 {
  class SdlWindow;
}

namespace mobj {
  class Player;
}

namespace rend {

enum class FontType {
  kLargeRed,
  kSmallRed,
  kSmallGray,
  kSmallYellow
};

enum class Align {
  kTopLeft,
  kTopRight
};

class StatusBarRenderer {
 public:
  StatusBarRenderer() = default;

  static void Render(sdl2::SdlWindow* wnd, const mobj::Player* player, const graph::GraphicsManager* gm);

 private:
  static const int kBarHeight = 32 * kScaleCoef;
  static const int kArmsSectionXPos = 104 * kScaleCoef;

  static const int kCurrentAmmoXPos = 44 * kScaleCoef;
  static const int kCurrentAmmoYPos = 171 * kScaleCoef;

  static const int kAvailWeaponXPos = 111 * kScaleCoef;
  static const int kAvailWeaponYPos = 172 * kScaleCoef;
  static const int kAvailWeaponXShift = 12 * kScaleCoef;
  static const int kAvailWeaponYShift = 10 * kScaleCoef;

  static const int kBulletXPos = 288 * kScaleCoef;
  static const int kMaxBulletXShift = 26 * kScaleCoef;
  static const int kBulletYPos1 = 173 * kScaleCoef;
  static const int kBulletYPos2 = 179 * kScaleCoef;
  static const int kBulletYPos3 = 185 * kScaleCoef;
  static const int kBulletYPos4 = 191 * kScaleCoef;

  static const int kHealthXPos = 90 * kScaleCoef;
  static const int kHealthYPos = 171 * kScaleCoef;
  static const int kArmorXPos = 221 * kScaleCoef;
  static const int kArmorYPos = 171 * kScaleCoef;

  static const int kKeysXPos = 239 * kScaleCoef;
  static constexpr int kKeysYPos[3] = {
    171 * kScaleCoef,
    181 * kScaleCoef,
    191 * kScaleCoef
  };

  static const int kBlueKeyYPos = 171 * kScaleCoef;
  static const int kYellowKeyYPos = 181 * kScaleCoef;
  static const int kRedKeyYPos = 191 * kScaleCoef;

  static void DrawTextureAt(sdl2::SdlWindow* wnd, const graph::Texture& texture, int x_pos, int y_pos);
  // Draw a symbol. Returns width of the symbol
  static int DrawDigitAt(sdl2::SdlWindow* wnd, const graph::GraphicsManager* gm, const char digit,
                         int x_pos, int y_pos, FontType font, Align align = Align::kTopRight);
  static void DrawAmmoString(sdl2::SdlWindow* wnd, const graph::GraphicsManager* gm, int current_num,
                              int max_num, int x_pos, int y_pos);

  static void DrawBaseBar(sdl2::SdlWindow* wnd, const graph::GraphicsManager* gm);
  static void DrawAmmo(sdl2::SdlWindow* wnd, const graph::GraphicsManager* gm, const mobj::Player* player);
  static void DrawPlayerArmorHealth(sdl2::SdlWindow* wnd, const graph::GraphicsManager* gm, const mobj::Player* player);
  static void DrawCards(sdl2::SdlWindow* wnd, const graph::GraphicsManager* gm, const mobj::Player* player);
};

}

#endif  // STATUS_BAR_RENDERER_H_
