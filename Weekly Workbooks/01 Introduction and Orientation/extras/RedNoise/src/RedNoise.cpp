#include <CanvasTriangle.h>
#include <CanvasPoint.h>
#include <Colour.h>
#include <DrawingWindow.h>
#include <TextureMap.h>
#include <ModelTriangle.h>
#include <Utils.h>
#include <fstream>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>

#include <unordered_map>

#define WIDTH 320
#define HEIGHT 240

glm::vec3 g_camera_position(0.0, 0.0, 4.0);
glm::mat3 g_camera_orientation = glm::mat3(1.0);


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

void drawLine(DrawingWindow &window, CanvasPoint from, CanvasPoint to, const Colour& colour) {
    float dx = to.x - from.x;
    float dy = to.y - from.y;

    float steps = fmax(abs(dx), abs(dy));

    float x_step_size = dx / steps;
    float y_step_size = dy / steps;

    uint32_t packed_colour = (255 << 24) + (int(colour.red) << 16) + (int(colour.green) << 8) + int(colour.blue);

    for (int i = 0; i <= steps; i++) {
        float x = from.x + x_step_size * i;
        float y = from.y + y_step_size * i;
        window.setPixelColour(round(x), round(y), packed_colour);
    }
}

void drawDepthLine(DrawingWindow &window, CanvasPoint from, CanvasPoint to, const Colour& colour, std::vector<std::vector<float>>& depth_vector) {
    float dx = to.x - from.x;
    float dy = to.y - from.y;
    float d_depth = to.depth - from.depth;

//    not sure why this works, but without ceil lines look odd
    float steps = ceil(fmax(abs(dx), abs(dy)));

    float x_step_size = dx / steps;
    float y_step_size = dy / steps;
    float depth_step_size = d_depth / steps;

    uint32_t packed_colour = (255 << 24) + (int(colour.red) << 16) + (int(colour.green) << 8) + int(colour.blue);

    for (int i = 0; i <= steps; i++) {
        float current_depth = from.depth + depth_step_size * i;
        float x = round(from.x + x_step_size * i);
        float y = round(from.y + y_step_size * i);

        if (current_depth > depth_vector[y][x]) {
            window.setPixelColour(x, y, packed_colour);
            depth_vector[y][x] = current_depth;
        }
    }
}

void drawStrokedTriangle(DrawingWindow &window, CanvasTriangle triangle, Colour colour) {
    drawLine(window, triangle.v0(), triangle.v1(), colour);
    drawLine(window, triangle.v1(), triangle.v2(), colour);
    drawLine(window, triangle.v0(), triangle.v2(), colour);
}

CanvasPoint getCanvasPixelGivenProportion(CanvasPoint start, CanvasPoint end, float proportion) {
    float dx = end.x - start.x;
    float dy = end.y - start.y;

    float new_x = start.x + dx * proportion;
    float new_y = start.y + dy * proportion;

    return {new_x, new_y};
}

CanvasPoint getTexturePixelGivenProportion(CanvasPoint start, CanvasPoint end, float proportion) {
    float dx = end.texturePoint.x - start.texturePoint.x;
    float dy = end.texturePoint.y - start.texturePoint.y;

    float new_x = start.texturePoint.x + dx * proportion;
    float new_y = start.texturePoint.y + dy * proportion;

    CanvasPoint new_texture_point;
    new_texture_point.texturePoint = TexturePoint(new_x, new_y);

    return new_texture_point;
}

float getProportionAlongLineGivenPoint(CanvasPoint start, CanvasPoint end, CanvasPoint point) {
    float start_to_end_x = end.x - start.x;
    float start_to_end_y = end.y - start.y;
    float start_end_magnitude = sqrt((start_to_end_x) * (start_to_end_x) + (start_to_end_y) * (start_to_end_y));

    float start_to_point_x = point.x - start.x;
    float start_to_point_y = point.y - start.y;
    float start_point_magnitude = sqrt((start_to_point_x) * (start_to_point_x) + (start_to_point_y) * (start_to_point_y));

    return start_point_magnitude / start_end_magnitude;
}

