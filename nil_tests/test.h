#pragma once

#include "cunit.h"

#define START_FILE_TESTS cunit_suite(__FILE_NAME__, nullptr, nullptr)
#define TEST(FN) cunit_test(#FN, FN)