#ifndef APP_CLASS_H_
#define APP_CLASS_H_

#include "sdl_base_class.h"

#include "world/world.h"
#include "graphics/graphics_manager.h"

struct Keys {
  bool up = false;
  bool down = false;
  bool left = false;
  bool right = false;
};

class AppClass : public sdl2::SdlBaseClass {
 public:
  AppClass();

  virtual void InitInstance() override;
  virtual void ProcessScene(int ms_elapsed) override;
  virtual void RenderScene() override;

 private:
  //sdl2::SdlWindow wnd_ {"One More Doom", rend::kScreenXResolution, rend::kScreenYResolution};
  sdl2::SdlWindow wnd_ {"One More Doom", 1024, 768};

  world::World world_;
  graph::GraphicsManager gm_;
};

#endif  // APP_CLASS_H_
