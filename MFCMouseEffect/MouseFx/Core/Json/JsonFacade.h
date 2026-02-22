#pragma once

// Third-party JSON dependency boundary.
// Business modules should include this file instead of including vendor headers directly.
//
// Size-oriented options:
// - JSON_NO_IO: disable stream operators we do not use in runtime path.
// - JSON_USE_GLOBAL_UDLS: disable global literals (e.g. "_json") we do not use.
// - JSON_DISABLE_ENUM_SERIALIZATION: disable enum serialization helpers we do not use.
#ifndef JSON_NO_IO
#define JSON_NO_IO 1
#endif

#ifndef JSON_USE_GLOBAL_UDLS
#define JSON_USE_GLOBAL_UDLS 0
#endif

#ifndef JSON_DISABLE_ENUM_SERIALIZATION
#define JSON_DISABLE_ENUM_SERIALIZATION 1
#endif

#include "MouseFx/ThirdParty/json.hpp"
