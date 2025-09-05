#pragma once

#include <tier0/log.h>

#define ASSERT_MSG(cond, x, ...) if (!(cond)) { LOG_FATAL("ASSERTION FAILED: " #cond ": " x, ##__VA_ARGS__); }