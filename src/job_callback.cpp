#include <iostream>

#include "job_callback.h"

job_callback::job_callback (std::shared_ptr<file_manager_base> fm) : fm_(fm)
{
}

void job_callback::operator() (job_request request)
{
	std::cout << request.job_url << std::endl;
}
