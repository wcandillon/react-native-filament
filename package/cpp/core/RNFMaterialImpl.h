//
// Created by Hanno Gödecke on 25.03.24.
//

#pragma once

#include "RNFMaterialInstanceWrapper.h"
#include "jsi/RNFHybridObject.h"

#include <filament/Material.h>

namespace margelo {

using namespace filament;

class MaterialImpl {
public:
  explicit MaterialImpl(std::shared_ptr<Material> material) : _material(material) {}

  std::shared_ptr<Material> getMaterial() {
    return _material;
  }

  const std::vector<std::shared_ptr<MaterialInstanceWrapper>>& getInstances() {
    return _instances;
  }

public:
  std::shared_ptr<MaterialInstanceWrapper> createInstance();
  std::shared_ptr<MaterialInstanceWrapper> getDefaultInstance();
  void setDefaultFloatParameter(std::string name, double value);
  void setDefaultTextureParameter(std::string name, Texture* texture, TextureSampler sampler);
  void setBaseColorSRGB(std::vector<double> rgba);
  std::string getName();

private:
  std::mutex _mutex;
  std::shared_ptr<Material> _material;
  // Keep track of all instances
  std::vector<std::shared_ptr<MaterialInstanceWrapper>> _instances;
};
} // namespace margelo