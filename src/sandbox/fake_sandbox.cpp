#include "fake_sandbox.h"

fake_sandbox::fake_sandbox()
{
}

fake_sandbox::~fake_sandbox()
{
}

sandbox_results fake_sandbox::run(const std::string &binary, const std::vector<std::string> &arguments)
{
	return sandbox_results();
}