void drawFilledTriangle(DrawingWindow &window, CanvasTriangle triangle, const Colour& colour, std::vector<std::vector<float>>& depth_vector) {
    if (triangle.v1().y < triangle.v0().y) std::swap(triangle.v1(), triangle.v0());
    if (triangle.v2().y < triangle.v0().y) std::swap(triangle.v2(), triangle.v0());
    if (triangle.v2().y < triangle.v1().y) std::swap(triangle.v2(), triangle.v1());

    float proportion_split = (triangle.v1().y - triangle.v0().y) / (triangle.v2().y - triangle.v0().y);
    float split_point_x = triangle.v0().x + proportion_split * (triangle.v2().x - triangle.v0().x);
    float split_point_depth = triangle.v0().depth + proportion_split * (triangle.v2().depth - triangle.v0().depth);
    CanvasPoint split_point(split_point_x, triangle.v1().y, split_point_depth);

    for (int y = int(ceil(triangle.v0().y)); y <= int(floor(triangle.v1().y)); y++) {
        float proportion_a = (y - triangle.v0().y) / (triangle.v1().y - triangle.v0().y);
        float x_a = triangle.v0().x + proportion_a * (triangle.v1().x - triangle.v0().x);
        float depth_a = triangle.v0().depth + proportion_a * (triangle.v1().depth - triangle.v0().depth);

        float proportion_b = (y - triangle.v0().y) / (split_point.y - triangle.v0().y);
        float x_b = triangle.v0().x + proportion_b * (split_point.x - triangle.v0().x);
        float depth_b = triangle.v0().depth + proportion_b * (split_point.depth - triangle.v0().depth);

        CanvasPoint from_a(x_a, y, depth_a);
        CanvasPoint from_b(x_b, y, depth_b);

        if (from_a.x > from_b.x) std::swap(from_a, from_b);

        drawDepthLine(window, from_a, from_b, colour, depth_vector);
    }

    for (int y = int(ceil(triangle.v1().y)); y <= int(floor(triangle.v2().y)); y++) {
        float proportion_a = (y - triangle.v1().y) / (triangle.v2().y - triangle.v1().y);
        float x_a = triangle.v1().x + proportion_a * (triangle.v2().x - triangle.v1().x);
        float depth_a = triangle.v1().depth + proportion_a * (triangle.v2().depth - triangle.v1().depth);

        float proportion_b = (y - split_point.y) / (triangle.v2().y - split_point.y);
        float x_b = split_point.x + proportion_b * (triangle.v2().x - split_point.x);
        float depth_b = split_point.depth + proportion_b * (triangle.v2().depth - split_point.depth);

        CanvasPoint from_a(x_a, y, depth_a);
        CanvasPoint from_b(x_b, y, depth_b);

        if (from_a.x > from_b.x) std::swap(from_a, from_b);

        drawDepthLine(window, from_a, from_b, colour, depth_vector);
    }
}


