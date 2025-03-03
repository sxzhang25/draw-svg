#include "software_renderer.h"

#include <cmath>
#include <vector>
#include <iostream>
#include <algorithm>

#include "triangulation.h"

using namespace std;

namespace CS248 {


// Implements SoftwareRenderer //

// fill a sample location with color
void SoftwareRendererImp::fill_sample(int sx, int sy, const Color &color) {
  // Task 2: implement this function
  // check bounds
  if (sx < 0 || sx >= sample_buffer_width)
    return;
  if (sy < 0 || sy >= sample_buffer_height)
    return;

  Color pixel_color;
  float inv255 = 1.0 / 255.0;
  pixel_color.r = sample_buffer[4 * (sx + sy * sample_buffer_width)] * inv255;
  pixel_color.g = sample_buffer[4 * (sx + sy * sample_buffer_width) + 1] * inv255;
  pixel_color.b = sample_buffer[4 * (sx + sy * sample_buffer_width) + 2] * inv255;
  pixel_color.a = sample_buffer[4 * (sx + sy * sample_buffer_width) + 3] * inv255;

  // to change after task 5 is implemented
  // pixel_color = ref->alpha_blending_helper(pixel_color, color);
  pixel_color = this->alpha_blending(pixel_color, color);

  // std::cout << "blended pixel_color, " << pixel_color.r << ", " << pixel_color.g << ", " << pixel_color.b << ", " << pixel_color.a << std::endl;

  sample_buffer[4 * (sx + sy * sample_buffer_width)] = (unsigned char)(pixel_color.r * 255);
  sample_buffer[4 * (sx + sy * sample_buffer_width) + 1] = (unsigned char)(pixel_color.g * 255);
  sample_buffer[4 * (sx + sy * sample_buffer_width) + 2] = (unsigned char)(pixel_color.b * 255);
  sample_buffer[4 * (sx + sy * sample_buffer_width) + 3] = (unsigned char)(pixel_color.a * 255);
}

// fill samples in the entire pixel specified by pixel coordinates
void SoftwareRendererImp::fill_pixel(int x, int y, const Color &color) {
  // Task 2: Re-implement this function
  for (int i = 0; i < sample_rate; i++) {
    for (int j = 0; j < sample_rate; j++){
      fill_sample(sample_rate * x + i, sample_rate * y + j, color);
    }
  }

  // // check bounds
	// if (x < 0 || x >= width) return;
	// if (y < 0 || y >= height) return;

	// Color pixel_color;
	// float inv255 = 1.0 / 255.0;
	// pixel_color.r = pixel_buffer[4 * (x + y * width)] * inv255;
	// pixel_color.g = pixel_buffer[4 * (x + y * width) + 1] * inv255;
	// pixel_color.b = pixel_buffer[4 * (x + y * width) + 2] * inv255;
	// pixel_color.a = pixel_buffer[4 * (x + y * width) + 3] * inv255;

	// pixel_color = ref->alpha_blending_helper(pixel_color, color);

	// pixel_buffer[4 * (x + y * width)] = (uint8_t)(pixel_color.r * 255);
	// pixel_buffer[4 * (x + y * width) + 1] = (uint8_t)(pixel_color.g * 255);
	// pixel_buffer[4 * (x + y * width) + 2] = (uint8_t)(pixel_color.b * 255);
	// pixel_buffer[4 * (x + y * width) + 3] = (uint8_t)(pixel_color.a * 255);

}

void SoftwareRendererImp::draw_svg( SVG& svg ) {

  // set top level transformation
  transformation = canvas_to_screen;

  // canvas outline
  Vector2D a = transform(Vector2D(0, 0)); a.x--; a.y--;
  Vector2D b = transform(Vector2D(svg.width, 0)); b.x++; b.y--;
  Vector2D c = transform(Vector2D(0, svg.height)); c.x--; c.y++;
  Vector2D d = transform(Vector2D(svg.width, svg.height)); d.x++; d.y++;

  svg_bbox_top_left = Vector2D(a.x+1, a.y+1);
  svg_bbox_bottom_right = Vector2D(d.x-1, d.y-1);

  // clear sample buffer
  std::fill(this->sample_buffer.begin(), this->sample_buffer.end(), 255);

  // draw all elements
  for (size_t i = 0; i < svg.elements.size(); ++i) {
    draw_element(svg.elements[i]);
  }

  // draw canvas outline
  rasterize_line(a.x, a.y, b.x, b.y, Color::Black);
  rasterize_line(a.x, a.y, c.x, c.y, Color::Black);
  rasterize_line(d.x, d.y, b.x, b.y, Color::Black);
  rasterize_line(d.x, d.y, c.x, c.y, Color::Black);

  // resolve and send to pixel buffer
  resolve();

}

void SoftwareRendererImp::set_sample_rate( size_t sample_rate ) {

  // Task 2: 
  // You may want to modify this for supersampling support
  this->sample_rate = sample_rate;
  this->sample_buffer_width = width * sample_rate;
  this->sample_buffer_height = height * sample_rate;
  set_sample_buffer(sample_buffer_width, sample_buffer_height);
}

void SoftwareRendererImp::set_pixel_buffer( unsigned char* pixel_buffer,
                                             size_t width, size_t height ) {

  // Task 2: 
  // You may want to modify this for supersampling support
  this->pixel_buffer = pixel_buffer;
  this->width = width;
  this->height = height;

  this->sample_buffer_width = width * sample_rate;
  this->sample_buffer_height = height * sample_rate;
  set_sample_buffer(sample_buffer_width, sample_buffer_height);
}

// For Task 2
void SoftwareRendererImp::set_sample_buffer(size_t width, size_t height) {
  this->sample_buffer.assign(4 * width * height, 255);
  // std::fill(sample_buffer.begin(), sample_buffer.end(), 255.f);
}

void SoftwareRendererImp::draw_element( SVGElement* element ) {

	// Task 3 (part 1):
	// Modify this to implement the transformation stack
  transformation = transformation * element->transform;

	switch (element->type) {
	case POINT:
		draw_point(static_cast<Point&>(*element));
		break;
	case LINE:
		draw_line(static_cast<Line&>(*element));
		break;
	case POLYLINE:
		draw_polyline(static_cast<Polyline&>(*element));
		break;
	case RECT:
		draw_rect(static_cast<Rect&>(*element));
		break;
	case POLYGON:
		draw_polygon(static_cast<Polygon&>(*element));
		break;
	case ELLIPSE:
		draw_ellipse(static_cast<Ellipse&>(*element));
		break;
	case IMAGE:
		draw_image(static_cast<Image&>(*element));
		break;
	case GROUP:
		draw_group(static_cast<Group&>(*element));
		break;
	default:
		break;
	}

  transformation = transformation * element->transform.inv();

}


// Primitive Drawing //

void SoftwareRendererImp::draw_point( Point& point ) {

  Vector2D p = transform(point.position);
  rasterize_point( p.x, p.y, point.style.fillColor );

}

void SoftwareRendererImp::draw_line( Line& line ) { 

  Vector2D p0 = transform(line.from);
  Vector2D p1 = transform(line.to);
  rasterize_line( p0.x, p0.y, p1.x, p1.y, line.style.strokeColor );

}

void SoftwareRendererImp::draw_polyline( Polyline& polyline ) {

  Color c = polyline.style.strokeColor;

  if( c.a != 0 ) {
    int nPoints = polyline.points.size();
    for( int i = 0; i < nPoints - 1; i++ ) {
      Vector2D p0 = transform(polyline.points[(i+0) % nPoints]);
      Vector2D p1 = transform(polyline.points[(i+1) % nPoints]);
      rasterize_line( p0.x, p0.y, p1.x, p1.y, c );
    }
  }
}

void SoftwareRendererImp::draw_rect( Rect& rect ) {

  Color c;
  
  // draw as two triangles
  float x = rect.position.x;
  float y = rect.position.y;
  float w = rect.dimension.x;
  float h = rect.dimension.y;

  Vector2D p0 = transform(Vector2D(   x   ,   y   ));
  Vector2D p1 = transform(Vector2D( x + w ,   y   ));
  Vector2D p2 = transform(Vector2D(   x   , y + h ));
  Vector2D p3 = transform(Vector2D( x + w , y + h ));
  
  // draw fill
  c = rect.style.fillColor;
  if (c.a != 0 ) {
    rasterize_triangle( p0.x, p0.y, p1.x, p1.y, p2.x, p2.y, c );
    rasterize_triangle( p2.x, p2.y, p1.x, p1.y, p3.x, p3.y, c );
  }

  // draw outline
  c = rect.style.strokeColor;
  if( c.a != 0 ) {
    rasterize_line( p0.x, p0.y, p1.x, p1.y, c );
    rasterize_line( p1.x, p1.y, p3.x, p3.y, c );
    rasterize_line( p3.x, p3.y, p2.x, p2.y, c );
    rasterize_line( p2.x, p2.y, p0.x, p0.y, c );
  }

}

void SoftwareRendererImp::draw_polygon( Polygon& polygon ) {

  Color c;

  // draw fill
  c = polygon.style.fillColor;
  if( c.a != 0 ) {

    // triangulate
    vector<Vector2D> triangles;
    triangulate( polygon, triangles );

    // draw as triangles
    for (size_t i = 0; i < triangles.size(); i += 3) {
      Vector2D p0 = transform(triangles[i + 0]);
      Vector2D p1 = transform(triangles[i + 1]);
      Vector2D p2 = transform(triangles[i + 2]);
      rasterize_triangle( p0.x, p0.y, p1.x, p1.y, p2.x, p2.y, c );
    }
  }

  // draw outline
  c = polygon.style.strokeColor;
  if( c.a != 0 ) {
    int nPoints = polygon.points.size();
    for( int i = 0; i < nPoints; i++ ) {
      Vector2D p0 = transform(polygon.points[(i+0) % nPoints]);
      Vector2D p1 = transform(polygon.points[(i+1) % nPoints]);
      rasterize_line( p0.x, p0.y, p1.x, p1.y, c );
    }
  }
}

void SoftwareRendererImp::draw_ellipse( Ellipse& ellipse ) {

  // Advanced Task
  // Implement ellipse rasterization

}

void SoftwareRendererImp::draw_image( Image& image ) {

  // Advanced Task
  // Render image element with rotation

  Vector2D p0 = transform(image.position);
  Vector2D p1 = transform(image.position + image.dimension);

  rasterize_image( p0.x, p0.y, p1.x, p1.y, image.tex );
}

void SoftwareRendererImp::draw_group( Group& group ) {

  for ( size_t i = 0; i < group.elements.size(); ++i ) {
    draw_element(group.elements[i]);
  }

}

// Rasterization //

// The input arguments in the rasterization functions 
// below are all defined in screen space coordinates

void SoftwareRendererImp::rasterize_point( float x, float y, Color color ) {

  // fill in the nearest pixel
  int sx = (int)floor(x);
  int sy = (int)floor(y);

  // check bounds
  if (sx < 0 || sx >= width) return;
  if (sy < 0 || sy >= height) return;

  // fill sample - NOT doing alpha blending!
  // TODO: Call fill_pixel here to run alpha blending
  // pixel_buffer[4 * (sx + sy * width)] = (uint8_t)(color.r * 255);
  // pixel_buffer[4 * (sx + sy * width) + 1] = (uint8_t)(color.g * 255);
  // pixel_buffer[4 * (sx + sy * width) + 2] = (uint8_t)(color.b * 255);
  // pixel_buffer[4 * (sx + sy * width) + 3] = (uint8_t)(color.a * 255);
  fill_pixel(sx, sy, color);
}

// void SoftwareRendererImp::swap(float* a, float* b) {
//   float temp = *a;
//   *a = *b;
//   *b = temp;
// }

// float SoftwareRendererImp::absolute(float x) {
//   if (x < 0) return -x;
//   else return x;
// }

int SoftwareRendererImp::integer_part(float x) {
  return floor(x);
}

int SoftwareRendererImp::round_number(float x) {
  return integer_part(x + 0.5f);
}

float SoftwareRendererImp::fractional_part(float x) {
  return x - integer_part(x);
}

float SoftwareRendererImp::reverse_fractional_part(float x) {
  return 1.0f - fractional_part(x);
}

Color SoftwareRendererImp::apply_brightness(Color color, float brightness)
{
  // with premultiplied alpha
  color.r = color.r * brightness;
  color.g = color.g * brightness;
  color.b = color.b * brightness;
  color.a = brightness;
  return color;
}

void SoftwareRendererImp::rasterize_line( float x0, float y0,
                                          float x1, float y1,
                                          Color color) {

  // Task 0: 
  // Implement Bresenham's algorithm (delete the line below and implement your own)
  // ref->rasterize_line_helper(x0, y0, x1, y1, width, height, color, this);

  // float eps = 0;
  // float x;
  // float y;
  // float m = (y1 - y0) / (x1 - x0);

  // // Special case: vertical line.
  // if (x0 == x1) {
  //   for (int y = (int)floor(min(y0, y1)); y <= (int)floor(max(y0, y1)); y++) {
  //     rasterize_point(x0, y, color);
  //   }
  // } else {
  //   if (0 <= m && m <= 1) { // Case: 0 <= m <= 1.
  //     if (x0 > x1) {
  //       rasterize_line(x1, y1, x0, y0, color);
  //     } else {
  //       y = y0;
  //       for (int dx = 0; dx <= (int)floor(x1 - x0); dx++) {
  //         x = x0 + dx;
  //         rasterize_point(x, y, color);
  //         if (eps + m < 0.5) {
  //           eps += m;
  //         } else {
  //           y += 1;
  //           eps += m - 1;
  //         }
  //       }
  //     }
  //   } else if (m > 1) { // Case: m > 1.
  //     if (y0 > y1) {
  //       rasterize_line(x1, y1, x0, y0, color);
  //     } else {
  //       x = x0;
  //       for (int dy = 0; dy <= (int)floor(y1 - y0); dy++) {
  //         y = y0 + dy;
  //         rasterize_point(x, y, color);
  //         if (eps + 1 / m < 0.5) {
  //           eps += 1 / m;
  //         } else {
  //           x += 1;
  //           eps += 1 / m - 1;
  //         }
  //       }
  //     }
  //   } 
  //   else if (m < 0 && m >= -1) { // Case: 0 > m >= -1.
  //     if (x1 > x0) {
  //       rasterize_line(x1, y1, x0, y0, color);
  //     } else {
  //       y = y0;
  //       for (int dx = 0; dx >= (int)floor(x1 - x0); dx--) {
  //         x = x0 + dx;
  //         rasterize_point(x, y, color);
  //         if (eps + m > -0.5) {
  //           eps += m;
  //         } else {
  //           y += 1;
  //           eps += m + 1;
  //         }
  //       }
  //     }
  //   } else { // Case: m < -1.
  //     if (y1 > y0) {
  //       rasterize_line(x1, y1, x0, y0, color);
  //     } else {
  //       x = x0;
  //       for (int dy = 0; dy >= (int)floor(y1 - y0); dy--) {
  //         y = y0 + dy;
  //         rasterize_point(x, y, color);
  //         if (eps + 1 / m > -0.5) {
  //           eps += 1 / m;
  //         } else {
  //           x += 1;
  //           eps += 1 / m + 1;
  //         }
  //       }
  //     }
  //   }
  // }

  // Advanced Task
  // Drawing Smooth Lines with Line Width

  // Xiaolin Wu's line algorithm

  // accounting for the fact that pixel centers are at (x + 0.5, y + 0.5)
  x0 = x0 + 0.5f;
  x1 = x1 + 0.5f;
  y0 = y0 + 0.5f;
  y1 = y1 + 0.5f;

  int steep = std::fabs(y1 - y0) > std::fabs(x1 - x0);

  if (steep) {
    std::swap(x0, y0);
    std::swap(x1, y1);
  }

  if (x0 > x1) {
    std::swap(x0, x1);
    std::swap(y0, y1);
  }

  // slope
  float dx = x1 - x0;
  float dy = y1 - y0;
  float gradient = dy / dx;
  if (dx == 0.0) {
    gradient = 1.0;
  }

  // handle first endpoint
  float x_end = static_cast<float> (round_number(x0));
  float y_end = y0 + gradient * (x_end - x0);
  float x_gap = reverse_fractional_part(x0 + 0.5f);
  int x_px_l1 = static_cast<int>(x_end) - 1;
  int y_px_l1 = integer_part(y_end) - 1;
  if (steep) {
    fill_pixel(y_px_l1, x_px_l1, apply_brightness(color, reverse_fractional_part(y_end) * x_gap));
    fill_pixel(y_px_l1 + 1, x_px_l1, apply_brightness(color, fractional_part(y_end) * x_gap));
  } else {
    fill_pixel(x_px_l1, y_px_l1, apply_brightness(color, reverse_fractional_part(y_end) * x_gap));
    fill_pixel(x_px_l1, y_px_l1 + 1, apply_brightness(color, fractional_part(y_end) * x_gap));
  }
  float intery = y_end + gradient - 1;

  // handle second endpoint
  float x_end_2 = static_cast<float>(round_number(x1));
  float y_end_2 = y1 + gradient * (x_end_2 - x1);
  float x_gap_2 = fractional_part(x1 + 0.5f);
  int x_px_l2 = static_cast<int>(x_end_2) - 1;
  int y_px_l2 = integer_part(y_end_2) - 1;
  if (steep) {
    fill_pixel(y_px_l2, x_px_l2, apply_brightness(color, reverse_fractional_part(y_end_2) * x_gap_2));
    fill_pixel(y_px_l2 + 1, x_px_l2, apply_brightness(color, fractional_part(y_end_2) * x_gap_2));
  } else {
    fill_pixel(x_px_l2, y_px_l2, apply_brightness(color, reverse_fractional_part(y_end_2) * x_gap_2));
    fill_pixel(x_px_l2, y_px_l2 + 1, apply_brightness(color, fractional_part(y_end_2) * x_gap_2));
  }

  // main loop
  if (steep) {
    for (int x = x_px_l1 + 1; x <= x_px_l2 - 1; x++) {
      fill_pixel(integer_part(intery), x, apply_brightness(color, reverse_fractional_part(intery)));
      fill_pixel(integer_part(intery) + 1, x, apply_brightness(color, fractional_part(intery)));
      intery += gradient;
    }
  } else {
    for (int x = x_px_l1 + 1; x <= x_px_l2 - 1; x++) {
      fill_pixel(x, integer_part(intery), apply_brightness(color, reverse_fractional_part(intery)));
      fill_pixel(x, integer_part(intery) + 1, apply_brightness(color, fractional_part(intery)));
      intery += gradient;
    }
  }
}

void SoftwareRendererImp::rasterize_triangle( float x0, float y0,
                                              float x1, float y1,
                                              float x2, float y2,
                                              Color color ) {
  // Task 1: 
  // Implement triangle rasterization

  // CCW function
  auto ccw = [](float x, float y, float ax, float ay, float bx, float by) {
    float res = -(x - ax) * (by - ay) + (y - ay) * (bx - ax);
    if (res > 0) return 1;
    else if (res < 0) return -1;
    else return 0;
  };

  // Point inside triangle function
  auto inside = [=](float x, float y) {
    int ccw1 = ccw(x, y, x0, y0, x1, y1);
    int ccw2 = ccw(x, y, x1, y1, x2, y2);
    int ccw3 = ccw(x, y, x2, y2, x0, y0);
    return (ccw1 * ccw2 >= 0) && (ccw2 * ccw3 >= 0) && (ccw3 * ccw1 >= 0);
  };

  // Check all points within bounding box of triangle.
  float min_x = min(x0, min(x1, x2));
  float min_y = min(y0, min(y1, y2));
  float max_x = max(x0, max(x1, x2));
  float max_y = max(y0, max(y1, y2));

  for (int x = (int)round(min_x); x <= (int)round(max_x); x++) {
    for (int y = (int)round(min_y); y <= (int)round(max_y); y++) {
      // if (inside(x + 0.5, y + 0.5)) {
      //   rasterize_point(x + 0.5, y + 0.5, color);
      // }

      // modification to support supersampling
      for (int i = 0; i < sample_rate; i++) {
        for (int j = 0; j < sample_rate; j++) {
          float sample_rate_step_size = 1.0 / (2 * sample_rate);

          float x_s = x + (2 * i + 1) * sample_rate_step_size;
          float y_s = y + (2 * j + 1) * sample_rate_step_size;        
          if (inside(x_s, y_s)) {
            fill_sample(sample_rate * x + i, sample_rate * y + j, color);
          }
        }
      }
    }
  }

  // Advanced Task
  // Implementing Triangle Edge Rules

}

void SoftwareRendererImp::rasterize_image( float x0, float y0,
                                           float x1, float y1,
                                           Texture& tex ) {
  // Task 4: 
  // Implement image rasterization
  for (int x = ceil(x0); x < floor(x1); ++x) {
    for (int y = ceil(y0); y < floor(y1); ++y) {
      float u = (float)(x + 0.5 - x0) / (x1 - x0);
      float v = (float)(y + 0.5 - y0) / (y1 - y0);
      // Color sample_xy = sampler->sample_nearest(tex, u, v, 0);
      Color sample_xy = sampler->sample_bilinear(tex, u, v, 0);
      fill_pixel(x, y, sample_xy);
    }
  }
}

// resolve samples to pixel buffer
void SoftwareRendererImp::resolve( void ) {

  // Task 2: 
  // Implement supersampling
  // You may also need to modify other functions marked with "Task 2".

  for (int i = 0; i < width; i++) {
    for (int j = 0; j < height; j++) {
      // box filter
      int acc_r = 0;
      int acc_g = 0;
      int acc_b = 0;
      int acc_a = 0;

      for (int s_i = 0; s_i < sample_rate; s_i++) {
        for (int s_j = 0; s_j < sample_rate; s_j++) {
          int s_x = sample_rate * i + s_i;
          int s_y = sample_rate * j + s_j;

          acc_r += sample_buffer[4 * (s_x + s_y * sample_buffer_width)];
          acc_g += sample_buffer[4 * (s_x + s_y * sample_buffer_width) + 1];
          acc_b += sample_buffer[4 * (s_x + s_y * sample_buffer_width) + 2];
          acc_a += sample_buffer[4 * (s_x + s_y * sample_buffer_width) + 3];
        }
      }

      size_t sample_rate_sq = sample_rate * sample_rate;
      pixel_buffer[4 * (i + j * width)] = acc_r / sample_rate_sq;
      pixel_buffer[4 * (i + j * width) + 1] = acc_g / sample_rate_sq;
      pixel_buffer[4 * (i + j * width) + 2] = acc_b / sample_rate_sq;
      pixel_buffer[4 * (i + j * width) + 3] = acc_a / sample_rate_sq;
    }
  }

  return;

}

Color SoftwareRendererImp::alpha_blending(Color pixel_color, Color color)
{
  // Task 5
  // Implement alpha compositing
  pixel_color.a = 1 - (1 - pixel_color.a) * (1 - color.a);
  pixel_color.r = (1 - color.a) * pixel_color.r + color.r;
  pixel_color.g = (1 - color.a) * pixel_color.g + color.g;
  pixel_color.b = (1 - color.a) * pixel_color.b + color.b;

  return pixel_color;
}


} // namespace CS248
