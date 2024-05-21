//
// Created by Hanno Gödecke on 20.03.24.
//

#pragma once

#include <filament/MaterialInstance.h>

#include "jsi/RNFHybridObject.h"

namespace margelo {

using namespace filament;

class MaterialInstanceWrapper : public HybridObject {

public:
  explicit MaterialInstanceWrapper(MaterialInstance* materialInstance)
      : HybridObject("MaterialInstanceWrapper"), _materialInstance(materialInstance) {}

  void loadHybridMethods() override;

  // Convenience method for updating baseColorFactor parameter
  static void changeAlpha(MaterialInstance* materialInstance, double alpha);

public: // Public API
  void setCullingMode(std::string mode);
  void setTransparencyMode(std::string mode);
  // Convenience method for updating baseColorFactor parameter
  void changeAlpha(double alpha);
  void setParameter(std::string name, double value);
  void setBaseColorSRGB(std::vector<double> rgba);
  std::string getName();

public: // Internal API
  MaterialInstance* getMaterialInstance() {
    return _materialInstance;
  }

private:
  std::mutex _mutex;
  MaterialInstance* _materialInstance;
};

} // namespace margelo