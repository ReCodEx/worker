#include "filesystem.h"

void helpers::copy_directory(const fs::path &src, const fs::path &dest)
{
	try {
		if (!fs::exists(src)) {
			throw filesystem_exception("Source directory does not exist");
		} else if (!fs::is_directory(src)) {
			throw filesystem_exception("Source directory is not a directory");
		} else if (fs::exists(dest)) {
			throw filesystem_exception("Destination directory exists");
		} else if (!fs::create_directories(dest)) {
			throw filesystem_exception("Destination directory cannot be created");
		}

		fs::directory_iterator endit;
		for (fs::directory_iterator it(src); it != endit; ++it) {
			if (fs::is_directory(it->status())) {
				helpers::copy_directory(it->path(), dest / it->path().filename());
			} else {
				fs::copy(it->path(), dest / it->path().filename());
			}
		}
	} catch (fs::filesystem_error &e) {
		throw filesystem_exception("Error in copying directories: " + std::string(e.what()));
	}

	return;
}
