#include "rasterizer.h"

using namespace std;

namespace CGL {

  RasterizerImp::RasterizerImp(PixelSampleMethod psm, LevelSampleMethod lsm,
    size_t width, size_t height,
    unsigned int sample_rate) {
    this->psm = psm;
    this->lsm = lsm;
    this->width = width;
    this->height = height;
    this->sample_rate = sample_rate;

    sample_buffer.resize(width * height * sample_rate, Color::White);
  }
 

  // Used by rasterize_point and rasterize_line
  void RasterizerImp::fill_pixel(size_t x, size_t y, Color c) {
    // TODO: Task 2: You might need to this function to fix points and lines (such as the black rectangle border in test4.svg)
    // NOTE: You are not required to implement proper supersampling for points and lines
    // fill in the nearest pixel
    if (x < 0 || x >= width * sqrt(sample_rate)) return;
    if (y < 0 || y >= height * sqrt(sample_rate)) return;
    sample_buffer[y * width * sqrt(sample_rate) + x] = c;
  }

  // Rasterize a point: simple example to help you start familiarizing
  // yourself with the starter code.
  //
  void RasterizerImp::rasterize_point(float x, float y, Color color) {
    // fill in the nearest pixel
    int sx = (int)floor(x);
    int sy = (int)floor(y);

    // check bounds
    if (sx < 0 || sx >= width) return;
    if (sy < 0 || sy >= height) return;

    fill_pixel(sx, sy, color);
    return;
  }

  // Rasterize a line.
  void RasterizerImp::rasterize_line(float x0, float y0,
    float x1, float y1,
    Color color) {
    if (x0 > x1) {
      swap(x0, x1); swap(y0, y1);
    }

    float pt[] = { x0,y0 };
    float m = (y1 - y0) / (x1 - x0);
    float dpt[] = { 1,m };
    int steep = abs(m) > 1;
    if (steep) {
      dpt[0] = x1 == x0 ? 0 : 1 / abs(m);
      dpt[1] = x1 == x0 ? (y1 - y0) / abs(y1 - y0) : m / abs(m);
    }

    while (floor(pt[0]) <= floor(x1) && abs(pt[1] - y0) <= abs(y1 - y0)) {
      rasterize_point(pt[0], pt[1], color);
      pt[0] += dpt[0]; pt[1] += dpt[1];
    }
  }

  // Rasterize a triangle.
  void RasterizerImp::rasterize_triangle(float x0, float y0,
    float x1, float y1,
    float x2, float y2,
    Color color) {
    // TODO: Task 1: Implement basic triangle rasterization here, no supersampling
    // Step0: Using sample rate to perform a coordinate transformation
    x0 = x0 * sqrt(sample_rate);
    y0 = y0 * sqrt(sample_rate);
    x1 = x1 * sqrt(sample_rate);
    y1 = y1 * sqrt(sample_rate);
    x2 = x2 * sqrt(sample_rate);
    y2 = y2 * sqrt(sample_rate);
    // Step 1: Determine the bounding box.
    int x_min = min(x0, min(x1, x2));
    int x_max = max(x0, max(x1, x2));
    int y_min = min(y0, min(y1, y2));
    int y_max = max(y0, max(y1, y2));
    

    // Step 2: Iterate through the bounding box and check if the point is inside the triangle by three line tests
    for (int x = x_min; x <= x_max; x++) {
      for (int y = y_min; y <= y_max; y++) {
        x = x + 0.5;
        y = y + 0.5;
        // Check if the point is inside the triangle
        float line1 = (x - x0) * (y1 - y0) - (y - y0) * (x1 - x0);
        float line2 = (x - x1) * (y2 - y1) - (y - y1) * (x2 - x1);
        float line3 = (x - x2) * (y0 - y2) - (y - y2) * (x0 - x2);
        if ((line1 >= 0 && line2 >= 0 && line3 >= 0) || (line1 <= 0 && line2 <= 0 && line3 <= 0)) {
          fill_pixel(x, y, color);
        }
      }
    }

    // TODO: Task 2: Update to implement super-sampled rasterization
    // You may also need to update other functions that call rasterize_triangle
  }


