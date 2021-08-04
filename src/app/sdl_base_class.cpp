/*
 * sdl_base_class.cpp
 *
 *  Created on: Oct 1, 2020
 *      Author: dan
 */

#include "sdl_base_class.h"

#include <stdexcept>
#include <chrono>

using namespace std;

namespace sdl2 {

SdlBaseClass::SdlBaseClass() {
  if (SDL_Init(SDL_INIT_VIDEO) != 0) {
    throw runtime_error(string("SdlBaseClass::SDL_Init error: ") + SDL_GetError());
  }
  if (TTF_Init() != 0) {
    throw runtime_error(string("SdlBaseClass::TTF_Init error: ") + TTF_GetError());
  }
}

SdlBaseClass::~SdlBaseClass() {
   for (auto wnd : windows_) {
     delete wnd;
   }

  TTF_Quit();
  SDL_Quit();
}

void SdlBaseClass::RunFastLoop() {
  InitInstance();

  auto last_time = chrono::steady_clock::now();

  while (b_looping_) {
    auto current_time = chrono::steady_clock::now();
    auto delta = chrono::duration_cast<chrono::milliseconds>(current_time - last_time);

    ProcessEvents();
    ProcessScene(delta.count());
    RenderScene();

    last_time = current_time;
  }

  ClearInstance();
}

void SdlBaseClass::RunFixedStepLoop(int ms_step) {
  using namespace std::chrono;

  InitInstance();

  auto previous = steady_clock::now();
  milliseconds lag {0};
  milliseconds step {ms_step};
  while (b_looping_) {
    auto current = steady_clock::now();
    auto elapsed = current - previous;
    previous = current;
    lag += duration_cast<milliseconds>(elapsed);

    ProcessEvents();
    
    while (lag >= step) {
      ProcessScene(ms_step);
      lag -= step;
    }

    RenderScene();
  }

  ClearInstance();
}

void SdlBaseClass::ProcessEvents() {
  SDL_Event event;

  while (SDL_PollEvent(&event)) {
    switch (event.type) {
      case SDL_QUIT:
        OnQuitEvent(event);
        return;
      case SDL_WINDOWEVENT:
        if (event.window.event == SDL_WINDOWEVENT_CLOSE) {
          SDL_Event quitEvent;
          quitEvent.type = SDL_QUIT;
          SDL_PushEvent(&quitEvent);
          return;
        } else {
          for (auto wnd : windows_) {
            wnd->OnWindowEvent(event);
          }
        }
        break;
      case SDL_MOUSEMOTION:
        OnMouseMotionEvent(event);
        break;
      case SDL_MOUSEWHEEL:
        OnMouseWheelEvent(event);
        break;
      case SDL_MOUSEBUTTONDOWN:
      case SDL_MOUSEBUTTONUP:
        OnMouseButtonEvent(event);
        break;
      case SDL_KEYUP:
        OnKeyboardUpEvent(event);
        break;
      case SDL_KEYDOWN:
        OnKeyboardEvent(event);
        break;
    }
  }

  const Uint8* key_states = SDL_GetKeyboardState(NULL);
  for (const auto& [key, handler] : key_state_handlers_) {
    if (key_states[key]) {
      handler();
    }
  }
}

void SdlBaseClass::OnKeyboardUpEvent(const SDL_Event& event) {
  if (key_up_handlers_.count(event.key.keysym.sym)) {
    key_up_handlers_[event.key.keysym.sym](event);
  }
}

void SdlBaseClass::SetKeyUpHandler(SDL_Keycode key, std::function<void(const SDL_Event&)> handler) {
  if (handler == nullptr && key_up_handlers_.count(key)) {
    key_up_handlers_.erase(key);
  } else {
    key_up_handlers_[key] = handler;
  }
}

void SdlBaseClass::SetKeyStateHandler(SDL_Scancode key, std::function<void(void)> handler) {
  if (handler == nullptr && key_state_handlers_.count(key)) {
    key_state_handlers_.erase(key);
  } else {
    key_state_handlers_[key] = handler;
  }
}

void SdlBaseClass::OnQuitEvent([[maybe_unused]] const SDL_Event& event) {
  b_looping_ = false;
}

SdlWindow* SdlBaseClass::CreateWindow(std::string caption, int x_size, int y_size, int x_pos, int y_pos) {
  SdlWindow* wnd = new SdlWindow(caption, x_size, y_size, x_pos, y_pos);
  windows_.push_back(wnd);
  return wnd;
}

}   // namespace sdl2

