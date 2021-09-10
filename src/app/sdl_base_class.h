/*
 * sdl_base_class.h
 *
 *  Created on: Oct 1, 2020
 *      Author: dan
 */

#ifndef SDL_BASE_CLASS_H_
#define SDL_BASE_CLASS_H_

#include "sdl_window.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <string>
#include <vector>
#include <functional>
#include <unordered_map>

namespace sdl2 {

class SdlBaseClass {
 public:
  SdlBaseClass();
  virtual ~SdlBaseClass();

  void RunFixedStepLoop(int ms_step);
  void RunFastLoop();

  void SetKeyUpHandler(SDL_Keycode key, std::function<void(const SDL_Event&)> handler);
  void SetKeyStateHandler(SDL_Scancode key, std::function<void(void)> handler);

  bool IsButtonPressed(SDL_Scancode key) const;

 protected:
  SdlWindow* CreateWindow(std::string caption, int x_size = 800, int y_size = 600,
	 		                      int x_pos = SDL_WINDOWPOS_CENTERED, int y_pos = SDL_WINDOWPOS_CENTERED);

  virtual void InitInstance() {}
  virtual void ClearInstance() {}

  virtual void ProcessScene(int ms_elapsed) = 0;

  virtual void RenderScene() {}

  virtual void OnKeyboardUpEvent(const SDL_Event& event);
  virtual void OnKeyboardEvent([[maybe_unused]] const SDL_Event& event) {}
  virtual void OnMouseMotionEvent([[maybe_unused]] const SDL_Event& event) {}
  virtual void OnMouseButtonEvent([[maybe_unused]] const SDL_Event& event) {}
  virtual void OnMouseWheelEvent([[maybe_unused]] const SDL_Event& event) {}
  virtual void OnQuitEvent(const SDL_Event& event);

protected:
  std::vector<SdlWindow*> windows_;
  std::unordered_map<SDL_Keycode, std::function<void(const SDL_Event&)>> key_up_handlers_;
  std::unordered_map<SDL_Keycode, std::function<void(void)>> key_state_handlers_;

private:
  void ProcessEvents();

private:
  bool b_looping_ = true;
};

}   // namespace sdl2

#endif /* SDL_BASE_CLASS_H_ */
