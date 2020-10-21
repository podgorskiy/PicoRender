#include <optix_world.h>
#include "ray_payload.h"
#include "types.h"

/*! the parameters that describe each individual sphere geometry */
rtDeclareVariable(float3, center, , );
rtDeclareVariable(float,  radius, , );

/*! the implicit state's ray we will intersect against */
rtDeclareVariable(optix::Ray, ray, rtCurrentRay, );

/*! the attributes we use to communicate between intersection programs and hit program */
rtDeclareVariable(float3, hit_rec_normal, attribute hit_rec_normal, );
rtDeclareVariable(float3, hit_rec_p, attribute hit_rec_p, );

/*! the per ray data we operate on */
rtDeclareVariable(RayPayload, ray_payload, rtPayload, );


// Program that performs the ray-sphere intersection
//
// note that this is here is a simple, but not necessarily most numerically
// stable ray-sphere intersection variant out there. There are more
// stable variants out there, but for now let's stick with the one that
// the reference code used.
RT_PROGRAM void hit_sphere(int pid)
{
  const vec3 oc = to_glm(ray.origin - center);
  const scal  a = dot(to_glm(ray.direction), to_glm(ray.direction));
  const scal  b = dot(oc, to_glm(ray.direction));
  const scal  c = dot(oc, oc) - radius * radius;
  const scal  discriminant = b * b - a * c;

  if (discriminant < 0.f) return;

  float temp = (-b - sqrtf(discriminant)) / a;
  if (temp < ray.tmax && temp > ray.tmin) {
    if (rtPotentialIntersection(temp)) {
      hit_rec_p = ray.origin + temp * ray.direction;
      hit_rec_normal = (hit_rec_p - center) / radius;
      rtReportIntersection(0);
    }
  }
  temp = (-b + sqrtf(discriminant)) / a;
  if (temp < ray.tmax && temp > ray.tmin) {
    if (rtPotentialIntersection(temp)) {
      hit_rec_p = ray.origin + temp * ray.direction;
      hit_rec_normal = (hit_rec_p - center) / radius;
      rtReportIntersection(0);
    }
  }
}

/*! returns the bounding box of the pid'th primitive
  in this gometry. Since we only have one sphere in this
  program (we handle multiple spheres by having a different
  geometry per sphere), the'pid' parameter is ignored */
RT_PROGRAM void get_bounds(int pid, float result[6])
{
  optix::Aabb* aabb = (optix::Aabb*)result;
  aabb->m_min = center - radius;
  aabb->m_max = center + radius;
}
