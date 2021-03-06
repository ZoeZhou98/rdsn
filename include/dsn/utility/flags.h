// Copyright (c) 2017-present, Xiaomi, Inc.  All rights reserved.
// This source code is licensed under the Apache License Version 2.0, which
// can be found in the LICENSE file in the root directory of this source tree.

#pragma once

#include <string>
#include <cstdint>
#include <functional>
#include "errors.h"
#include "enum_helper.h"
#include "utils.h"

enum class flag_tag
{
    FT_MUTABLE = 0, /** flag data is mutable */
    FV_MAX_INDEX = 0,
};

ENUM_BEGIN(flag_tag, flag_tag::FV_MAX_INDEX)
ENUM_REG(flag_tag::FT_MUTABLE)
ENUM_END(flag_tag)

// support std::hash with enum types is implemented since gcc 6.1
// so we should define hash for flag_tag to compatible with gcc < 6.1
namespace std {
template <>
struct hash<flag_tag>
{
    size_t operator()(const flag_tag &t) const { return size_t(t); }
};
} // namespace std

// Example:
//    DSN_DEFINE_string("core", filename, "my_file.txt", "The file to read");
//    DSN_DEFINE_validator(filename, [](const char *fname){ return is_file(fname); });
//    auto fptr = file::open(FLAGS_filename, O_RDONLY | O_BINARY, 0);

#define DSN_DECLARE_VARIABLE(type, name) extern type FLAGS_##name

#define DSN_DECLARE_int32(name) DSN_DECLARE_VARIABLE(int32_t, name)
#define DSN_DECLARE_uint32(name) DSN_DECLARE_VARIABLE(uint32_t, name)
#define DSN_DECLARE_int64(name) DSN_DECLARE_VARIABLE(int64_t, name)
#define DSN_DECLARE_uint64(name) DSN_DECLARE_VARIABLE(uint64_t, name)
#define DSN_DECLARE_double(name) DSN_DECLARE_VARIABLE(double, name)
#define DSN_DECLARE_bool(name) DSN_DECLARE_VARIABLE(bool, name)
#define DSN_DECLARE_string(name) DSN_DECLARE_VARIABLE(const char *, name)

#define DSN_DEFINE_VARIABLE(type, section, name, default_value, desc)                              \
    type FLAGS_##name = default_value;                                                             \
    static dsn::flag_registerer FLAGS_REG_##name(section, #name, desc, &FLAGS_##name)

#define DSN_DEFINE_int32(section, name, val, desc)                                                 \
    DSN_DEFINE_VARIABLE(int32_t, section, name, val, desc)
#define DSN_DEFINE_uint32(section, name, val, desc)                                                \
    DSN_DEFINE_VARIABLE(uint32_t, section, name, val, desc)
#define DSN_DEFINE_int64(section, name, val, desc)                                                 \
    DSN_DEFINE_VARIABLE(int64_t, section, name, val, desc)
#define DSN_DEFINE_uint64(section, name, val, desc)                                                \
    DSN_DEFINE_VARIABLE(uint64_t, section, name, val, desc)
#define DSN_DEFINE_double(section, name, val, desc)                                                \
    DSN_DEFINE_VARIABLE(double, section, name, val, desc)
#define DSN_DEFINE_bool(section, name, val, desc)                                                  \
    DSN_DEFINE_VARIABLE(bool, section, name, val, desc)
#define DSN_DEFINE_string(section, name, val, desc)                                                \
    DSN_DEFINE_VARIABLE(const char *, section, name, val, desc)

// Convenience macro for the registration of a flag validator.
// `validator` must be a std::function<bool(FLAG_TYPE)> and receives the flag value as argument,
// returns true if validation passed.
// The program corrupts if the validation failed.
#define DSN_DEFINE_validator(name, validator)                                                      \
    static auto FLAGS_VALIDATOR_FN_##name = validator;                                             \
    static const dsn::flag_validator FLAGS_VALIDATOR_##name(                                       \
        #name, []() -> bool { return FLAGS_VALIDATOR_FN_##name(FLAGS_##name); })

#define DSN_TAG_VARIABLE(name, tag)                                                                \
    COMPILE_ASSERT(sizeof(decltype(FLAGS_##name)), exist_##name##_##tag);                          \
    static dsn::flag_tagger FLAGS_TAGGER_##name##_##tag(#name, flag_tag::tag)

namespace dsn {

// An utility class that registers a flag upon initialization.
class flag_registerer
{
public:
    flag_registerer(const char *section, const char *name, const char *desc, int32_t *val);
    flag_registerer(const char *section, const char *name, const char *desc, uint32_t *val);
    flag_registerer(const char *section, const char *name, const char *desc, int64_t *val);
    flag_registerer(const char *section, const char *name, const char *desc, uint64_t *val);
    flag_registerer(const char *section, const char *name, const char *desc, double *val);
    flag_registerer(const char *section, const char *name, const char *desc, bool *val);
    flag_registerer(const char *section, const char *name, const char *desc, const char **val);
};

// An utility class that registers a validator upon initialization.
using validator_fn = std::function<bool()>;
class flag_validator
{
public:
    flag_validator(const char *name, validator_fn);
};

class flag_tagger
{
public:
    flag_tagger(const char *name, const flag_tag &tag);
};

// Loads all the flags from configuration.
extern void flags_initialize();

// update the specified flag to val
extern error_s update_flag(const std::string &name, const std::string &val);

// determine if the tag is exist for the specified flag
extern bool has_tag(const std::string &name, const flag_tag &tag);

// list all the flags
extern std::string list_all_flags();

// get the json string of a specified flag
extern error_with<std::string> get_flag_str(const std::string &flag_name);
} // namespace dsn
