//
// Created by Marc Rousavy on 21.02.24.
//

#include "JFilamentRecorder.h"

namespace margelo {

JFilamentRecorder::JFilamentRecorder(const jni::alias_ref<jhybridobject>& javaThis, int width, int height, int fps, long bitRate)
    : FilamentRecorder(width, height, fps, bitRate), _javaPart(make_global(javaThis)) {}

JFilamentRecorder::~JFilamentRecorder() {
  __android_log_write(ANDROID_LOG_INFO, TAG, "Destroying JFilamentRecorder...");
}

void JFilamentRecorder::registerNatives() {
  registerHybrid({
      makeNativeMethod("initHybrid", JFilamentRecorder::initHybrid),
  });
}

void* JFilamentRecorder::getNativeWindow() {
  throw std::runtime_error("getNativeWindow() is not yet implemented!");
}

std::future<void> JFilamentRecorder::startRecording() {
  auto self = shared<JFilamentRecorder>();
  return std::async(std::launch::async, [self]() {
    static const auto method = self->javaClassLocal()->getMethod<void()>("startRecording");
    method(self->_javaPart);
  });
}

std::future<std::string> JFilamentRecorder::stopRecording() {
  auto self = shared<JFilamentRecorder>();
  return std::async(std::launch::async, [self]() -> std::string {
    static const auto method = self->javaClassLocal()->getMethod<std::string()>("stopRecording");
    return method(self->_javaPart);
  });
}

bool JFilamentRecorder::getIsRecording() {
  throw std::runtime_error("isRecording is not yet implemented!");
}

jni::local_ref<JFilamentRecorder::jhybriddata> JFilamentRecorder::initHybrid(jni::alias_ref<jhybridobject> jThis, int width, int height,
                                                                             int fps, long bitRate) {
  __android_log_write(ANDROID_LOG_INFO, TAG, "Initializing JFilamentRecorder...");
  return makeCxxInstance(jThis, int width, int height, int fps, long bitRate);
}

} // namespace margelo