void drawTexturedTriangle(DrawingWindow &window, CanvasTriangle canvas_triangle, const std::string& file_path) {
    TextureMap texture_map = TextureMap(file_path);

//    sort canvas and find split point
    if (canvas_triangle.v1().y < canvas_triangle.v0().y) std::swap(canvas_triangle.v1(), canvas_triangle.v0());
    if (canvas_triangle.v2().y < canvas_triangle.v0().y) std::swap(canvas_triangle.v2(), canvas_triangle.v0());
    if (canvas_triangle.v2().y < canvas_triangle.v1().y) std::swap(canvas_triangle.v2(), canvas_triangle.v1());

    float ratio_canvas = (canvas_triangle.v2().x - canvas_triangle.v0().x) / (canvas_triangle.v2().y - canvas_triangle.v0().y);
    float split_canvas_x = canvas_triangle.v0().x + ratio_canvas * (canvas_triangle.v1().y - canvas_triangle.v0().y);

    CanvasPoint split_point(split_canvas_x, canvas_triangle.v1().y);

//    find proportion of split point along the entire side
    float split_proportion = getProportionAlongLineGivenPoint(canvas_triangle.v0(), canvas_triangle.v2(), split_point);

    CanvasPoint texture_split = getTexturePixelGivenProportion(canvas_triangle.v0(), canvas_triangle.v2(), split_proportion);

    split_point.texturePoint = texture_split.texturePoint;


    for (int y = canvas_triangle.v0().y; y < canvas_triangle.v2().y; y++) {
        if (y <= canvas_triangle.v1().y) {
            float ratio_a = (canvas_triangle.v1().x - canvas_triangle.v0().x) / (canvas_triangle.v1().y - canvas_triangle.v0().y);
            float x_a = canvas_triangle.v0().x + (y - canvas_triangle.v0().y) * ratio_a;

            CanvasPoint current_point_a(x_a, y);
            float proportion_along_a = getProportionAlongLineGivenPoint(canvas_triangle.v0(), canvas_triangle.v1(), current_point_a);
            CanvasPoint point_on_texture_a = getTexturePixelGivenProportion(canvas_triangle.v0(), canvas_triangle.v1(), proportion_along_a);

            float ratio_b = (split_canvas_x - canvas_triangle.v0().x) / (canvas_triangle.v1().y - canvas_triangle.v0().y);
            float x_b = canvas_triangle.v0().x + (y - canvas_triangle.v0().y) * ratio_b;

            CanvasPoint current_point_b(x_b, y);
            float proportion_along_b = getProportionAlongLineGivenPoint(canvas_triangle.v0(), split_point, current_point_b);
            CanvasPoint point_on_texture_b = getTexturePixelGivenProportion(canvas_triangle.v0(), split_point, proportion_along_b);

//          draw horizontal scanline
            for (int x = x_a; x < int(x_b); x++) {
                float proportion = (x - x_a) / (x_b - x_a);
                CanvasPoint texture_point = getTexturePixelGivenProportion(point_on_texture_a, point_on_texture_b, proportion);

                uint32_t texture_colour = texture_map.pixels[int(texture_point.texturePoint.y) * texture_map.width + int(texture_point.texturePoint.x)];
                window.setPixelColour(x, y, texture_colour);
            }

        } else {
            float ratio_a = (canvas_triangle.v2().x - canvas_triangle.v1().x) / (canvas_triangle.v2().y - canvas_triangle.v1().y);
            float x_a = canvas_triangle.v1().x + (y - canvas_triangle.v1().y) * ratio_a;
            CanvasPoint current_point_a = CanvasPoint(x_a, y);

            float proportion_along_a = getProportionAlongLineGivenPoint(canvas_triangle.v1(), canvas_triangle.v2(), current_point_a);
            CanvasPoint point_on_texture_a = getTexturePixelGivenProportion(canvas_triangle.v1(), canvas_triangle.v2(), proportion_along_a);

            float ratio_b = (canvas_triangle.v2().x - split_canvas_x) / (canvas_triangle.v2().y - canvas_triangle.v1().y);
            float x_b = split_canvas_x + (y - canvas_triangle.v1().y) * ratio_b;
            CanvasPoint current_point_b = CanvasPoint(x_b, y);
            float proportion_along_b = getProportionAlongLineGivenPoint(split_point, canvas_triangle.v2(), current_point_b);
            CanvasPoint point_on_texture_b = getTexturePixelGivenProportion(split_point, canvas_triangle.v2(), proportion_along_b);

//          draw horizontal scanline

            for (int x = x_a; x < int(x_b); x++) {
                float proportion = (x - x_a) / (x_b - x_a);
                CanvasPoint texture_point = getTexturePixelGivenProportion(point_on_texture_a, point_on_texture_b, proportion);

                uint32_t texture_colour = texture_map.pixels[int(texture_point.texturePoint.y) * texture_map.width + int(texture_point.texturePoint.x)];
                window.setPixelColour(x, y, texture_colour);
            }
        }
    }

}

std::unordered_map<std::string, Colour> parsePaletteMtlFile(const std::string& file_name) {

    std::unordered_map<std::string, Colour> parsed_palette;

    std::ifstream input_stream(file_name);
    std::string next_line;


    while (std::getline(input_stream, next_line)) {
        if (!next_line.empty()) {
            std::vector<std::string> parsed_line = split(next_line, ' ');
            if (parsed_line[0][0] == 'n') { //n for newmtl
                std::string colour_key = parsed_line[1];

                std::getline(input_stream, next_line);
                std::vector<std::string> parsed_rgb_line = split(next_line, ' ');

                int red = int(round(std::stof(parsed_rgb_line[1]) * 255));
                int green = int(round(std::stof(parsed_rgb_line[2]) * 255));
                int blue = int(round(std::stof(parsed_rgb_line[3]) * 255));

                Colour colour_value(red, green, blue);

                parsed_palette[colour_key] = colour_value;
            }
        }
    }

    return parsed_palette;
}