  void RasterizerImp::rasterize_interpolated_color_triangle(float x0, float y0, Color c0,
    float x1, float y1, Color c1,
    float x2, float y2, Color c2)
  {
    // TODO: Task 4: Rasterize the triangle, calculating barycentric coordinates and using them to interpolate vertex colors across the triangle
    // Hint: You can reuse code from rasterize_triangle
    // Step0: Using sample rate to perform a coordinate transformation
    x0 = x0 * sqrt(sample_rate);
    y0 = y0 * sqrt(sample_rate);
    x1 = x1 * sqrt(sample_rate);
    y1 = y1 * sqrt(sample_rate);
    x2 = x2 * sqrt(sample_rate);
    y2 = y2 * sqrt(sample_rate);
    // Step 1: Determine the bounding box.
    int x_min = min(x0, min(x1, x2));
    int x_max = max(x0, max(x1, x2));
    int y_min = min(y0, min(y1, y2));
    int y_max = max(y0, max(y1, y2));

    for (int x = x_min; x <= x_max; x++) {
      for (int y = y_min; y <= y_max; y++) {
        x = x + 0.5;
        y = y + 0.5;
        // Check if the point is inside the triangle
        float alpha = ((y1 - y2) * (x - x2) + (x2 - x1) * (y - y2)) /
          ((y1 - y2) * (x0 - x2) + (x2 - x1) * (y0 - y2));
        float beta = ((y2 - y0) * (x - x2) + (x0 - x2) * (y - y2)) /
          ((y1 - y2) * (x0 - x2) + (x2 - x1) * (y0 - y2));
        float gamma = 1.0f - alpha - beta;
        if (alpha >= 0 && beta >= 0 && gamma >= 0) {
          Color c = alpha * c0 + beta * c1 + gamma * c2;
          fill_pixel(x, y, c);
        }
      }
    }
  }


  void RasterizerImp::rasterize_textured_triangle(float x0, float y0, float u0, float v0,
    float x1, float y1, float u1, float v1,
    float x2, float y2, float u2, float v2,
    Texture& tex)
  {
    // TODO: Task 5: Fill in the SampleParams struct and pass it to the tex.sample function.
    // TODO: Task 6: Set the correct barycentric differentials in the SampleParams struct.
    // Hint: You can reuse code from rasterize_triangle/rasterize_interpolated_color_triangle

    // Step0: Using sample rate to perform a coordinate transformation
    x0 = x0 * sqrt(sample_rate);
    y0 = y0 * sqrt(sample_rate);
    x1 = x1 * sqrt(sample_rate);
    y1 = y1 * sqrt(sample_rate);
    x2 = x2 * sqrt(sample_rate);
    y2 = y2 * sqrt(sample_rate);
    // Step 1: Determine the bounding box.
    int x_min = min(x0, min(x1, x2));
    int x_max = max(x0, max(x1, x2));
    int y_min = min(y0, min(y1, y2));
    int y_max = max(y0, max(y1, y2));

    for (int x = x_min; x <= x_max; x++) {
      for (int y = y_min; y <= y_max; y++) {
        x = x + 0.5;
        y = y + 0.5;
        // Check if the point is inside the triangle
        float alpha = ((y1 - y2) * (x - x2) + (x2 - x1) * (y - y2)) /
          ((y1 - y2) * (x0 - x2) + (x2 - x1) * (y0 - y2));
        float beta = ((y2 - y0) * (x - x2) + (x0 - x2) * (y - y2)) /
          ((y1 - y2) * (x0 - x2) + (x2 - x1) * (y0 - y2));
        float gamma = 1.0f - alpha - beta;
        if (alpha >= 0 && beta >= 0 && gamma >= 0) {
          // calclate new alpha, beta, gamma for (x+1, y) and (x, y+1)
          float alpha_x = ((y1 - y2) * (x + 1 - x2) + (x2 - x1) * (y - y2)) /
            ((y1 - y2) * (x0 - x2) + (x2 - x1) * (y0 - y2));
          float beta_x = ((y2 - y0) * (x + 1 - x2) + (x0 - x2) * (y - y2)) /
            ((y1 - y2) * (x0 - x2) + (x2 - x1) * (y0 - y2));
          float gamma_x = 1.0f - alpha_x - beta_x;
          float alpha_y = ((y1 - y2) * (x - x2) + (x2 - x1) * (y + 1 - y2)) /
            ((y1 - y2) * (x0 - x2) + (x2 - x1) * (y0 - y2));
          float beta_y = ((y2 - y0) * (x - x2) + (x0 - x2) * (y + 1 - y2)) /
            ((y1 - y2) * (x0 - x2) + (x2 - x1) * (y0 - y2));
          float gamma_y = 1.0f - alpha_y - beta_y;
          // calculate (x,y), (x+1,y) and (x,y+1) point in uv world
          Vector2D uv = alpha * Vector2D(u0, v0) + beta * Vector2D(u1, v1) + gamma * Vector2D(u2, v2);
          Vector2D uv_x = alpha_x * Vector2D(u0, v0) + beta_x * Vector2D(u1, v1) + gamma_x * Vector2D(u2, v2);
          Vector2D uv_y = alpha_y * Vector2D(u0, v0) + beta_y * Vector2D(u1, v1) + gamma_y * Vector2D(u2, v2);
          // calculate the barycentric differential Vector2D
          SampleParams sp;
          sp.p_uv = uv;
          sp.p_dx_uv = uv_x;
          sp.p_dy_uv = uv_y;
          sp.lsm = lsm;
          sp.psm = psm;
          Color c = tex.sample(sp);
          fill_pixel(x, y, c);
        }
      }
    }
  }

