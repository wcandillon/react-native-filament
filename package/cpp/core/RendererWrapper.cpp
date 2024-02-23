#include "RendererWrapper.h"

namespace margelo {
using namespace filament;

RendererWrapper::RendererWrapper(const std::shared_ptr<Renderer>& renderer) : _renderer(renderer) {
  // Support translucent views
  // TODO: make this configurable in the future
  _renderer->setClearOptions({.clear = true});
}

void RendererWrapper::loadHybridMethods() {
  registerHybridMethod("beginFrame", &RendererWrapper::beginFrame, this);
  registerHybridMethod("render", &RendererWrapper::render, this);
  registerHybridMethod("endFrame", &RendererWrapper::endFrame, this);
}

bool RendererWrapper::beginFrame(std::shared_ptr<SwapChainWrapper> swapChain, double frameTimeNanos) {
  return _renderer->beginFrame(swapChain->getSwapChain().get(), frameTimeNanos);
}
void RendererWrapper::render(std::shared_ptr<ViewWrapper> view) {
  _renderer->render(view->getView().get());
}
void RendererWrapper::endFrame() {
  _renderer->endFrame();
}
} // namespace margelo