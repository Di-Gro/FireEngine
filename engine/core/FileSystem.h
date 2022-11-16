#pragma once

#include <filesystem>

namespace fs = std::filesystem;


namespace FileSystem {

	class File {
	public:
		static bool Exist(fs::path path);
	};

}

