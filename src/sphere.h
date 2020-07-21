#ifndef SPHERE_H
#define SPHERE_H

#include "hittable.h"

class sphere : public hittable {
 public:
  sphere() {}
  sphere(point3 cen, double r) : center(cen), radius(r){};

  virtual bool hit(const ray& r, double t_min, double t_max,
                   hit_record& rec) const override;

 public:
  point3 center;
  double radius;
};

bool sphere::hit(const ray& r, double t_min, double t_max,
                 hit_record& rec) const {
  vec3 oc = r.origin() - center;
  auto a = r.direction().length_squared();
  auto half_b = dot(oc, r.direction());
  auto c = oc.length_squared() - radius * radius;
  auto discriminant = half_b * half_b - a * c;

  if (discriminant <= 0) {
    return false;
  }
  auto root = sqrt(discriminant);
  auto temp = (-half_b - root) / a;
  if (temp > t_max || temp < t_min) {
    temp = (-half_b + root) / a;
  }
  if (temp > t_max || temp < t_min) {
    return false;
  }
  rec.t = temp;
  rec.p = r.at(rec.t);
  rec.normal = (rec.p - center) / radius;
  vec3 outward_normal = (rec.p - center) / radius;
  rec.set_face_normal(r, outward_normal);
  return true;
}

#endif
