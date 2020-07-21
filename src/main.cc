#include <cstdlib>
#include <iostream>

#include "camera.h"
#include "color.h"
#include "hittable_list.h"
#include "rtweekend.h"
#include "sphere.h"

color ray_color(const ray& r, const hittable& world, int depth) {
  if (depth <= 0) {
    return color(0, 0, 0);
  }

  hit_record rec;
  if (world.hit(r, 0.001, infinity, rec)) {
    point3 target = rec.p + rec.normal + random_in_hemisphere(rec.normal);
    return 0.5 * ray_color(ray(rec.p, target - rec.p), world, depth - 1);
  }

  vec3 unit_direction = unit_vector(r.direction());
  auto t = 0.5 * (unit_direction.y() + 1.0);
  return (1.0 - t) * color(1.0, 1.0, 1.0) + t * color(0.5, 0.7, 1.0);
}

int main(int argc, char const* argv[]) {
  if (argc < 3) {
    std::cerr << "Usage: " << argv[0] << " <width>"
              << " <height>"
              << " <samples>" << std::endl;
    return 1;
  }

  // image dimensions
  const int width = atoi(argv[1]);
  const int height = atoi(argv[2]);
  const int samples = atoi(argv[3]);
  const auto aspect_ratio = double(width) / double(height);
  const int max_depth = 50;

  // world
  hittable_list world;
  world.add(make_shared<sphere>(point3(0, 0, -1), 0.5));
  world.add(make_shared<sphere>(point3(0, -100.5, -1), 100));

  // camera
  camera cam(aspect_ratio);

  std::cout << "P3\n" << width << ' ' << height << "\n255\n";

  for (int j = height - 1; j >= 0; --j) {
    std::cerr << "\rScanlines remaining: " << j << ' ' << std::flush;
    for (int i = 0; i < width; ++i) {
      color pixel_color(0, 0, 0);
      for (int s = 0; s < samples; ++s) {
        auto u = (i + random_double()) / (width - 1);
        auto v = (j + random_double()) / (height - 1);
        ray r = cam.get_ray(u, v);
        pixel_color += ray_color(r, world, max_depth);
      }
      write_color(std::cout, pixel_color, samples);
    }
  }

  std::cerr << "\nDone." << std::endl;

  return 0;
}
