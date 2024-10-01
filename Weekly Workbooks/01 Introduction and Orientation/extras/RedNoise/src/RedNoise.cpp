#include <CanvasTriangle.h>
#include <DrawingWindow.h>
#include <Utils.h>
#include <fstream>
#include <vector>
#include <glm/glm.hpp>

#define WIDTH 320
#define HEIGHT 240

void draw(DrawingWindow &window) {
    window.clearPixels();
    for (size_t y = 0; y < window.height; y++) {
        for (size_t x = 0; x < window.width; x++) {
            float red = rand() % 256;
            float green = 0.0;
            float blue = 0.0;
            uint32_t colour = (255 << 24) + (int(red) << 16) + (int(green) << 8) + int(blue);
            window.setPixelColour(x, y, colour);
        }
    }
}

std::vector<float> interpolateSingleFloats(float from, float to, int numberOfValues) {
    std::vector<float> interpolated_floats = {from};

    float step_size = (to - from) / (float(numberOfValues) - 1);

    for(int i = 0; i < numberOfValues - 1; i++) {
        interpolated_floats.push_back(interpolated_floats.back() + step_size);
    }

    return interpolated_floats;
}

std::vector<glm::vec3> interpolateThreeElementValues(glm::vec3 from, glm::vec3 to, int numberOfValues) {
    std::vector<glm::vec3> interpolated_vectors = {from};


    glm::vec3 step_size = (to - from) / (float(numberOfValues) - 1);

    for(int i = 0; i < numberOfValues - 1; i++) {
        interpolated_vectors.push_back(interpolated_vectors.back() + step_size);
    }

    return interpolated_vectors;


}

void draw_greyscale(DrawingWindow &window) {
    window.clearPixels();

    std::vector<float> interpolated_floats = interpolateSingleFloats(256, 0, int(window.width));

    for (size_t y = 0; y < window.height; y++) {
        for (size_t x = 0; x < window.width; x++) {
            float red = interpolated_floats[x];
            float green = interpolated_floats[x];
            float blue = interpolated_floats[x];
            uint32_t colour = (255 << 24) + (int(red) << 16) + (int(green) << 8) + int(blue);
            window.setPixelColour(x, y, colour);
        }
    }
}

void draw_colour(DrawingWindow &window) {
    window.clearPixels();

    glm::vec3 topLeft(255, 0, 0);        // red
    glm::vec3 topRight(0, 0, 255);       // blue
    glm::vec3 bottomRight(0, 255, 0);    // green
    glm::vec3 bottomLeft(255, 255, 0);   // yellow

    std::vector<glm::vec3> left_column= interpolateThreeElementValues(topLeft, bottomLeft, window.height);
    std::vector<glm::vec3> right_column= interpolateThreeElementValues(topRight, bottomRight, window.height);

    for (size_t y = 0; y < window.height; y++) {
        std::vector<glm::vec3> horizontal = interpolateThreeElementValues(left_column[y], right_column[y], window.width);

        for (size_t x = 0; x < window.width; x++) {
            float red = horizontal[x].x;
            float green = horizontal[x].y;
            float blue = horizontal[x].z;
            uint32_t colour = (255 << 24) + (int(red) << 16) + (int(green) << 8) + int(blue);
            window.setPixelColour(x, y, colour);
        }
    }
}

void handleEvent(SDL_Event event, DrawingWindow &window) {
    if (event.type == SDL_KEYDOWN) {
        if (event.key.keysym.sym == SDLK_LEFT) std::cout << "LEFT" << std::endl;
        else if (event.key.keysym.sym == SDLK_RIGHT) std::cout << "RIGHT" << std::endl;
        else if (event.key.keysym.sym == SDLK_UP) std::cout << "UP" << std::endl;
        else if (event.key.keysym.sym == SDLK_DOWN) std::cout << "DOWN" << std::endl;
    } else if (event.type == SDL_MOUSEBUTTONDOWN) {
        window.savePPM("output.ppm");
        window.saveBMP("output.bmp");
    }
}


int main(int argc, char *argv[]) {

    std::vector<glm::vec3> result;

    glm::vec3 from(1.0, 4.0, 9.2);
    glm::vec3 to(4.0, 1.0, 9.8);

    result = interpolateThreeElementValues(from, to, 4);
    for(size_t i=0; i<result.size(); i++) std::cout << "("
                                                    << result[i].x << ", " << result[i].y << ", " << result[i].z
                                                    << ") \n";
    std::cout << std::endl;

    DrawingWindow window = DrawingWindow(WIDTH, HEIGHT, false);
    SDL_Event event;
    while (true) {
        // We MUST poll for events - otherwise the window will freeze !
        if (window.pollForInputEvents(event)) handleEvent(event, window);
        draw_colour(window);
        // Need to render the frame at the end, or nothing actually gets shown on the screen !
        window.renderFrame();
    }
}