std::vector<ModelTriangle> parseModelObjFile(const std::string& file_name, float scaling_factor) {
    std::unordered_map<std::string, Colour> parsed_palette = parsePaletteMtlFile("cornell-box.mtl");

    std::vector<ModelTriangle> parsed_model_triangles;

    std::vector<glm::vec3> vertex_tracker;

    std::ifstream input_stream(file_name);
    std::string next_line;

    std::string current_colour;

    while (std::getline(input_stream, next_line)) {
        if (!next_line.empty()) {
            std::vector<std::string> parsed_line = split(next_line, ' ');

            if (next_line.at(0) == 'u') {
                current_colour = parsed_line[1];

            } else if (next_line.at(0) == 'v') {
                float x = std::stof((parsed_line[1])) * scaling_factor;
                float y = std::stof((parsed_line[2])) * scaling_factor;
                float z = std::stof((parsed_line[3])) * scaling_factor;

                glm::vec3 next_vertex(x, y, z);
                vertex_tracker.push_back(next_vertex);

            } else if (next_line.at(0) == 'f') {
                int v0_index = std::stoi(split(parsed_line[1], '/')[0]) - 1;
                int v1_index = std::stoi(split(parsed_line[2], '/')[0]) - 1;
                int v2_index = std::stoi(split(parsed_line[3], '/')[0]) - 1;

                ModelTriangle next_triangle(vertex_tracker[v0_index], vertex_tracker[v1_index],vertex_tracker[v2_index], parsed_palette[current_colour]);

                parsed_model_triangles.push_back(next_triangle);

            }
        }
    }
    return parsed_model_triangles;
}

CanvasPoint projectVertexOntoCanvasPoint(glm::vec3 camera_position, float focal_length, glm::vec3 vertex_position, glm::mat3 camera_orientation) {
    glm::vec3 camera_to_vertex = vertex_position - camera_position;
    glm::vec3 adjusted_vector = camera_to_vertex * camera_orientation;

    float u = -focal_length * (adjusted_vector.x / adjusted_vector.z) * 160 + WIDTH / 2;
    float v = focal_length * (adjusted_vector.y / adjusted_vector.z) * 160 + HEIGHT / 2;
    float depth = -1 / adjusted_vector.z;

    CanvasPoint projected_vertex(u, v);
    projected_vertex.depth = depth;

    return projected_vertex;
}

void drawScene(DrawingWindow &window, const std::vector<ModelTriangle>& parsed_triangles) {
    window.clearPixels();
    std::vector<std::vector<float>> depth_vector(HEIGHT, std::vector<float>(WIDTH, 0.0));

    for (const auto & parsed_triangle : parsed_triangles) {
        std::vector<CanvasPoint> canvas_points;

        for (const auto & vertex : parsed_triangle.vertices) {
            canvas_points.push_back(projectVertexOntoCanvasPoint(g_camera_position, 2, vertex, g_camera_orientation));
        }

        CanvasTriangle canvas_triangle(canvas_points[0], canvas_points[1], canvas_points[2]);
        drawFilledTriangle(window, canvas_triangle, parsed_triangle.colour, depth_vector);
    }

}


