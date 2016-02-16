#include "fallback_file_manager.h"
#include <memory>

fallback_file_manager::fallback_file_manager (file_manager_ptr primary, file_manager_ptr secondary)
{
	primary_manager_ = std::move(primary);
	secondary_manager_ = std::move(secondary);
}

void fallback_file_manager::get_file(const std::string &src_name, const std::string &dst_name)
{
	try {
		primary_manager_->get_file(src_name, dst_name);
		return;
	} catch (...) {}

	secondary_manager_->get_file(src_name, dst_name);
	primary_manager_->put_file(dst_name, src_name);
}

void fallback_file_manager::put_file(const std::string &src_name, const std::string &dst_url)
{
	secondary_manager_->put_file(src_name, dst_url);
}
