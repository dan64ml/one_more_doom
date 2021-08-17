#include "app_class.h"

#include <cassert>
#include <iostream>

using namespace std;

AppClass::AppClass() {
  //wnd_.;
}

void AppClass::InitInstance() {
  //map_.Open("/home/dan/tmp/DOOM2.WAD");
  //map_.Open("/usr/share/games/doom/DOOM2.WAD");
  //map_.Open("/usr/share/games/doom/freedoom1.wad");
  
  gm_.Load("/home/dan/tmp/DOOM2.WAD");

  world_.OpenWad("/home/dan/tmp/DOOM2.WAD");
  world_.LoadLevel(10); // 11 - wrong torch // 16 flying torch //27
  player_ = world_.GetPlayer();

  renderer_.SetDrawWindow(&wnd_);

  // 17 - right texture alignment error; 4 - floor has wrong direction
  // 25 and angle - 45: switch is shifted
  // 26 - 45 - switch shifted again
  // 19 - sprites after closed portal
  // 20 - lots of items
  // !!! 11 - ceiling (?sky) flats are strange !!!
  
  //map_.LoadLevel(25); //16 // 27 - sprites // flats 12 // 25, 2, 4, 13 /* textures 8, 11 */


 	SetKeyStateHandler(SDL_SCANCODE_RIGHT/*SDL_SCANCODE_D*/, [this]() {
      this->keybd_.right = true;
    }
	);

 	SetKeyStateHandler(SDL_SCANCODE_LEFT/*SDL_SCANCODE_A*/, [this]() {
      this->keybd_.left = true;
    }
	);

 	SetKeyStateHandler(SDL_SCANCODE_UP/*SDL_SCANCODE_W*/, [this]() {
      this->keybd_.up = true;
    }
	);

 	SetKeyStateHandler(SDL_SCANCODE_DOWN/*SDL_SCANCODE_S*/, [this]() {
      this->keybd_.down = true;
    }
	);

 	SetKeyStateHandler(SDL_SCANCODE_LSHIFT, [this]() {
      this->keybd_.run = true;
    }
	);

 	SetKeyStateHandler(SDL_SCANCODE_LCTRL/*SDL_SCANCODE_LALT*/, [this]() {
      this->keybd_.side_move = true;
    }
	);

 	SetKeyStateHandler(SDL_SCANCODE_SPACE, [this]() {
      this->keybd_.fire = true;
    }
	);
}

void AppClass::ProcessScene([[maybe_unused]] int ms_elapsed) {
  int speed_idx = keybd_.run ? 1 : 0;

  int delta_angle = 0;
  double forward_move = 0;
  double side_move = 0;

  if (keybd_.left) {
    if (keybd_.side_move) {
      side_move -= kSideMove[speed_idx];
    } else {
      delta_angle += kAngleMove[speed_idx];
    }
    keybd_.left = false;
  }
  if (keybd_.right) {
    if (keybd_.side_move) {
      side_move += kSideMove[speed_idx];
    } else {
      delta_angle -= kAngleMove[speed_idx];
    }
    keybd_.right = false;
  }

  if (keybd_.up) {
    forward_move += kForwardMove[speed_idx];
    keybd_.up = false;
  }
  if (keybd_.down) {
    forward_move -= kForwardMove[speed_idx];
    keybd_.down = false;
  }

  keybd_.run = false;
  keybd_.side_move = false;

  player_->Move(delta_angle, forward_move, side_move);

  world_.TickTime();
}

void AppClass::RenderScene() {
  renderer_.RenderScene(world_.GetBsp(), &gm_, world_.GetPlayer());
}