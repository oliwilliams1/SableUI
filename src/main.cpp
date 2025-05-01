#include <SDL3/SDL.h>
#include <vector>
#include <cstdio> // For printf

const int WIDTH = 800;
const int HEIGHT = 600;

int main() {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return -1;
    }

    SDL_Window* window = SDL_CreateWindow("Pixel Buffer", WIDTH, HEIGHT, 0);
    if (!window) {
        printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        SDL_Quit();
        return -1;
    }

    SDL_Surface* surface = SDL_GetWindowSurface(window);
    std::vector<uint32_t> pixelBuffer(WIDTH * HEIGHT, 0xFF000000); // Black

    // Modify pixel buffer (example: fill with red)
    for (int y = 0; y < HEIGHT; ++y) {
        for (int x = 0; x < WIDTH; ++x) {
            pixelBuffer[y * WIDTH + x] = 0xFFFF0000; // Red
        }
    }

    // Lock the surface to access pixel data
    SDL_LockSurface(surface);
    std::memcpy(surface->pixels, pixelBuffer.data(), pixelBuffer.size() * sizeof(uint32_t));
    SDL_UnlockSurface(surface);

    SDL_UpdateWindowSurface(window);
    SDL_Delay(3000); // Show for 3 seconds

    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}