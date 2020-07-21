#include <cstdlib>
#include <iostream>

#include "color.h"
#include "hittable_list.h"
#include "rtweekend.h"
#include "sphere.h"

color ray_color(const ray& r, const hittable& world) {
  hit_record rec;
  if (world.hit(r, 0, infinity, rec)) {
    return 0.5 * (rec.normal + color(1, 1, 1));
  }
  vec3 unit_direction = unit_vector(r.direction());
  auto t = 0.5 * (unit_direction.y() + 1.0);
  return (1.0 - t) * color(1.0, 1.0, 1.0) + t * color(0.5, 0.7, 1.0);
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

  // world
  hittable_list world;
  world.add(make_shared<sphere>(point3(0, 0, -1), 0.5));
  world.add(make_shared<sphere>(point3(0, -100.5, -1), 100));

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
      color pixel_color = ray_color(r, world);
      write_color(std::cout, pixel_color);
    }
  }

  std::cerr << "\nDone." << std::endl;

  return 0;
}
