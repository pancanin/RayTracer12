#include <iostream>
#include <cstdint>
#include <fstream>
#include <string>
#include <chrono>

#define _USE_MATH_DEFINES
#include <math.h>

#include "src/out/ppm/PPMImageMeta.h"
#include "src/out/ppm/PPMImage.h"
#include "src/out/ppm/PPMImageWriter.h"
#include "src/out/ppm/PPMImageFileWriter.h"

#include "src/sampling/Raygen.h"
#include "src/sampling/Camera.h"
#include "src/scene/Scene.h"
#include "out/ppm/PPMColor.h"
#include "src/utils/TypeDefs.h"
#include "sampling/Tracer.h"
#include "optimisations/ThreadPool.h"
#include "optimisations/BucketRenderer.h"
#include "optimisations/AABB.h"
#include "optimisations/AABBTree.h"

int main(int argc, char** argv) {
  if (argc < 2) {
    std::cerr << "Provide the path to the crtscene file as a first argument to the program.\n";
    return -1;
  }

  std::cout << "Parsing scene object..." << '\n';
  Scene scene(argv[1]);
  std::cout << "Completed parsing scene object" << '\n';

  int32_t RENDER_WIDTH = scene.settings.imageSettings.width;
  int32_t RENDER_HEIGHT = scene.settings.imageSettings.heigth;
  PPMImageMeta imageMetadata(RENDER_WIDTH, RENDER_HEIGHT, MAX_COLOR);
  PPMImage image(imageMetadata);
  Camera camera(scene.camera.position, scene.camera.matrix);
  Raygen rayGenerator(RENDER_WIDTH, RENDER_HEIGHT, camera, -1);
  PPMColor backGroundColor = PPMColor::from(scene.settings.backgroundColor);
  const float shadowBias = 0.001f;
  // Higher values result in more lit scene
  const float lightIntensityMultiplier = 0.3;
  LightOptions lightOptions{ shadowBias, lightIntensityMultiplier };
  AABBTree tree(scene.objects);
  Lighting lighting(lightOptions, scene.lights, tree);
  Tracer tracer(scene, lighting, tree);

  auto start = std::chrono::steady_clock::now();

  {
    BucketRenderer bucketRenderer(rayGenerator, tracer, scene, image);

    std::cout << "Rendering..." << '\n';

    bucketRenderer.render();
  }

  auto end = std::chrono::steady_clock::now();
  std::chrono::duration<double> elapsedSeconds = end - start;
  std::cout << "Rendering took " << elapsedSeconds.count() << " sec." << '\n';

  PPMImageFileWriter fileWriter(image, "out.ppm");
  fileWriter.write();

  return 0;
}
