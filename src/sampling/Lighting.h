#ifndef LIGHTING_H
#define LIGHTING_H

#include <vector>

#include <utils/TypeDefs.h>
#include "sampling/Ray.h"
#include "optimisations/AABBTree.h"

struct Light {
  Vec3 pos;
  float intensity;
};

struct LightOptions {
  float shadowBias;
  float albedo;
};

struct Lighting {
  inline Lighting(LightOptions& opts, const std::vector<Light>& lights, AABBTree& tree) :
    options(opts), lights(lights), tree(tree) {}

  /// <summary>
  /// Calculates the shadow color from an intersection point.
  /// </summary>
  /// <param name="intersectionData">Data about the currently traced intersection.</param>
  /// <returns>Color for the shadow on the pixel currently sampled.</returns>
  inline InternalColor light(const IntersectionData& intersectionData) {
    InternalColor lightContributionColor{};

    for (const Light& light : lights) {
      Vec3 lightDir = light.pos - intersectionData.p;
      lightDir = glm::normalize(lightDir);
      Ray shadowRay{ pushIntersectionPoint(intersectionData.p, intersectionData.pN, options.shadowBias), lightDir };
      
      bool intersects = false;
      IntersectionData intrData = tree.intersectAABBTree(shadowRay);
      float lightSphereRadius = glm::length(lightDir);
      if (intrData.t < lightSphereRadius) {
        intersects = true;
      }

      if (!intersects) {
        float color = light.intensity / calcSphereArea(lightSphereRadius) * options.albedo * calcCosineLaw(intersectionData.pN, lightDir);
        lightContributionColor += InternalColor(color);
      }
    }

    return lightContributionColor;
  }

  inline float calcCosineLaw(const Vec3& hitNormal, const Vec3& lightDir) const {
    return glm::max(0.0f, glm::dot(hitNormal, lightDir));
  }

  inline float calcSphereArea(float radius) const {
    return 4.0f * M_PI * radius * radius;
  }

  /// <summary>
  /// Applies shadow bias to the intersection point in the direction of the provided hit vector.
  /// In effect, it pushes the intersection point in the direction of the provided normal, so we can 'raise'
  /// through the object and so the newly created shadow ray can go on and eventually hit a light.
  /// By choosing the proper shadow bias we can avoid 'self-intersection' from the point to the (same) point on the same plane.
  /// </summary>
  inline Vec3 pushIntersectionPoint(const Vec3& originalPoint, const Vec3& hitNormal, float bias) const {
    return originalPoint + (hitNormal * bias);
  }
private:
  LightOptions options;
  std::vector<Light> lights;
  AABBTree& tree;
};


#endif // !LIGHTING_H
