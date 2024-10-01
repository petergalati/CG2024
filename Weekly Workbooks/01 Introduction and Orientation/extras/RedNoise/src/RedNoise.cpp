#include <CanvasTriangle.h>
#include <CanvasPoint.h>
#include <Colour.h>
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

void drawGreyscale(DrawingWindow &window) {
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

void drawColour(DrawingWindow &window) {
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

void drawLine(DrawingWindow &window, CanvasPoint from, CanvasPoint to, Colour colour) {
    float dx = to.x - from.x;
    float dy = to.y - from.y;

    float steps = fmax(abs(dx), abs(dy));

    float x_step_size = dx / steps;
    float y_step_size = dy / steps;

    uint32_t packed_colour = (255 << 24) + (int(colour.red) << 16) + (int(colour.green) << 8) + int(colour.blue);

    for (float i = 0.0; i <= steps; i++) {
        float x = from.x + x_step_size * i;
        float y = from.y + y_step_size * i;
        window.setPixelColour(round(x), round(y), packed_colour);
    }
}

void drawStrokedTriangle(DrawingWindow &window, CanvasTriangle triangle, Colour colour) {
    drawLine(window, triangle.v0(), triangle.v1(), colour);
    drawLine(window, triangle.v1(), triangle.v2(), colour);
    drawLine(window, triangle.v0(), triangle.v2(), colour);
}

void drawFilledTriangle(DrawingWindow &window, CanvasTriangle triangle, Colour colour) {
    if (triangle.v1().y < triangle.v0().y) std::swap(triangle.v1(), triangle.v0());
    if (triangle.v2().y < triangle.v0().y) std::swap(triangle.v2(), triangle.v0());
    if (triangle.v2().y < triangle.v1().y) std::swap(triangle.v2(), triangle.v1());

    float ratio = (triangle.v2().x - triangle.v0().x) / (triangle.v2().y - triangle.v0().y);
    float split_point_x = triangle.v0().x + ratio * (triangle.v1().y - triangle.v0().y);

    for (int y = triangle.v0().y; y < triangle.v2().y; y++) {
        if (y <= triangle.v1().y) {
            float ratio_a = (triangle.v1().x - triangle.v0().x) / (triangle.v1().y - triangle.v0().y);
            float x_a = triangle.v0().x + (y - triangle.v0().y) * ratio_a;

            float ratio_b = (split_point_x - triangle.v0().x) / (triangle.v1().y - triangle.v0().y);
            float x_b = triangle.v0().x + (y - triangle.v0().y) * ratio_b;

            CanvasPoint from_a = CanvasPoint(x_a, y);
            CanvasPoint from_b = CanvasPoint(x_b, y);

            drawLine(window, from_a, from_b, colour);
        } else {
            float ratio_a = (triangle.v2().x - triangle.v1().x) / (triangle.v2().y - triangle.v1().y);
            float x_a = triangle.v1().x + (y - triangle.v1().y) * ratio_a;

            float ratio_b = (triangle.v2().x - split_point_x) / (triangle.v2().y - triangle.v1().y);
            float x_b = split_point_x + (y - triangle.v1().y) * ratio_b;

            CanvasPoint from_a = CanvasPoint(x_a, y);
            CanvasPoint from_b = CanvasPoint(x_b, y);

            drawLine(window, from_a, from_b, colour);
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


//int main(int argc, char *argv[]) {
//
//    std::vector<glm::vec3> result;
//
//    glm::vec3 from(1.0, 4.0, 9.2);
//    glm::vec3 to(4.0, 1.0, 9.8);
//
//    result = interpolateThreeElementValues(from, to, 4);
//    for(size_t i=0; i<result.size(); i++) std::cout << "("
//                                                    << result[i].x << ", " << result[i].y << ", " << result[i].z
//                                                    << ") \n";
//    std::cout << std::endl;
//
//    DrawingWindow window = DrawingWindow(WIDTH, HEIGHT, false);
//    SDL_Event event;
//    while (true) {
//        // We MUST poll for events - otherwise the window will freeze !
//        if (window.pollForInputEvents(event)) handleEvent(event, window);
//        draw_colour(window);
//        // Need to render the frame at the end, or nothing actually gets shown on the screen !
//        window.renderFrame();
//    }
//}



// draw line
//int main(int argc, char *argv[]) {
//
//
//    DrawingWindow window = DrawingWindow(WIDTH, HEIGHT, false);
//
//    SDL_Event event;
//    while (true) {
//        // We MUST poll for events - otherwise the window will freeze !
//        if (window.pollForInputEvents(event)) handleEvent(event, window);
//        drawLine(
//                window,
//                CanvasPoint(0, 0),
//                CanvasPoint(WIDTH / 2, HEIGHT / 2),
//                Colour(255, 255, 255)
//        );
//
//        drawLine(
//                window,
//                CanvasPoint(WIDTH - 1, 0),
//                CanvasPoint(WIDTH / 2, HEIGHT / 2),
//                Colour(255, 255, 255)
//        );
//
//        drawLine(
//                window,
//                CanvasPoint(WIDTH / 2, 0),
//                CanvasPoint(WIDTH / 2, HEIGHT - 1),
//                Colour(255, 255, 255)
//        );
//
//        drawLine(
//                window,
//                CanvasPoint(WIDTH / 3, HEIGHT / 2),
//                CanvasPoint(2 * (WIDTH / 3), HEIGHT / 2),
//                Colour(255, 255, 255)
//        );
//
//        // Need to render the frame at the end, or nothing actually gets shown on the screen !
//        window.renderFrame();
//    }
//}



// draw stroked triangle
//int main(int argc, char *argv[]) {
//
//    DrawingWindow window = DrawingWindow(WIDTH, HEIGHT, false);
//
//    SDL_Event event;
//    while (true) {
//        // We MUST poll for events - otherwise the window will freeze !
//        if (window.pollForInputEvents(event)) handleEvent(event, window);
//
//        if (event.type == SDL_KEYDOWN) {
//            if (event.key.keysym.sym == SDLK_u) {
//                Colour random_colour = Colour(rand() % 256, rand() % 256, rand() % 256);
//
//                CanvasPoint random_v0 = CanvasPoint(rand() % WIDTH, rand() % HEIGHT, HEIGHT);
//                CanvasPoint random_v1 = CanvasPoint(rand() % WIDTH, rand() % HEIGHT, HEIGHT);
//                CanvasPoint random_v2 = CanvasPoint(rand() % WIDTH, rand() % HEIGHT, HEIGHT);
//
//                CanvasTriangle random_triangle = CanvasTriangle(random_v0, random_v1, random_v2);
//
//                drawStrokedTriangle(window, random_triangle, random_colour);
//
//            }
//        }
//
//
//
//
//        // Need to render the frame at the end, or nothing actually gets shown on the screen !
//        window.renderFrame();
//    }
//}



// draw filled triangle
int main(int argc, char *argv[]) {

    DrawingWindow window = DrawingWindow(WIDTH, HEIGHT, false);

    SDL_Event event;
    while (true) {
        // We MUST poll for events - otherwise the window will freeze !
        if (window.pollForInputEvents(event)) handleEvent(event, window);

        if (event.type == SDL_KEYDOWN) {
            if (event.key.keysym.sym == SDLK_f) {
                Colour random_colour = Colour(rand() % 256, rand() % 256, rand() % 256);

                CanvasPoint random_v0 = CanvasPoint(rand() % WIDTH, rand() % HEIGHT, HEIGHT);
                CanvasPoint random_v1 = CanvasPoint(rand() % WIDTH, rand() % HEIGHT, HEIGHT);
                CanvasPoint random_v2 = CanvasPoint(rand() % WIDTH, rand() % HEIGHT, HEIGHT);

                CanvasTriangle random_triangle = CanvasTriangle(random_v0, random_v1, random_v2);

                drawFilledTriangle(window, random_triangle, random_colour);
                drawStrokedTriangle(window, random_triangle, Colour(255, 255, 255));


            }
        }




        // Need to render the frame at the end, or nothing actually gets shown on the screen !
        window.renderFrame();
    }
}
