//
// Created by Marc Rousavy on 21.02.24.
//

#pragma once

#include <jsi/jsi.h>
#include <unordered_map>
#include "HybridObject.h"
#include <type_traits>
#include <memory>

namespace margelo {

using namespace facebook;

template<typename ArgType, typename Enable = void>
struct JSIConverter {
    static ArgType fromJSI(jsi::Runtime&, const jsi::Value&) {
        static_assert(always_false<ArgType>::value, "This type is not supported by the JSIConverter!");
        return ArgType();
    }
    static jsi::Value toJSI(jsi::Runtime&, ArgType) {
        static_assert(always_false<ArgType>::value, "This type is not supported by the JSIConverter!");
        return jsi::Value::undefined();
    }

private:
    template<typename>
    struct always_false : std::false_type {};
};

template<>
struct JSIConverter<int> {
    static int fromJSI(jsi::Runtime&, const jsi::Value& arg) {
        return static_cast<int>(arg.asNumber());
    }
    static jsi::Value toJSI(jsi::Runtime&, int arg) {
        return jsi::Value(arg);
    }
};

template<>
struct JSIConverter<double> {
    static double fromJSI(jsi::Runtime&, const jsi::Value& arg) {
        return arg.asNumber();
    }
    static jsi::Value toJSI(jsi::Runtime&, double arg) {
        return jsi::Value(arg);
    }
};

template<>
struct JSIConverter<int64_t> {
    static double fromJSI(jsi::Runtime& runtime, const jsi::Value& arg) {
        return arg.asBigInt(runtime).asInt64(runtime);
    }
    static jsi::Value toJSI(jsi::Runtime& runtime, int64_t arg) {
        return jsi::BigInt::fromInt64(runtime, arg);
    }
};

template<>
struct JSIConverter<uint64_t> {
    static double fromJSI(jsi::Runtime& runtime, const jsi::Value& arg) {
        return arg.asBigInt(runtime).asUint64(runtime);
    }
    static jsi::Value toJSI(jsi::Runtime& runtime, uint64_t arg) {
        return jsi::BigInt::fromUint64(runtime, arg);
    }
};

template<>
struct JSIConverter<bool> {
    static bool fromJSI(jsi::Runtime&, const jsi::Value& arg) {
        return arg.asBool();
    }
    static jsi::Value toJSI(jsi::Runtime&, bool arg) {
        return jsi::Value(arg);
    }
};

template<>
struct JSIConverter<std::string> {
    static std::string fromJSI(jsi::Runtime& runtime, const jsi::Value& arg) {
        return arg.asString(runtime).utf8(runtime);
    }
    static jsi::Value toJSI(jsi::Runtime& runtime, const std::string& arg) {
        return jsi::String::createFromUtf8(runtime, arg);
    }
};

template<typename ElementType>
struct JSIConverter<std::vector<ElementType>> {
    static std::vector<ElementType> fromJSI(jsi::Runtime& runtime, const jsi::Value& arg) {
        jsi::Array array = arg.asObject(runtime).asArray(runtime);
        size_t length = array.size(runtime);

        std::vector<ElementType> vector;
        vector.reserve(length);
        for (size_t i = 0; i < length; ++i) {
            jsi::Value elementValue = array.getValueAtIndex(runtime, i);
            vector.emplace_back(JSIConverter<ElementType>::fromJSI(runtime, elementValue));
        }
        return vector;
    }
    static jsi::Value toJSI(jsi::Runtime& runtime, std::vector<ElementType>& vector) {
        jsi::Array array(runtime, vector.size());
        for (size_t i = 0; i < vector.size(); i++) {
            jsi::Value value = JSIConverter<ElementType>::toJSI(runtime, vector[i]);
            array.setValueAtIndex(runtime, i, std::move(value));
        }
        return array;
    }
};

template<typename ValueType>
struct JSIConverter<std::unordered_map<std::string, ValueType>> {
    static std::unordered_map<std::string, ValueType> fromJSI(jsi::Runtime& runtime, const jsi::Value& arg) {
        jsi::Object object = arg.asObject(runtime);
        jsi::Array propertyNames = object.getPropertyNames(runtime);
        size_t length = propertyNames.size(runtime);

        std::unordered_map<std::string, ValueType> map;
        for (size_t i = 0; i < length; ++i) {
            auto key = propertyNames.getValueAtIndex(runtime, i).asString(runtime).utf8(runtime);
            jsi::Value value = object.getProperty(runtime, key.c_str());
            map.emplace(key, JSIConverter<ValueType>::fromJSI(runtime, value));
        }
        return map;
    }
    static jsi::Value toJSI(jsi::Runtime& runtime, std::unordered_map<std::string, ValueType>& map) {
        jsi::Object object(runtime);
        for (const auto& pair : map) {
            jsi::Value value = JSIConverter<ValueType>::toJSI(runtime, pair.second);
            jsi::String key = jsi::String::createFromUtf8(runtime, pair.first);
            object.setProperty(runtime, key, std::move(value));
        }
        return object;
    }
};

template<typename T>
struct is_shared_ptr_to_host_object : std::false_type {};

template<typename T>
struct is_shared_ptr_to_host_object<std::shared_ptr<T>> : std::is_base_of<jsi::HostObject, T> {};

template<typename T>
struct JSIConverter<T, std::enable_if_t<is_shared_ptr_to_host_object<T>::value>> {
    static T fromJSI(jsi::Runtime& runtime, const jsi::Value& arg) {
        return arg.asObject(runtime).asHostObject<typename T::element_type>(runtime);
    }
    static jsi::Value toJSI(jsi::Runtime& runtime, T& arg) {
        return jsi::Object::createFromHostObject(runtime, arg);
    }
};

} // margelo
