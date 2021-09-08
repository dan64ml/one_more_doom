#ifndef APP_CLASS_H_
#define APP_CLASS_H_

#include "sdl_base_class.h"

#include "world/world.h"
#include "graphics/graphics_manager.h"
#include "renderer/renderer.h"
#include "objects/player.h"

struct Keys {
  bool up = false;
  bool down = false;
  bool left = false;
  bool right = false;
  bool run = false;
  bool side_move = false;
  bool fire = false;

  char number_key = 0;
};

class AppClass : public sdl2::SdlBaseClass {
 public:
  AppClass();

  virtual void InitInstance() override;
  virtual void ProcessScene(int ms_elapsed) override;
  virtual void RenderScene() override;

 private:
  sdl2::SdlWindow wnd_ {"One More Doom", rend::kScreenXResolution, rend::kScreenYResolution};
  //sdl2::SdlWindow wnd_ {"One More Doom", 1024, 768};

  world::World world_;
  graph::GraphicsManager gm_;
  rend::Renderer renderer_;

  Keys keybd_;

  mobj::Player* player_;

  // Speeds, units per frame (35Hz)
  const uint16_t kAngleMove[2] {640, 1280};
//  const uint16_t kAngleMove[2] {64, 1280};
  const double kForwardMove[2] {8.33, 16.67};
  const double kSideMove[2] {8.0, 13.34};
};

#endif  // APP_CLASS_H_
