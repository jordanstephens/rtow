#include <cstdlib>
#include <iostream>

#include "color.h"
#include "ray.h"
#include "vec3.h"

double hit_sphere(const point3& center, double radius, const ray& r) {
  vec3 oc = r.origin() - center;
  auto a = dot(r.direction(), r.direction());
  auto b = 2.0 * dot(oc, r.direction());
  auto c = dot(oc, oc) - radius * radius;
  auto discriminant = b * b - 4 * a * c;
  if (discriminant < 0) {
    return -1.0;
  } else {
    return (-b - sqrt(discriminant)) / (2.0 * a);
  }
}

color ray_color(const ray& r) {
  auto t = hit_sphere(point3(0, 0, -1), 0.5, r);
  if (t > 0.0) {
    vec3 N = unit_vector(r.at(t) - vec3(0, 0, -1));
    return 0.5 * color(N.x() + 1, N.y() + 1, N.z() + 1);
  }
  vec3 d_unit = unit_vector(r.direction());
  t = 0.5 * (d_unit.y() + 1.0);
  color white = color(1.0, 1.0, 1.0);
  color blue = color(0.5, 0.7, 1.0);
  return (1.0 - t) * white + t * blue;
}

int main(int argc, char const* argv[]) {
  if (argc < 3) {
    std::cerr << "Usage: " << argv[0] << " <width>"
              << " <height>" << std::endl;
    return 1;
  }

  // image dimensions
  const int width = atoi(argv[1]);
  const int height = atoi(argv[2]);
  const auto aspect_ratio = double(width) / double(height);
  auto origin = point3(0, 0, 0);

  // camera
  auto viewport_height = 2.0;
  auto viewport_width = aspect_ratio * viewport_height;
  auto focal_length = 1.0;

  auto horizontal = vec3(viewport_width, 0, 0);
  auto vertical = vec3(0, viewport_height, 0);
  auto lower_left_corner =
      origin - horizontal / 2 - vertical / 2 - vec3(0, 0, focal_length);

  std::cout << "P3\n" << width << ' ' << height << "\n255\n";

  for (int j = height - 1; j >= 0; --j) {
    std::cerr << "\rScanlines remaining: " << j << ' ' << std::flush;
    for (int i = 0; i < width; ++i) {
      auto x = double(i) / (width - 1);
      auto y = double(j) / (height - 1);
      ray r(origin, lower_left_corner + x * horizontal + y * vertical - origin);
      color pixel_color = ray_color(r);
      write_color(std::cout, pixel_color);
    }
  }

  std::cerr << "\nDone." << std::endl;

  return 0;
}
