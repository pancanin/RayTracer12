#ifndef TRIANGLE_H
#define TRIANGLE_H

#include <utils/TypeDefs.h>

#include "sampling/Vertex.h"
#include "sampling/IntersectionData.h"

struct Triangle {
  inline Triangle(const Vertex& a, const Vertex& b, const Vertex& c) : a(a), b(b), c(c),
    n(glm::normalize(glm::cross(b.pos - a.pos, c.pos - a.pos))) {}

  Vertex a;
  Vertex b;
  Vertex c;
  Vec3 n;

  inline float area() const {
    return glm::length(glm::cross(b.pos - a.pos, c.pos - a.pos)) / 2.0f;
  }

  inline bool intersect(const Ray& ray, IntersectionData& intersectionData) const {
    float rayProjectionOnPlaneNormal = dot(ray.dir, n);

    /* The ray is perpendicular we have no work */
    if (rayProjectionOnPlaneNormal == 0.0f) {
      return false;
    }

    // This is the projection of the vector to the point in the triangle on the normal.
    // It is also the distance from the ray origin to the plane.
    float t = glm::dot(this->a.pos - ray.origin, n) / rayProjectionOnPlaneNormal;

    if (t <= 0 || t > intersectionData.t) {
      return false;
    }

    Vec3 p = ray.origin + t * ray.dir;

    if (dot(n, cross(b.pos - a.pos, p - a.pos)) < 0.0f ||
      dot(n, cross(c.pos - b.pos, p - b.pos)) < 0.0f ||
      dot(n, cross(a.pos - c.pos, p - c.pos)) < 0.0f) {
      return false;
    }

    intersectionData.t = t;
    intersectionData.p = p;
    intersectionData.intersection = true;

    // Calculate barycentric coordinates of the hit and set the hit normal
    float areaM = glm::length(glm::cross(p - a.pos, c.pos - a.pos)) / 2;
    float areaN = glm::length(glm::cross(b.pos - a.pos, p - a.pos)) / 2;
    float areaTri = glm::length(glm::cross(b.pos - a.pos, c.pos - a.pos)) / 2;

    float u = areaM / areaTri;
    float v = areaN / areaTri;
    float w = 1 - u - v;
    intersectionData.hitBaryCentricCoordinates = Vec3(1 - u - v, u, v);
    intersectionData.pN = b.smoothNormal * u + c.smoothNormal * v + a.smoothNormal * w;
    intersectionData.pNNonSmooth = n;

    return true;
  }
};

#endif // !TRIANGLE_H