#ifndef RAYGEN_H
#define RAYGEN_H

#include <utils/TypeDefs.h>

struct Raygen {
  inline Raygen(int32_t wid, int32_t hei, const glm::vec3& cameraPos, float focalDist):
    _width(wid), _height(hei), aspRat(static_cast<float>(wid) / hei), cameraPos(cameraPos), focalDist(focalDist) {

  }
  /// <summary>
  /// Creates a ray from camera position to an image plane.
  /// x, y are in raster coordinates.
  /// </summary>
  inline Ray gen(float x, float y) {
    /* Sample the pixel at its center */
    x += 0.5f;
    y += 0.5f;

    /* Transform to NDC - 0.0 - 1.0 */
    x /= _width;
    y /= _height;

    /* To Screen space - bottom left is [-1;-1] and top right is [1;1] */
    x = (x * 2) - 1;
    y = 1 - (y * 2);

    /* Consider the aspect ratio */
    x *= aspRat;

    glm::vec3 rayDir(x, y, focalDist);

    return Ray { cameraPos, glm::normalize(rayDir) };
  }

  int32_t _width;
  int32_t _height;
  float aspRat;
  glm::vec3 cameraPos;
  float focalDist;
};

#endif // !RAYGEN_H