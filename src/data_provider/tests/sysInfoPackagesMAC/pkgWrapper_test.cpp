/*
 * Wazuh SysInfo
 * Copyright (C) 2015, Wazuh Inc.
 * December 14, 2020.
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General Public
 * License (version 2) as published by the FSF - Free Software
 * Foundation.
 */

#include "pkgWrapper_test.h"
#include "packages/packageMac.h"
#include "packages/pkgWrapper.h"

void PKGWrapperTest::SetUp() {};

void PKGWrapperTest::TearDown() {};

using ::testing::_;
using ::testing::Return;

const auto INPUT_PATH {std::filesystem::current_path() / "input_files/"};

TEST_F(PKGWrapperTest, Ok)
{
    struct PackageContext ctx {INPUT_PATH, "PKGWrapperTest_Ok", ""};
    std::shared_ptr<PKGWrapper> wrapper;
    EXPECT_NO_THROW(wrapper = std::make_shared<PKGWrapper>(ctx));
}
