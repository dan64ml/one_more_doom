/*
 * draw_window.h
 *
 *  Created on: Oct 1, 2020
 *      Author: dan
 */

#ifndef SDL_WINDOW_H_
#define SDL_WINDOW_H_

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <string>
#include <vector>

namespace sdl2 {

class SdlWindow {
public:
  SdlWindow(std::string caption, int x_size, int y_size,
      int x_pos = SDL_WINDOWPOS_CENTERED, int y_pos = SDL_WINDOWPOS_CENTERED);
  virtual ~SdlWindow();

  void SetPosition(int x, int y);
  void SetSize(int w, int h);
  void SetTitle(std::string title);

  void OnWindowEvent(const SDL_Event& event);

  int GetXSize() const;
  int GetYSize() const;

  // Fast pixel operations
  // Draw point at {x, y}. Origin is fixed in the lower left corner.
  void RenderFBPoint(int x, int y, uint8_t r, uint8_t g, uint8_t b);
  void RenderFBPointAlpha(int x, int y, uint32_t c);
  void PrintString(const std::string& str);
  void ShowFBRender();
  void ClearFBRender(uint8_t r, uint8_t g, uint8_t b, uint8_t a);


  // Slow operations...
  // Рисуют примитив на экране
  void DrawPoint(int x, int y);
  void DrawLine(int x1, int y1, int x2, int y2);

  // Рисуют в рендерере. Требуют ShowRender() для вывода результата на экран
  void ClearRender(uint8_t r, uint8_t g, uint8_t b, uint8_t a);
  void RenderPoint(int x, int y);
  void RenderLine(int x1, int y1, int x2, int y2);
  void RenderFillRect(int x1, int y1, int x2, int y2);

  void RenderDoublePoint(int x, int y);
  void RenderTriplePoint(int x, int y);
  void RenderQuadPoint(int x, int y);

  void ShowRender();

  void SetDrawColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a);

  // Point {0, 0} moves to {dx, dy}
  void MoveOrigin(int dx, int dy) { origin_x_shift_ = dx; origin_y_shift_ = dy; }
  void SetYUp(bool up) { y_direction_ = up ? -1 : 1; }

private:
  void clear();

private:
  SDL_Window* wnd_ = nullptr;
  SDL_Renderer* render_ = nullptr;
  uint32_t window_id_;

  uint32_t wnd_flags_ = SDL_WINDOW_SHOWN;
  uint32_t render_flags_ = SDL_RENDERER_ACCELERATED;

  TTF_Font* font_ = nullptr;

  int origin_x_shift_ = 0;
  int origin_y_shift_ = 0;
  int y_direction_ = 1;

  // Window size
  int x_size_;
  int y_size_;
  std::vector<uint32_t> frame_buf_;
  SDL_Surface* sdl_surface_ = nullptr;
};

inline
void SdlWindow::RenderFBPointAlpha(int x, int y, uint32_t c) {
  if (!(0xFF000000 & c)) {
    return;
  }

  int idx = (y_size_ - (y + 1)) * x_size_ + x; 
  frame_buf_.data()[idx] = c;
}

}   // namespace sdl2
 
#endif /* SDL_WINDOW_H_ */
