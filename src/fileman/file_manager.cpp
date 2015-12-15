#include "file_manager.h"
#include <memory>

file_manager::file_manager(const std::string &caching_dir, const std::string &remote_url,
						   const std::string &username, const std::string &password)
{
	cache_man_ = cache_man_ptr(new cache_manager{caching_dir});
	http_man_ = http_man_ptr(new http_manager{remote_url, username, password});
}

file_manager::file_manager(cache_man_ptr cache_manager, http_man_ptr http_manager)
{
	cache_man_ = std::move(cache_manager);
	http_man_ = std::move(http_manager);
}

void file_manager::get_file(const std::string &src_name, const std::string &dst_path)
{
	try {
		cache_man_->get_file(src_name, dst_path);
		return;
	} catch(...) {}

	http_man_->get_file(src_name, cache_man_->get_destination());
	cache_man_->get_file(src_name, dst_path);
}

void file_manager::put_file(const std::string &name)
{
	http_man_->put_file(name);
}

void file_manager::set_params(const std::string &destination, const std::string &username,
							const std::string &password)
{
	http_man_->set_params(destination, username, password);
}

std::string file_manager::get_destination() const
{
	return http_man_->get_destination();
}
