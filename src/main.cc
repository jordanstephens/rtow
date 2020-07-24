#include <omp.h>

#include <chrono>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

#include "camera.h"
#include "color.h"
#include "hittable_list.h"
#include "material.h"
#include "pingpong.h"
#include "rtweekend.h"
#include "sphere.h"

const int MAX_DEPTH = 32;
const int MAX_RETRIES = 10;
const int WAIT_MS = 10;

color ray_color(const ray& r, const hittable& world, int depth) {
  if (depth <= 0) {
    return color(0, 0, 0);
  }

  hit_record rec;
  if (world.hit(r, 0.001, infinity, rec)) {
    ray scattered;
    color attenuation;
    if (rec.mat_ptr->scatter(r, rec, attenuation, scattered)) {
      return attenuation * ray_color(scattered, world, depth - 1);
    }
    return color(0, 0, 0);
  }

  vec3 unit_direction = unit_vector(r.direction());
  auto t = 0.5 * (unit_direction.y() + 1.0);
  return (1.0 - t) * color(1.0, 1.0, 1.0) + t * color(0.5, 0.7, 1.0);
}

hittable_list random_scene() {
  hittable_list world;

  auto ground_material = make_shared<lambertian>(color(0.5, 0.5, 0.5));
  world.add(make_shared<sphere>(point3(0, -1000, 0), 1000, ground_material));

  for (int a = -11; a < 11; a++) {
    for (int b = -11; b < 11; b++) {
      auto choose_mat = random_double();
      point3 center(a + 0.9 * random_double(), 0.2, b + 0.9 * random_double());

      if ((center - point3(4, 0.2, 0)).length() > 0.9) {
        shared_ptr<material> sphere_material;

        if (choose_mat < 0.8) {
          // diffuse
          auto albedo = color::random() * color::random();
          sphere_material = make_shared<lambertian>(albedo);
          world.add(make_shared<sphere>(center, 0.2, sphere_material));
        } else if (choose_mat < 0.95) {
          // metal
          auto albedo = color::random(0.5, 1);
          auto fuzz = random_double(0, 0.5);
          sphere_material = make_shared<metal>(albedo, fuzz);
          world.add(make_shared<sphere>(center, 0.2, sphere_material));
        } else {
          // glass
          sphere_material = make_shared<dielectric>(1.5);
          world.add(make_shared<sphere>(center, 0.2, sphere_material));
        }
      }
    }
  }

  auto material1 = make_shared<dielectric>(1.5);
  world.add(make_shared<sphere>(point3(0, 1, 0), 1.0, material1));

  auto material2 = make_shared<lambertian>(color(0.4, 0.2, 0.1));
  world.add(make_shared<sphere>(point3(-4, 1, 0), 1.0, material2));

  auto material3 = make_shared<metal>(color(0.7, 0.6, 0.5), 0.0);
  world.add(make_shared<sphere>(point3(4, 1, 0), 1.0, material3));

  return world;
}

void flush_page(std::ostream& out, std::vector<color> buffer) {
  std::stringstream page;
  for (size_t i = 0; i < buffer.size(); i++) {
    color pixel = buffer.at(i);
    page << static_cast<int>(256 * pixel.x()) << ' '
         << static_cast<int>(256 * pixel.y()) << ' '
         << static_cast<int>(256 * pixel.z()) << '\n';
  }
  out << page.str() << std::flush;
}

void render(int width, int height, camera cam, hittable_list world, int samples,
            int max_depth) {
  std::cout << "P3\n" << width << ' ' << height << "\n255\n";

  pingpong<color> buffer(width);

  for (int j = height - 1; j >= 0; j -= 2) {
    std::cerr << "\rScanlines remaining: " << j + 1 << ' ' << std::flush;

#pragma omp parallel shared(j, buffer)
    {
#pragma omp for schedule(dynamic)
      for (int i = 0; i < width * 2; ++i) {
        color pixel_color(0, 0, 0);
        auto x = i >= width ? i - width : i;
        auto y = i >= width ? j - 1 : j;
        auto page = int(i >= width);
        for (int s = 0; s < samples; ++s) {
          auto u = (x + random_double()) / (width - 1);
          auto v = (y + random_double()) / (height - 1);
          ray r = cam.get_ray(u, v);
          pixel_color += ray_color(r, world, max_depth);
        }
        bool sealed = false;
        for (int n = 0; n < MAX_RETRIES; n++) {
          try {
            sealed = buffer.set(page, x, correct_color(pixel_color, samples));
            break;
          } catch (const std::exception& e) {
            std::this_thread::sleep_for(std::chrono::milliseconds(WAIT_MS));
            if (n == MAX_RETRIES - 1) {
              std::cerr << "\r" << e.what() << std::endl;
              throw std::runtime_error("Maximum retries exceeded");
            }
          }
        }
        if (sealed) {
          flush_page(std::cout, buffer.get_page(page));
          buffer.clear_page(page);
        }
      }
    }
  }

  std::cerr << "\rDone." << std::endl;
}

int main(int argc, char const* argv[]) {
  if (argc < 3) {
    std::cerr << "Usage: " << argv[0] << " <width>"
              << " <height>"
              << " <samples>"
              << " <threads>" << std::endl;
    return 1;
  }

  // image dimensions
  const int width = atoi(argv[1]);
  const int height = atoi(argv[2]);
  const int samples = argc > 3 ? atoi(argv[3]) : 128;
  const int threads = argc > 4 ? atoi(argv[4]) : 1;
  const auto aspect_ratio = double(width) / double(height);

  omp_set_num_threads(threads);
  // world
  auto world = random_scene();

  // camera
  point3 lookfrom(13, 2, 3);
  point3 lookat(0, 0, 0);
  vec3 vup(0, 1, 0);
  auto dist_to_focus = 10.0;
  auto aperture = 0.1;

  camera cam(lookfrom, lookat, vup, 20, aspect_ratio, aperture, dist_to_focus);

  try {
    render(width, height, cam, world, samples, MAX_DEPTH);
  } catch (const std::exception& e) {
    std::cerr << "\n" << e.what() << std::endl;
  }

  return 0;
}
