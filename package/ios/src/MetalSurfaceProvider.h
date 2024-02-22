//
//  MetalSurfaceProvider.h
//  Pods
//
//  Created by Marc Rousavy on 22.02.24.
//

#pragma once

#include "SurfaceProvider.h"
#include "MetalSurface.h"
#include <Metal/Metal.h>

namespace margelo {

class MetalSurfaceProvider : public SurfaceProvider {
  explicit MetalSurfaceProvider(CAMetalLayer* layer): _surface(std::make_shared<MetalSurface>(layer)) {
    onSurfaceCreated(_surface);
  }
  
  std::shared_ptr<Surface> getSurfaceOrNull() override {
    return _surface;
  }
  
private:
  std::shared_ptr<MetalSurface> _surface;
};

} // namespace margelo
