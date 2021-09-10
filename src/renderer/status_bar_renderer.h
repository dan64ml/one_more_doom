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

  static void DrawTextureAt(sdl2::SdlWindow* wnd, const graph::Texture& texture, int x_pos, int y_pos);
  // Draw symbol in top left corner. Returns width of the symbol
  static int DrawDigitAt(sdl2::SdlWindow* wnd, const graph::GraphicsManager* gm, const char digit,
                         int x_pos, int y_pos, FontType font, Align align = Align::kTopRight);
  // Draw symbol in top right corner. Returns width of the symbol
  static int DrawDigitAtRight(sdl2::SdlWindow* wnd, const graph::GraphicsManager* gm, const char digit, int x_pos, int y_pos, FontType font);

  static void DrawBaseBar(sdl2::SdlWindow* wnd, const graph::GraphicsManager* gm);
  static void DrawAmmo(sdl2::SdlWindow* wnd, const graph::GraphicsManager* gm, const mobj::Player* player);
};

}

#endif  // STATUS_BAR_RENDERER_H_
