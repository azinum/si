// si_gfx.c

#include <stdlib.h>
#include <si.h>
#include <SDL2/SDL.h>

#define MAX_TITLE_LENGTH 64

static SDL_Renderer* renderer;
static SDL_Window* window;
static SDL_Event event;

int object_to_int(const struct Object* object, int* value) {
  if (object->type != T_NUMBER) {
    si_error("Invalid type\n");
    return -1;
  }
  *value = object->value.number;
  return 0;
}

int object_to_int2(const struct Object* object) {
  if (object->type != T_NUMBER) {
    si_error("Invalid type\n");
    return -1;
  }
  return (int)object->value.number;
}

// (x, y, w, h, r, g, b)
static int libsi_gfx_drawrect(struct VM_state* vm) {
  int arg_count = si_get_argc(vm);
  if (arg_count < 7) {
    si_error("Missing arguments (x, y, w, h, r, g, b)\n");
    return 0;
  }
  const int x = object_to_int2(si_get_arg(vm, 0));
  const int y = object_to_int2(si_get_arg(vm, 1));
  const int w = object_to_int2(si_get_arg(vm, 2));
  const int h = object_to_int2(si_get_arg(vm, 3));
  const int r = object_to_int2(si_get_arg(vm, 4));
  const int g = object_to_int2(si_get_arg(vm, 5));
  const int b = object_to_int2(si_get_arg(vm, 6));
  
  const SDL_Rect rect = {
    x, y, w, h
  };
  SDL_SetRenderDrawColor(renderer, r, g, b, 255);
  SDL_RenderDrawRect(renderer, &rect);
  return 0;
}

// (title, width, height)
static int libsi_gfx_createwindow(struct VM_state* vm) {
  int arg_count = si_get_argc(vm);
  if (arg_count < 3) {
    si_error("Missing arguments (title, width, height)\n");
    return 0;
  }
  if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
    si_error("%s\n", SDL_GetError());
    return 0;
  }

  struct Object* title_obj = si_get_arg(vm, 0);
  if (title_obj->type != T_STRING) {
    si_error("Invalid argument type (should be T_STRING)\n");
    return 0;
  }
  char title[MAX_TITLE_LENGTH] = {0};
  snprintf(title, MAX_TITLE_LENGTH, "%.*s", title_obj->value.str.length, title_obj->value.str.data);

  int width, height = 0;
  if (object_to_int(si_get_arg(vm, 1), &width) != 0) return 0;
  if (object_to_int(si_get_arg(vm, 2), &height) != 0) return 0;

  if (!(window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, 0))) {
    si_error("%s\n", SDL_GetError());
    return 0;
  }
  if (!(renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC))) {
    si_error("%s\n", SDL_GetError());
    return 0;
  }
  if (SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND) != 0) {
    si_error("%s\n", SDL_GetError());
    return 0;
  }
  return 0;
}

static int libsi_gfx_swapbuffers(struct VM_state* vm) {
  SDL_RenderPresent(renderer);
  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
  SDL_RenderClear(renderer);
  return 0;
}

static int libsi_gfx_pollevent(struct VM_state* vm) {
  while (SDL_PollEvent(&event)) {
    switch (event.type) {
      case SDL_QUIT:
        si_push_number(vm, 1);
        return 1;

      case SDL_KEYDOWN: {
        switch (event.key.keysym.sym) {
          case SDLK_ESCAPE:
            si_push_number(vm, 1);
            return 1;
          default:
            break;
        }
        break;                  
      }
      default:
        break;
    }
  }
  si_push_number(vm, 0);
  return 1;
}

static int libsi_gfx_destroywindow(struct VM_state* vm) {
  SDL_DestroyWindow(window);
  SDL_DestroyRenderer(renderer);
  SDL_Quit();
  return 0;
}

static struct Lib_def libsi_sdl_funcs[] = {
  {"gfx_drawrect", libsi_gfx_drawrect},
  {"gfx_createwindow", libsi_gfx_createwindow},
  {"gfx_swapbuffers", libsi_gfx_swapbuffers},
  {"gfx_pollevent", libsi_gfx_pollevent},
  {"gfx_destroywindow", libsi_gfx_destroywindow},
  {NULL, NULL},
};

extern int init(struct VM_state* vm) {
  lib_load(vm, libsi_sdl_funcs);
  return 0;
}

