/*
 * draw_window.cpp
 *
 *  Created on: Oct 1, 2020
 *      Author: dan
 */

#include "sdl_window.h"

#include <stdexcept>

namespace sdl2 {

SdlWindow::SdlWindow(std::string caption, int x_size, int y_size, int x_pos, int y_pos) 
  : x_size_(x_size), y_size_(y_size) {
  wnd_ = SDL_CreateWindow(caption.c_str(), x_pos, y_pos, x_size_, y_size_, wnd_flags_);
  if (wnd_ == nullptr) {
    clear();
    throw std::runtime_error(std::string("SDL_CreateWindow: ") + SDL_GetError());
  }

  render_ = SDL_CreateRenderer(wnd_, -1, render_flags_);
  if (render_ == nullptr) {
    clear();
    throw std::runtime_error(std::string("SDL_CreateRenderer: ") + SDL_GetError());
  }

  window_id_ = SDL_GetWindowID(wnd_);

  font_ = TTF_OpenFont("/usr/share/fonts/truetype/ubuntu/UbuntuMono-RI.ttf", 28);
  if (font_ == nullptr) {
    clear();
    throw std::runtime_error(std::string("TTF_OpenFont: ") + TTF_GetError());
  }

  frame_buf_.resize(x_size_ * y_size_);
	sdl_surface_ = SDL_CreateRGBSurfaceFrom(frame_buf_.data(), x_size_, y_size_, 32, 4 * x_size_,
			0xff, 0xff00, 0xff0000, 0xff000000);
	if (sdl_surface_ == nullptr) {
		clear();
		throw std::runtime_error(std::string("SDL_CreateRGBSurfaceFrom(): ") + SDL_GetError());
	}
}

SdlWindow::~SdlWindow() {
  clear();
}

int SdlWindow::GetXSize() const {
  int size;
  SDL_GetWindowSize(wnd_, &size, nullptr);
  return size;
}

int SdlWindow::SdlWindow::GetYSize() const {
  int size;
  SDL_GetWindowSize(wnd_, nullptr, &size);
  return size;
}

void SdlWindow::ClearFBRender(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
  uint32_t clear_color = r;
  clear_color |= g << 8;
  clear_color |= b << 16;
  clear_color |= a << 24;

  std::fill(begin(frame_buf_), end(frame_buf_), clear_color);
}

void SdlWindow::SetPosition(int x, int y) {
  SDL_SetWindowPosition(wnd_, x, y);
}

void SdlWindow::SetSize(int w, int h) {
  SDL_SetWindowSize(wnd_, w, h);
  x_size_ = w;
  y_size_ = h;
}

void SdlWindow::SetDrawColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
  SDL_SetRenderDrawColor(render_, r, g, b, a);
}

void SdlWindow::DrawPoint(int x, int y) {
  RenderPoint(x, y);
  SDL_RenderPresent(render_);
}

void SdlWindow::DrawLine(int x1, int y1, int x2, int y2) {
  RenderLine(x1, y1, x2, y2);
  SDL_RenderPresent(render_);
}

void SdlWindow::ClearRender(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
  SDL_SetRenderDrawColor(render_, r, g, b, a);
  SDL_RenderClear(render_);
}

void SdlWindow::RenderPoint(int x, int y) {
  SDL_RenderDrawPoint(render_, x + origin_x_shift_, y_direction_ * y + origin_y_shift_);
}

void SdlWindow::RenderFBPoint(int x, int y, uint8_t r, uint8_t g, uint8_t b) {
  uint32_t pixel = 0xFF000000;
  pixel |= r;
  pixel |= g << 8;
  pixel |= b << 16;

  int idx = (y_size_ - (y + 1)) * x_size_ + x; 
  frame_buf_.data()[idx] = pixel;
}

void SdlWindow::ShowFBRender() {
	  SDL_Texture* mTexture = SDL_CreateTextureFromSurface(render_, sdl_surface_);

	  SDL_RenderCopy(render_, mTexture, NULL, NULL);
	  SDL_RenderPresent(render_);

	  SDL_DestroyTexture(mTexture);
}

void SdlWindow::RenderLine(int x1, int y1, int x2, int y2) {
  SDL_RenderDrawLine(render_, x1 + origin_x_shift_, y_direction_ * y1 + origin_y_shift_, 
                      x2 + origin_x_shift_, y_direction_ * y2 + origin_y_shift_);
}

void SdlWindow::ShowRender() {
  SDL_RenderPresent(render_);
}

void SdlWindow::clear() {
  if (sdl_surface_) {
    SDL_FreeSurface(sdl_surface_);
  }
  if (font_) {
    TTF_CloseFont(font_);
  }
  if (render_) {
    SDL_DestroyRenderer(render_);
  }
  if (wnd_) {
    SDL_DestroyWindow(wnd_);
  }
}

void SdlWindow::OnWindowEvent(const SDL_Event& event) {
  if (event.window.windowID != window_id_) {
    return;
  }

  switch (event.window.type) {
  case SDL_WINDOWEVENT_SIZE_CHANGED:
    break;
  }
}

void SdlWindow::SetTitle(std::string title) {
  SDL_SetWindowTitle(wnd_, title.c_str());
}

void SdlWindow::RenderFillRect(int x1, int y1, int x2, int y2) {
  SDL_Rect rect {x1, y1, x2, y2};
  SDL_RenderFillRect(render_, &rect);
}

void SdlWindow::RenderDoublePoint(int x, int y) {
  RenderPoint(2*x, 2*y);
  RenderPoint(2*x+1, 2*y);
  RenderPoint(2*x, 2*y+1);
  RenderPoint(2*x+1, 2*y+1);
}

void SdlWindow::RenderTriplePoint(int x, int y) {
  for (int i = 0; i < 3; ++i) {
    for (int j = 0; j < 3; ++j) {
      RenderPoint(3*x + i, 3*y + j);
    }
  }
}

void SdlWindow::RenderQuadPoint(int x, int y) {
  for (int i = 0; i < 4; ++i) {
    for (int j = 0; j < 4; ++j) {
      RenderPoint(4*x + i, 4*y + j);
    }
  }
}

void SdlWindow::PrintString(const std::string& str) {
  SDL_Surface* text_surface = TTF_RenderText_Solid(font_, str.c_str(), {255, 255, 255, 0});
  SDL_BlitSurface(text_surface, NULL, sdl_surface_, NULL);
  SDL_FreeSurface(text_surface);
}

void SdlWindow::SaveScreenshot(const std::string& file_name) {
  SDL_Surface* sshot = SDL_CreateRGBSurface(0, x_size_, y_size_, 32,
                                            0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000);
  SDL_RenderReadPixels(render_, NULL, SDL_PIXELFORMAT_ARGB8888, sshot->pixels, sshot->pitch);
  SDL_SaveBMP(sshot, file_name.c_str());
  SDL_FreeSurface(sshot);
}

}   // namespace sdl2

