#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "../src/sandbox/isolate_sandbox.h"

TEST(IsolateSandbox, BasicCreation)
{
	sandbox_limits limits;
	EXPECT_NO_THROW(isolate_sandbox s(limits, 34));
	isolate_sandbox is(limits, 34);
	EXPECT_EQ(is.get_dir(), "/tmp/box/34");
	EXPECT_THROW(isolate_sandbox s(limits, 2365), sandbox_exception);
}
