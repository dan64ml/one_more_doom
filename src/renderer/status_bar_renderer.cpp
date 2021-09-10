#include "status_bar_renderer.h"

#include "app/sdl_window.h"
#include "objects/player.h"
//#include "graphics/graphics_manager.h"
#include "graphics/texture.h"
#include "renderer/renderer.h"

namespace rend {

void StatusBarRenderer::Render(sdl2::SdlWindow* wnd, const mobj::Player* player, const graph::GraphicsManager* gm) {
  DrawBaseBar(wnd, gm);
  DrawAmmo(wnd, gm, player);

  //auto test = gm->GetSprite("AMMNUM0");
  auto test = gm->GetSTBarElement("STCFN048");
  DrawTextureAt(wnd, test, 300, 300);
}

void StatusBarRenderer::DrawAmmo(sdl2::SdlWindow* wnd, const graph::GraphicsManager* gm, const mobj::Player* player) {
  const auto weapon = player->GetWeapon();

  // Current ammo
  int num = weapon.GetCurrentAmmoNumber();
  num = std::min(num, 999);

  int digit_x_pos = kCurrentAmmoXPos;
  do {
    char digit = '0' + num % 10;
    num /= 10;

    int width = DrawDigitAtRight(wnd, gm, digit, digit_x_pos, kCurrentAmmoYPos, FontType::kLargeRed);
    digit_x_pos -= width;
  } while (num);

  // Available weapon
  DrawDigitAt(wnd, gm, '2', 111 * kScaleCoef, 172 * kScaleCoef, FontType::kSmallGray, Align::kTopLeft);
}

void StatusBarRenderer::DrawBaseBar(sdl2::SdlWindow* wnd, const graph::GraphicsManager* gm) {
  auto bar = gm->GetSprite("STBAR");
  DrawTextureAt(wnd, bar, 0, kScreenYResolution - kBarHeight);

  auto arm_texture = gm->GetSTBarElement("STARMS");
  DrawTextureAt(wnd, arm_texture, kArmsSectionXPos, kScreenYResolution - kBarHeight);
}

void StatusBarRenderer::DrawTextureAt(sdl2::SdlWindow* wnd, const graph::Texture& texture, int x_pos, int y_pos) {
  int x_size = texture.GetXSize();
  int y_size = texture.GetYSize();

  int sx_from = x_pos;
  int sx_to = sx_from + x_size * kScaleCoef;
  int sx_offset = sx_from; // -
  sx_from = std::max(0, sx_from);
  sx_to = std::min(kScreenXResolution, sx_to);

  int sy_from = (kOriginYResolution - y_size) * kScaleCoef - y_pos;
  int sy_to = kOriginYResolution * kScaleCoef - y_pos;
  int sy_offset = sy_from;  // -
  sy_from = std::max(0, sy_from);
  sy_to = std::min(kScreenYResolution, sy_to);

  for (int x = sx_from; x < sx_to; ++x) {
    for (int y = sy_from; y < sy_to; ++y) {
      uint32_t color = texture.GetPixel((x - sx_offset) / kScaleCoef, y_size - 1 - (y - sy_offset) / kScaleCoef);
      wnd->RenderFBPointAlpha(x, y, color);
    }
  }
}

int StatusBarRenderer::DrawDigitAt(sdl2::SdlWindow* wnd, const graph::GraphicsManager* gm, const char digit,
                                   int x_pos, int y_pos, FontType font, Align align) {
  if (digit < '0' || digit > '9') {
    return 0;
  }

  std::string texture_name;
  switch (font)
  {
  case FontType::kLargeRed:
    texture_name = "STTNUM";
    break;
  case FontType::kSmallGray:
    texture_name = "STGNUM";
    break;
  case FontType::kSmallYellow:
    texture_name = "STYSNUM";
    break;
  case FontType::kSmallRed:
    texture_name = "STCFN0";
    break;
  
  default:
    break;
  }

  if (font == FontType::kSmallRed) {
    texture_name += std::to_string(static_cast<int>(digit));
  } else {
    texture_name += digit;
  }
  const auto texture = gm->GetSTBarElement(texture_name);

  int x_size = texture.GetXSize() * kScaleCoef;
  if (align == Align::kTopRight) {
    x_pos -= x_size;
  }
  DrawTextureAt(wnd, texture, x_pos, y_pos);

  return x_size;
}

int StatusBarRenderer::DrawDigitAtRight(sdl2::SdlWindow* wnd, const graph::GraphicsManager* gm, const char digit, int x_pos, int y_pos, FontType font) {
  if (digit < '0' || digit > '9') {
    return 0;
  }

  std::string texture_name = std::string("STTNUM") + digit;
  const auto texture = gm->GetSTBarElement(texture_name);

  int x_size = texture.GetXSize() * kScaleCoef;
  DrawTextureAt(wnd, texture, x_pos - x_size, y_pos);

  return x_size;
}

} // namespace rend