void handleEvent(SDL_Event event, DrawingWindow &window) {
    if (event.type == SDL_KEYDOWN) {
        switch (event.key.keysym.sym) {
            case SDLK_LEFT:
                g_camera_position.x += 0.1;
                std::cout << "LEFT" << std::endl;
                break;

            case SDLK_RIGHT:
                g_camera_position.x -= 0.1;
                std::cout << "RIGHT" << std::endl;
                break;

            case SDLK_UP:
                g_camera_position.y -= 0.1;
                std::cout << "UP" << std::endl;
                break;

            case SDLK_DOWN:
                g_camera_position.y += 0.1;
                std::cout << "DOWN" << std::endl;
                break;

            case SDLK_i:
                g_camera_position.z -= 0.1;
                std::cout << "IN" << std::endl;
                break;

            case SDLK_o:
                g_camera_position.z += 0.1;
                std::cout << "OUT" << std::endl;
                break;

            case SDLK_z:
            {
                float angle = glm::radians(1.0);
                glm::mat3 rotation_matrix = glm::mat3(
                        glm::vec3(1, 0, 0),
                        glm::vec3(0, cos(angle), -sin(angle)),
                        glm::vec3(0, sin(angle), cos(angle))
                );
                g_camera_orientation = rotation_matrix * g_camera_orientation;
                std::cout << "ROTATE UP X" << std::endl;

                break;
            }

            case SDLK_c:
            {
                float angle = glm::radians(-1.0);
                glm::mat3 rotation_matrix = glm::mat3(
                        glm::vec3(1, 0, 0),
                        glm::vec3(0, cos(angle), -sin(angle)),
                        glm::vec3(0, sin(angle), cos(angle))
                );
                g_camera_orientation = rotation_matrix * g_camera_orientation;
                std::cout << "ROTATE DOWN X" << std::endl;

                break;
            }

            case SDLK_t:
            {
                float angle = glm::radians(1.0);
                glm::mat3 rotation_matrix = glm::mat3(
                        glm::vec3(cos(angle), 0, sin(angle)),
                        glm::vec3(0, 1, 0),
                        glm::vec3(-sin(angle), 0, cos(angle))
                );
                g_camera_orientation = rotation_matrix * g_camera_orientation;
                std::cout << "ROTATE LEFT Y" << std::endl;

                break;
            }

            case SDLK_u:
            {
                float angle = glm::radians(-1.0);
                glm::mat3 rotation_matrix = glm::mat3(
                        glm::vec3(cos(angle), 0, sin(angle)),
                        glm::vec3(0, 1, 0),
                        glm::vec3(-sin(angle), 0, cos(angle))
                );
                g_camera_orientation = rotation_matrix * g_camera_orientation;
                std::cout << "ROTATE RIGHT Y" << std::endl;

                break;
            }

            default:
                break;
        }
    } else if (event.type == SDL_MOUSEBUTTONDOWN) {
        window.savePPM("output.ppm");
        window.saveBMP("output.bmp");
    }
}

int main(int argc, char *argv[]) {

    DrawingWindow window = DrawingWindow(WIDTH, HEIGHT, false);

    SDL_Event event;

    std::vector<ModelTriangle> parsed_triangles = parseModelObjFile("cornell-box.obj", 0.35);

    while (true) {
        if (window.pollForInputEvents(event)) handleEvent(event, window);

        drawScene(window, parsed_triangles);

        window.renderFrame();
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
//        window.renderFrame();
//    }
//}



// draw filled triangle
//int main(int argc, char *argv[]) {
//
//    DrawingWindow window = DrawingWindow(WIDTH, HEIGHT, false);
//
//    SDL_Event event;
//    while (true) {
//        if (window.pollForInputEvents(event)) handleEvent(event, window);
//
//        if (event.type == SDL_KEYDOWN) {
//            if (event.key.keysym.sym == SDLK_f) {
//                Colour random_colour = Colour(rand() % 256, rand() % 256, rand() % 256);
//
//                CanvasPoint random_v0 = CanvasPoint(rand() % WIDTH, rand() % HEIGHT, HEIGHT);
//                CanvasPoint random_v1 = CanvasPoint(rand() % WIDTH, rand() % HEIGHT, HEIGHT);
//                CanvasPoint random_v2 = CanvasPoint(rand() % WIDTH, rand() % HEIGHT, HEIGHT);
//
//                CanvasTriangle random_triangle = CanvasTriangle(random_v0, random_v1, random_v2);
//
//                drawFilledTriangle(window, random_triangle, random_colour);
//                drawStrokedTriangle(window, random_triangle, Colour(255, 255, 255));
//            }
//        }
//        window.renderFrame();
//    }
//}

// draw textured triangle
//int main(int argc, char *argv[]) {
//
//    DrawingWindow window = DrawingWindow(WIDTH, HEIGHT, false);
//
//    SDL_Event event;
//    while (true) {
//        if (window.pollForInputEvents(event)) handleEvent(event, window);
//
//
//
//        CanvasPoint canvas_v0 = CanvasPoint(160, 10);
//        canvas_v0.texturePoint = TexturePoint(195, 5);
//
//        CanvasPoint canvas_v1 = CanvasPoint(300, 230);
//        canvas_v1.texturePoint = TexturePoint(395, 380);
//
//        CanvasPoint canvas_v2 = CanvasPoint(10, 150);
//        canvas_v2.texturePoint = TexturePoint(65, 330);
//
//        CanvasTriangle canvas_triangle = CanvasTriangle(canvas_v0, canvas_v1, canvas_v2);
//
//        drawTexturedTriangle(window, canvas_triangle, "texture.ppm");
//        drawStrokedTriangle(window, canvas_triangle, Colour(255, 255, 255));
//
//
//        window.renderFrame();
//    }
//}
