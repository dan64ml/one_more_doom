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
//  gm_.Load("/home/dan/tmp/Wads/Doom1.WAD");

  int level = 27; //12; // 19 - flying firesticks 
                  // 6 - !!! //21; // 12 - wide open space // 29 - issue with clipping mobj (BFG9000)
                  // 23, 24 - animated floor and wall
                  // 29 - scrolling wall
                  // 22 - lots of barrels
                  // 17 - good candidate for demo, lots of sobj etc... 

  world_.OpenWad("/home/dan/tmp/DOOM2.WAD");
  world_.LoadLevel(level); // 10 - wrong portal // 11 - wrong torch // 16 flying torch // 29 animated wall
  player_ = world_.GetPlayer();

  renderer_.SetDrawWindow(&wnd_);
  renderer_.SetLevel(level);

  // 17 - right texture alignment error; 4 - floor has wrong direction
  // 25 and angle - 45: switch is shifted
  // 26 - 45 - switch shifted again
  // 19 - sprites after closed portal
  // 20 - lots of items
  // !!! 11 - ceiling (?sky) flats are strange !!!
  
  //map_.LoadLevel(25); //16 // 27 - sprites // flats 12 // 25, 2, 4, 13 /* textures 8, 11 */


 	SetKeyStateHandler(SDL_SCANCODE_RIGHT, [this]() {
      this->keybd_.right = true;
    }
	);

 	SetKeyStateHandler(SDL_SCANCODE_LEFT, [this]() {
      this->keybd_.left = true;
    }
	);

 	SetKeyStateHandler(SDL_SCANCODE_UP, [this]() {
      this->keybd_.up = true;
    }
	);

 	SetKeyStateHandler(SDL_SCANCODE_DOWN, [this]() {
      this->keybd_.down = true;
    }
	);

 	SetKeyStateHandler(SDL_SCANCODE_LSHIFT, [this]() {
      this->keybd_.run = true;
    }
	);

 	SetKeyStateHandler(SDL_SCANCODE_LALT, [this]() {
      this->keybd_.side_move = true;
    }
	);

  // Change the weapon
  SetKeyUpHandler(SDLK_1, [this](const SDL_Event&) {
      this->keybd_.number_key = '1';
    }
	); 
  SetKeyUpHandler(SDLK_2, [this](const SDL_Event&) {
      this->keybd_.number_key = '2';
    }
	); 
  SetKeyUpHandler(SDLK_3, [this](const SDL_Event&) {
      this->keybd_.number_key = '3';
    }
	); 
  SetKeyUpHandler(SDLK_4, [this](const SDL_Event&) {
      this->keybd_.number_key = '4';
    }
	); 
  SetKeyUpHandler(SDLK_5, [this](const SDL_Event&) {
      this->keybd_.number_key = '5';
    }
	); 
  SetKeyUpHandler(SDLK_6, [this](const SDL_Event&) {
      this->keybd_.number_key = '6';
    }
	); 
  SetKeyUpHandler(SDLK_7, [this](const SDL_Event&) {
      this->keybd_.number_key = '7';
    }
	); 
  SetKeyUpHandler(SDLK_8, [this](const SDL_Event&) {
      this->keybd_.number_key = '8';
    }
	); 
  SetKeyUpHandler(SDLK_9, [this](const SDL_Event&) {
      this->keybd_.number_key = '9';
    }
	);

  // Get screenshot
  SetKeyUpHandler(SDLK_s, [this](const SDL_Event&) {
      this->keybd_.get_screenshot = true;
    }
	); 

  // Use key
  SetKeyUpHandler(SDLK_SPACE, [this](const SDL_Event&) {
      this->keybd_.use = true;
    }
	); 
}

void AppClass::ProcessScene([[maybe_unused]] int ms_elapsed) {
  if (keybd_.get_screenshot) {
    keybd_.get_screenshot = false;
    wnd_.SaveScreenshot("screen.bmp");
  }

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

  player_->SetFireFlag(IsButtonPressed(SDL_SCANCODE_LCTRL));

  if (keybd_.number_key) {
    player_->ChangeWeapon(keybd_.number_key);
    keybd_.number_key = 0;
  }

  if (keybd_.use) {
    player_->SetUseFlag();
    keybd_.use = false;
  }

  player_->Move(delta_angle, forward_move, side_move);

  world_.TickTime();
}

void AppClass::RenderScene() {
  renderer_.RenderScene(world_.GetBsp(), &gm_, world_.GetPlayer());
}