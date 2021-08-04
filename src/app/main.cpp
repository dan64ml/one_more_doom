#include <iostream>
#include <SDL2/SDL.h>
#include <vector>

#include "app_class.h"

using namespace std;

int main(int, char**) {
  AppClass app;

  app.RunFixedStepLoop(27);
  
  return 0;
}
