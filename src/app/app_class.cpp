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
  
  gm_.Load("/home/dan/tmp/DOOM2.WAD");

  world_.OpenWad("/home/dan/tmp/DOOM2.WAD");
  world_.LoadLevel(17);

  renderer_.SetDrawWindow(&wnd_);

  // 17 - right texture alignment error; 4 - floor has wrong direction
  // 25 and angle - 45: switch is shifted
  // 26 - 45 - switch shifted again
  // 19 - sprites after closed portal
  // 20 - lots of items
  // !!! 11 - ceiling (?sky) flats are strange !!!
  
  //map_.LoadLevel(25); //16 // 27 - sprites // flats 12 // 25, 2, 4, 13 /* textures 8, 11 */


//  map_.Open("/usr/share/games/doom/freedoom1.wad");
//  map_.LoadLevel(0);  // 4

//  render_.SetWadMap(&map_);
//  render_.SetDrawWindow(&wnd_doom_);

// 	SetKeyStateHandler(SDL_SCANCODE_D, [this]() {
//    this->keybd_.right = true;
//		this->scene_.vp_angle -= 300; }
//	);
// 	SetKeyStateHandler(SDL_SCANCODE_A, [this]() {
//    this->keybd_.left = true;
//		this->scene_.vp_angle += 300; }
//	);
// 	SetKeyStateHandler(SDL_SCANCODE_W, [this]() {
//    this->keybd_.up = true;
//
//		double step = 8.33;
//    int dx = step * rend::BamCos(this->scene_.vp_angle);
//    int dy = step * rend::BamSin(this->scene_.vp_angle);
//    this->scene_.vp_x += dx;
//    this->scene_.vp_y += dy; 
//
////    this->player_.mom_x = dx;
////    this->player_.mom_y = dy;
////    this->player_.angle = this->scene_.vp_angle;
//   }
//	);
// 	SetKeyStateHandler(SDL_SCANCODE_S, [this]() {
//    this->keybd_.down = true;
//   }
//	);
}

void AppClass::ProcessScene([[maybe_unused]] int ms_elapsed) {
//    if (keybd_.left) {
//      player_.angle += kAngleSpeed;
//      keybd_.left = false;
//    }
//    if (keybd_.right) {
//      player_.angle -= kAngleSpeed;
//      keybd_.right = false;
//    }
//
// 		double step = 8.33;
//    if (keybd_.up) {
//      int dx = step * rend::BamCos(player_.angle);
//      int dy = step * rend::BamSin(player_.angle);
//      
//      player_.mom_x = dx;
//      player_.mom_y = dy;
//
//      keybd_.up = false;
//    }
//    if (keybd_.down) {
//      int dx = step * rend::BamCos(player_.angle);
//      int dy = step * rend::BamSin(player_.angle);
//      
//      player_.mom_x = -dx;
//      player_.mom_y = -dy;
//
//      keybd_.down = false;
//    }
//
//    player_.TimeTick();
  world_.TickTime();
}

void AppClass::RenderScene() {
  renderer_.RenderScene(world_.GetBsp(), &gm_, world_.GetPlayer());
}