  void RasterizerImp::set_sample_rate(unsigned int rate) {
    // TODO: Task 2: You may want to update this function for supersampling support

    this->sample_rate = rate;


    this->sample_buffer.resize(width * height * sample_rate, Color::White);
  }


  void RasterizerImp::set_framebuffer_target(unsigned char* rgb_framebuffer,
    size_t width, size_t height)
  {
    // TODO: Task 2: You may want to update this function for supersampling support

    this->width = width;
    this->height = height;
    this->rgb_framebuffer_target = rgb_framebuffer;


    this->sample_buffer.resize(width * height, Color::White);
  }


  void RasterizerImp::clear_buffers() {
    std::fill(rgb_framebuffer_target, rgb_framebuffer_target + 3 * width * height, 255);
    std::fill(sample_buffer.begin(), sample_buffer.end(), Color::White);
  }


  // This function is called at the end of rasterizing all elements of the
  // SVG file.  If you use a supersample buffer to rasterize SVG elements
  // for antialising, you could use this call to fill the target framebuffer
  // pixels from the supersample buffer data.
  //
  void RasterizerImp::resolve_to_framebuffer() {
    // TODO: Task 2: You will likely want to update this function for supersampling support
    for (size_t y = 0; y < height; y++) {
      for (size_t x = 0; x < width; x++) {
        Color c;
        for (size_t i = 0; i < sqrt(sample_rate); i++) {
          for (size_t j = 0; j < sqrt(sample_rate); j++) {
            int y2 = y * sqrt(sample_rate) + i;
            int x2 = x * sqrt(sample_rate) + j;
            c += sample_buffer[(y2 * width * sqrt(sample_rate) + x2)];
          }
        }
        c.r /= sample_rate;
        c.g /= sample_rate;
        c.b /= sample_rate;
        rgb_framebuffer_target[3 * (y * width + x)] = (unsigned char)(c.r * 255);
        rgb_framebuffer_target[3 * (y * width + x) + 1] = (unsigned char)(c.g * 255);
        rgb_framebuffer_target[3 * (y * width + x) + 2] = (unsigned char)(c.b * 255);
      }
    }

  }

  Rasterizer::~Rasterizer() { }


}// CGL
