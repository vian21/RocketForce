#include <stdio.h>
#include <SDL.h>

/* https://github.com/Polytonic/Glitter/issues/70#issuecomment-766281506 */
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_sdlrenderer2.h"

#if !SDL_VERSION_ATLEAST(2, 0, 17)
#error This backend requires SDL 2.0.17+ because of SDL_RenderGeometry() function
#endif

int rocketState = 0;
bool isMoving = false; // tracking if the rocket is moving 
float pos = 720.0f; // initial position of the rocket at the bottom of the screen
float speed = -200.0f; // move speed of the rocket
Uint32 lastTime = 0;

bool LoadTextureFromFile(const char* filename, SDL_Texture** texture_ptr, int& width, int& height, SDL_Renderer* renderer) {
    int channels;
    unsigned char* data = stbi_load(filename, &width, &height, &channels, 0);

    if (data == nullptr) {
        fprintf(stderr, "Failed to load image: %s\n", stbi_failure_reason());
        return false;
    }

    SDL_Surface* surface = SDL_CreateRGBSurfaceFrom((void*)data, width, height, channels * 8, channels * width,
                                                    0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000);

    if (surface == nullptr) {
        fprintf(stderr, "Failed to create SDL surface: %s\n", SDL_GetError());
        return false;
    }

    *texture_ptr = SDL_CreateTextureFromSurface(renderer, surface);

    if ((*texture_ptr) == nullptr) {
        fprintf(stderr, "Failed to create SDL texture: %s\n", SDL_GetError());
    }

    SDL_FreeSurface(surface);
    stbi_image_free(data);

    return true;
}

Uint32 updateRocket(Uint32 interval, void *param) {
    rocketState = (rocketState % 4) + 1;
    return interval;
}

int main(int, char **)
{
    /**** Setup SDL ****/
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0)
    {
        printf("Error: %s\n", SDL_GetError());
        return -1;
    }

#ifdef SDL_HINT_IME_SHOW_UI
    SDL_SetHint(SDL_HINT_IME_SHOW_UI, "1");
#endif

    // Create window with SDL_Renderer graphics context
    SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    SDL_Window *window = SDL_CreateWindow("Dear ImGui SDL2+SDL_Renderer example", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, window_flags);
    if (window == nullptr)
    {
        printf("Error: SDL_CreateWindow(): %s\n", SDL_GetError());
        return -1;
    }
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED);
    if (renderer == nullptr)
    {
        SDL_Log("Error creating SDL_Renderer!");
        return 0;
    }

    /**** END SDL setup ****/

    /**** IMGUI setup ****/
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO(); //get keyboard events and frame rates
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsLight();

    // Setup Platform/Renderer backends
    ImGui_ImplSDL2_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer2_Init(renderer);

    SDL_TimerID timerID = SDL_AddTimer(1500, updateRocket, NULL);

    // Main loop
    bool done = false;
    while (!done)
    {
        SDL_Event event;
        // read keyboard events e.g spacebar, cmd+Q, quit
        while (SDL_PollEvent(&event))
        {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT)
                done = true;
            if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(window))
                done = true;
            if (event.type == SDL_KEYDOWN && event.key.keysym.sym ==SDLK_SPACE )
                isMoving = true;
        }

        //Calculate time elapsed
        Uint32 currentTime = SDL_GetTicks(); //Returns elapsed time in milliseconds
        float deltaTime = (currentTime - lastTime)/1000.0f; // calculate deltaTime and converting in seconds
        lastTime = currentTime;

        //Update rocket position 
        if (isMoving){
            pos += speed * deltaTime;
            if (pos < 0){
                // reset the rocket position when reaching the top
                pos = 720.0f;
                isMoving = false;

            }
        }

        // Start the Dear ImGui frame
        ImGui_ImplSDLRenderer2_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        //Add the image
        SDL_Texture* my_texture;
        int my_image_width, my_image_height;
        bool ret = LoadTextureFromFile("assets/Rocket.png", &my_texture, my_image_width, my_image_height, renderer);
        (void)ret;

        /* Display image with IMGUI 
         * https://github.com/ocornut/imgui/issues/1635
         */
        ImGui::Begin("Rocket sprite on screen", NULL,
                    ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground |
                    ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_AlwaysAutoResize |
                    ImGuiWindowFlags_NoSavedSettings);
        ImGui::Text("pointer = %p", my_texture);
        ImGui::Text("size = %d x %d", my_image_width, my_image_height);
        ImVec2 position = ImVec2(640.0f - my_image_width / 2, pos - my_image_height / 2);  // centré rocket
        ImGui::SetCursorPos(position);
 
        /* Image splitting: https://github.com/ocornut/imgui/issues/675 */
        ImGui::Image((void*) my_texture, 
                    ImVec2(my_image_width, my_image_height),
                    ImVec2(0.0f + 0.25f * (rocketState - 1), 0.0f + 0.25f * (rocketState - 1)),
                    ImVec2(0.25f * rocketState, 0.25f * rocketState));
        printf ("(%f, %f), (%f, %f)\n",
                (0.0f + 0.25f * (rocketState - 1)),
                (0.0f + 0.25f * (rocketState - 1)),
                0.25f * rocketState,
                0.25f * rocketState);
        ImGui::End();

        // Rendering
        ImGui::Render();
        SDL_RenderSetScale(renderer, io.DisplayFramebufferScale.x, io.DisplayFramebufferScale.y);
        SDL_RenderClear(renderer);
        ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData());
        SDL_RenderPresent(renderer);
    }

    /**** Cleanup ****/
    ImGui_ImplSDLRenderer2_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_RemoveTimer(timerID);

    SDL_Quit();

    return 0;
}