#include <iostream>

#include "job_callback.h"

job_callback::job_callback (std::shared_ptr<file_manager_base> fm) : fm_(fm)
{
}

void job_callback::operator() (const std::string &job_id, const std::string &job_url, const std::string &result_url)
{
	std::cout << job_url << std::endl;
}
