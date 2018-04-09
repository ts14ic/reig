#include "StopWatch.h"

#include <reig.h>

#include <SDL2_gfxPrimitives.h>
#include <SDL2_framerate.h>

#include <iostream>
#include <iomanip>

using namespace std::string_literals;

class Main {
public:
    Main() {
        SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_EVENTS);
        sdl.window = SDL_CreateWindow(
            "reig SDL testbed",
            SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
            sdl.width, sdl.height,
            SDL_WINDOW_SHOWN
        );
        sdl.renderer = SDL_CreateRenderer(
            sdl.window, -1, 
            SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
        );
        
        gui.ctx.set_render_handler(&gui_handler);
        gui.ctx.set_user_ptr(this);

        gui.font.data = gui.ctx.set_font("/usr/share/fonts/TTF/impact.ttf", gui.font.id, 32.f);
        auto* surf = SDL_CreateRGBSurfaceFrom(
            gui.font.data.bitmap.data(), gui.font.data.width, gui.font.data.height, 8, gui.font.data.width,
            0, 0, 0, 0
        );
        SDL_Color colors[256];
        {
            Uint8 v = 0;
            for(SDL_Color& color: colors) {
                color.r = color.g = color.b = 0xFF;
                color.a = v++;
            }
        }
        if(SDL_SetPaletteColors(surf->format->palette, colors, 0, 256) < 0) {
            std::cerr << "Failed to set palette\n";
            exit(1);
        }
        gui.font.tex = SDL_CreateTextureFromSurface(sdl.renderer, surf);
        if(SDL_SetTextureBlendMode(gui.font.tex, SDL_BLENDMODE_BLEND) < 0) {
            std::cerr << "Failed to set texture blend mode: ["<< SDL_GetError() << "]\n";
            exit(0);
        }
    }
    
    int run() {
        int last;
        std::string fpsString;
        bool quit = false;
        // FPSmanager fpsManager;
        // SDL_initFramerate(&fpsManager);
        // SDL_setFramerate(&fpsManager, 60);
        while(!quit) {
            last = SDL_GetTicks();
            gui.ctx.start_new_frame();
            
            // =================== Input polling ===============
            for(SDL_Event evt; SDL_PollEvent(&evt);) {
                switch(evt.type) {
                    case SDL_QUIT:
                    quit = true;
                    break;
                    
                    case SDL_MOUSEMOTION:
                    gui.ctx.mouse.place(evt.motion.x, evt.motion.y);
                    break;
                    
                    case SDL_MOUSEWHEEL:
                    gui.ctx.mouse.scroll(-evt.wheel.y);
                    break;
                    
                    case SDL_MOUSEBUTTONDOWN: {
                        if(evt.button.button == SDL_BUTTON_LEFT) {
                            gui.ctx.mouse.leftButton.press(evt.button.x, evt.button.y);
                        }
                    }
                    break;
                    
                    case SDL_MOUSEBUTTONUP: {
                        if(evt.button.button == SDL_BUTTON_LEFT) {
                            gui.ctx.mouse.leftButton.release();
                        }
                    }
                    
                    default:;
                }
                if(quit) break;
            }
            if(quit) break;
            
            // ================== GUI setup =================
            using namespace reig::Colors::literals;
            using namespace reig::Colors::operators;

            reig::Rectangle rect {40, 0, 100, 30};
            reig::Color color {120_r, 100_g, 150_b};
            
            static float winX = 10, winY = 10;
            gui.ctx.start_window(fpsString.c_str(), winX, winY);
            for(int i = 0; i < 4; ++i) {
                rect.x -= 10; rect.y = 40 * i;
                color = color + 25_r + 25_g;
                std::string title = "some  " + std::to_string(i + 1);
                if(gui.ctx.widget(reig::button{title.c_str(), rect, color})) {
                    std::cout << "Button " << (i + 1) << ": pressed" << std::endl;
                }
            }

            color = color + 50_g;
            rect.y += 40; rect.width += 50;
            static float sliderValue0 = 20;
            if (gui.ctx.widget(reig::slider{rect, color, sliderValue0, 20, 40, 5})) {
                std::cout << "Slider 1: new value " << sliderValue0 << std::endl;
            }

            color = color + 50_g;
            rect.y += 40;
            rect.width += 50;
            
            static float sliderValue1 = 5.4f;
            if (gui.ctx.widget(reig::slider{rect, color, sliderValue1, 3, 7, 0.1f})) {
                std::cout << "Slider 2: new value " << sliderValue1 << std::endl;
            }
            
            static float sliderValue2 = 0.3f;
            rect.y += 40; rect.width += 50; rect.height += 10;
            if (gui.ctx.widget(reig::slider{rect, {220_r, 200_g, 150_b}, sliderValue2, 0.1f, 0.5f, 0.05f})) {
                std::cout << "Slider 2: new value " << sliderValue2 << std::endl;
            }
            
            static bool checkBox1 = false;
            color = color + 15_r - 35_r - 10_b;
            rect.x += 270; rect.width = 40; rect.height = 20;
            if(gui.ctx.widget(reig::checkbox{rect, color, checkBox1})) {
                std::cout << "Checkbox 1: new value " << checkBox1 << std::endl;
            }
            
            static bool checkBox2 = true;
            color = color - 100_r + 100_g + 100_b;
            rect.y -= 100; rect.width = rect.height = 50;
            if(gui.ctx.widget(reig::checkbox{rect, color, checkBox2})) {
                std::cout << "Checkbox 2: new value " << checkBox2 << std::endl;
            }
            
            static bool checkBox3 = false;
            color = reig::Colors::from_uint(0xFFFFFFFFu);
            rect.y += 60; rect.width = rect.height = 25;
            if(gui.ctx.widget(reig::checkbox{rect, color, checkBox3})) {
                std::cout << "Checkbox 3: new value " << checkBox3 << std::endl;
            }
            
            gui.ctx.end_window();
            
            // ================== Render ==================== 
            SDL_SetRenderDrawColor(sdl.renderer, 50, 50, 50, 255);
            SDL_RenderClear(sdl.renderer);
            
            auto ticks = SDL_GetTicks() - last;
            if(ticks != 0) {
                ticks = 1000 / ticks;
                fpsString = std::to_string(ticks) + " FPS";
            }
            gui.ctx.render_all();
            
            int mx, my;
            int state = SDL_GetMouseState(&mx, &my);
            if((state & SDL_BUTTON_LMASK) == SDL_BUTTON_LMASK) {
                filledCircleRGBA(sdl.renderer, mx, my, 15, 150, 220, 220, 150);
            }
            
            SDL_RenderPresent(sdl.renderer);
        }
        
        return 0;
    }

    static void gui_handler(reig::DrawData const& drawData, std::any& userPtr) {
        Main* self = std::any_cast<Main*>(userPtr);
        
        for(auto const& fig : drawData) {
            auto const& vertices = fig.vertices();
            auto const& indices = fig.indices();
            auto number = indices.size();
            
            if(number % 3 != 0) {
                continue;
            }
            
            if(fig.texture() == 0) {
                for(auto i = 0ul; i < number; i += 3) {
                    filledTrigonColor(
                        self->sdl.renderer,
                        vertices[indices[i  ]].position.x,
                        vertices[indices[i  ]].position.y,
                        vertices[indices[i+1]].position.x,
                        vertices[indices[i+1]].position.y,
                        vertices[indices[i+2]].position.x,
                        vertices[indices[i+2]].position.y,
                        reig::Colors::to_uint(vertices[i].color)
                    );
                }
            }
            else if(fig.texture() == self->gui.font.id) {
                SDL_Rect src;
                src.x = vertices[0].texCoord.x * self->gui.font.data.width;
                src.y = vertices[0].texCoord.y * self->gui.font.data.height;
                src.w = vertices[2].texCoord.x * self->gui.font.data.width  - src.x;
                src.h = vertices[2].texCoord.y * self->gui.font.data.height - src.y;
                SDL_Rect dst;
                dst.x = vertices[0].position.x;
                dst.y = vertices[0].position.y;
                dst.w = vertices[2].position.x - dst.x;
                dst.h = vertices[2].position.y - dst.y;
                SDL_RenderCopy(self->sdl.renderer, self->gui.font.tex, &src, &dst);
            }
        }
    }
    
private:
    struct sdl {
        SDL_Renderer* renderer = nullptr;
        SDL_Window*   window   = nullptr;
        int width  = 800;
        int height = 600;
    }
    sdl;
    
    struct gui {
        reig::Context ctx;
        
        struct font {
            reig::FontData data;
            SDL_Texture*   tex  = nullptr;
            int            id   = 100;
        }
        font;
    }
    gui;
};

int main(int, char*[]) {
    return Main{}.run();
}
