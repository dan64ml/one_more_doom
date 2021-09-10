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
  DrawPlayerArmorHealth(wnd, gm, player);

  //auto test = gm->GetSprite("AMMNUM0");
  auto test = gm->GetSTBarElement("STFB2");
  DrawTextureAt(wnd, test, 300, 300);
}

void StatusBarRenderer::DrawPlayerArmorHealth(sdl2::SdlWindow* wnd, const graph::GraphicsManager* gm, const mobj::Player* player) {
  auto percent = gm->GetSTBarElement("STTPRCNT");
  DrawTextureAt(wnd, percent, kHealthXPos, kHealthYPos);
  DrawTextureAt(wnd, percent, kArmorXPos, kArmorYPos);

  int health = player->GetHealth();
  int digit_x_pos = kHealthXPos;
  do {
    char digit = '0' + health % 10;
    health /= 10;

    int width = DrawDigitAt(wnd, gm, digit, digit_x_pos, kHealthYPos, FontType::kLargeRed);
    digit_x_pos -= width;
  } while (health);

  int armor = player->GetArmor();
  digit_x_pos = kArmorXPos;
  do {
    char digit = '0' + armor % 10;
    armor /= 10;

    int width = DrawDigitAt(wnd, gm, digit, digit_x_pos, kHealthYPos, FontType::kLargeRed);
    digit_x_pos -= width;
  } while (armor);
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

    int width = DrawDigitAt(wnd, gm, digit, digit_x_pos, kCurrentAmmoYPos, FontType::kLargeRed);
    digit_x_pos -= width;
  } while (num);

  // Available weapon
  FontType font = weapon.IsWeaponAvailable(2) ? FontType::kSmallRed : FontType::kSmallGray;
  DrawDigitAt(wnd, gm, '2', kAvailWeaponXPos, kAvailWeaponYPos, font, Align::kTopLeft);

  font = weapon.IsWeaponAvailable(3) ? FontType::kSmallRed : FontType::kSmallGray;
  DrawDigitAt(wnd, gm, '3', kAvailWeaponXPos + kAvailWeaponXShift, kAvailWeaponYPos, font, Align::kTopLeft);

  font = weapon.IsWeaponAvailable(4) ? FontType::kSmallRed : FontType::kSmallGray;
  DrawDigitAt(wnd, gm, '4', kAvailWeaponXPos + 2 * kAvailWeaponXShift, kAvailWeaponYPos, font, Align::kTopLeft);

  font = weapon.IsWeaponAvailable(5) ? FontType::kSmallRed : FontType::kSmallGray;
  DrawDigitAt(wnd, gm, '5', kAvailWeaponXPos, kAvailWeaponYPos + kAvailWeaponYShift, font, Align::kTopLeft);

  font = weapon.IsWeaponAvailable(6) ? FontType::kSmallRed : FontType::kSmallGray;
  DrawDigitAt(wnd, gm, '6', kAvailWeaponXPos + kAvailWeaponXShift, kAvailWeaponYPos + kAvailWeaponYShift, font, Align::kTopLeft);

  font = weapon.IsWeaponAvailable(7) ? FontType::kSmallRed : FontType::kSmallGray;
  DrawDigitAt(wnd, gm, '7', kAvailWeaponXPos + 2 * kAvailWeaponXShift, kAvailWeaponYPos + kAvailWeaponYShift, font, Align::kTopLeft);

  // Ammunition
  auto bullet = weapon.GetAmmo(wpn::kAmClip);
  DrawAmmoString(wnd, gm, bullet.first, bullet.second, kBulletXPos, kBulletYPos1);
  auto shell = weapon.GetAmmo(wpn::kAmShell);
  DrawAmmoString(wnd, gm, shell.first, shell.second, kBulletXPos, kBulletYPos2);
  auto rocket = weapon.GetAmmo(wpn::kAmMisl);
  DrawAmmoString(wnd, gm, rocket.first, rocket.second, kBulletXPos, kBulletYPos3);
  auto cell = weapon.GetAmmo(wpn::kAmCell);
  DrawAmmoString(wnd, gm, cell.first, cell.second, kBulletXPos, kBulletYPos4);
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
    //texture_name = "STCFN0";
    texture_name = "STGNUM";
    break;
  
  default:
    break;
  }

//  if (font == FontType::kSmallRed) {
//    texture_name += std::to_string(static_cast<int>(digit));
//  } else {
//    texture_name += digit;
//  }
  texture_name += digit;
  auto texture = gm->GetSTBarElement(texture_name);

  if (font == FontType::kSmallRed) {
    texture.SetPalette(4);
    texture.SetLightLevel(0);
  }

  int x_size = texture.GetXSize() * kScaleCoef;
  if (align == Align::kTopRight) {
    x_pos -= x_size;
  }
  DrawTextureAt(wnd, texture, x_pos, y_pos);

  return x_size;
}

void StatusBarRenderer::DrawAmmoString(sdl2::SdlWindow* wnd, const graph::GraphicsManager* gm, int current_num,
                                       int max_num, int x_pos, int y_pos) {
  int digit_x_pos = x_pos;
  do {
    char digit = '0' + current_num % 10;
    current_num /= 10;

    int width = DrawDigitAt(wnd, gm, digit, digit_x_pos, y_pos, FontType::kSmallYellow);
    digit_x_pos -= width;
  } while (current_num);

  digit_x_pos = x_pos + kMaxBulletXShift;
  do {
    char digit = '0' + max_num % 10;
    max_num /= 10;

    int width = DrawDigitAt(wnd, gm, digit, digit_x_pos, y_pos, FontType::kSmallYellow);
    digit_x_pos -= width;
  } while (max_num);
}

} // namespace rend
