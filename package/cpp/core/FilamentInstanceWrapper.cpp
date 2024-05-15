//
// Created by Hanno Gödecke on 26.03.24.
//

#include "FilamentInstanceWrapper.h"
#include "AnimatorWrapper.h"
#include "GlobalNameComponentManager.h"
#include <filament/Engine.h>
#include <filament/RenderableManager.h>
#include <filament/TransformManager.h>
#include <utils/NameComponentManager.h>

#include <map>

namespace margelo {

void FilamentInstanceWrapper::loadHybridMethods() {
  registerHybridMethod("getEntities", &FilamentInstanceWrapper::getEntities, this);
  registerHybridMethod("getRoot", &FilamentInstanceWrapper::getRoot, this);
  registerHybridMethod("createAnimator", &FilamentInstanceWrapper::createAnimator, this);
  registerHybridMethod("getBoundingBox", &FilamentInstanceWrapper::getBoundingBox, this);
  registerHybridMethod("syncWithInstance", &FilamentInstanceWrapper::syncWithInstance, this);
}

std::vector<std::shared_ptr<EntityWrapper>> FilamentInstanceWrapper::getEntities() {
  std::vector<std::shared_ptr<EntityWrapper>> entities;
  const Entity* entityArray = _instance->getEntities();
  size_t entityCount = _instance->getEntityCount();
  for (int i = 0; i < entityCount; i++) {
    entities.push_back(std::make_shared<EntityWrapper>(entityArray[i]));
  }
  return entities;
}

std::shared_ptr<EntityWrapper> FilamentInstanceWrapper::getRoot() {
  Entity rootEntity = _instance->getRoot();
  return std::make_shared<EntityWrapper>(rootEntity);
}

std::shared_ptr<AnimatorWrapper> FilamentInstanceWrapper::createAnimator() {
  Animator* animator = _instance->getAnimator();
  return std::make_shared<AnimatorWrapper>(animator);
}
std::shared_ptr<AABBWrapper> FilamentInstanceWrapper::getBoundingBox() {
  auto box = _instance->getBoundingBox();
  return std::make_shared<AABBWrapper>(box);
}

void FilamentInstanceWrapper::syncWithInstance(std::shared_ptr<FilamentInstanceWrapper> instanceWrapper) {
  FilamentInstance* masterInstance = instanceWrapper->getInstance();

  const FilamentAsset* asset = _instance->getAsset();
  Engine* engine = asset->getEngine();
  TransformManager& transformManager = engine->getTransformManager();
  Animator* masterAnimator = masterInstance->getAnimator();

  // Syncing the entities
  // TODO: we are not syncing the morph weights here yet
  // TODO: Put the name map generation into a function
  // TODO: Calculate the name map only once
  // TODO: Refactor the global name component manager pattern?
  // TODO: Wrap in transform transaction?

  // Get name map for master instance
  size_t masterEntitiesCount = masterInstance->getEntityCount();
  const Entity* masterEntities = masterInstance->getEntities();
  std::map<std::string, Entity> masterEntityMap;
  for (size_t entityIndex = 0; entityIndex < masterEntitiesCount; entityIndex++) {
    const Entity masterEntity = masterEntities[entityIndex];
    NameComponentManager::Instance masterNameInstance = GlobalNameComponentManager::getInstance()->getInstance(masterEntity);
    if (!masterNameInstance.isValid()) {
      continue;
    }
    auto masterInstanceName = GlobalNameComponentManager::getInstance()->getName(masterNameInstance);
    masterEntityMap[masterInstanceName] = masterEntity;
  }

  // Get name map for instance
  size_t instanceEntitiesCount = _instance->getEntityCount();
  const Entity* instanceEntities = _instance->getEntities();
  std::map<std::string, Entity> instanceEntityMap;
  for (size_t entityIndex = 0; entityIndex < instanceEntitiesCount; entityIndex++) {
    const Entity instanceEntity = instanceEntities[entityIndex];
    NameComponentManager::Instance instanceNameInstance = GlobalNameComponentManager::getInstance()->getInstance(instanceEntity);
    if (!instanceNameInstance.isValid()) {
      continue;
    }
    auto instanceName = GlobalNameComponentManager::getInstance()->getName(instanceNameInstance);
    instanceEntityMap[instanceName] = instanceEntity;
  }

  // Sync the same named entities:
  for (auto const& [name, masterEntity] : masterEntityMap) {
    auto instanceEntity = instanceEntityMap[name];
    if (instanceEntity.isNull()) {
      continue;
    }

    // Sync the transform
    TransformManager::Instance masterTransformInstance = transformManager.getInstance(masterEntity);
    TransformManager::Instance instanceTransformInstance = transformManager.getInstance(instanceEntity);

    if (!masterTransformInstance.isValid() || !instanceTransformInstance.isValid()) {
      Logger::log("FilamentInstanceWrapper", "Transform instance is for entity named %s is invalid", name.c_str());
      continue;
    }

    math::mat4f masterTransform = transformManager.getTransform(masterTransformInstance);
    transformManager.setTransform(instanceTransformInstance, masterTransform);
  }

  // Syncing the bones / joints
  masterAnimator->updateBoneMatricesForInstance(_instance);
}

} // namespace margelo
