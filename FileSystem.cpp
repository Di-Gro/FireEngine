#include "FileSystem.h"

#include <fstream>


namespace FileSystem {

	bool File::Exist(fs::path path) {
		std::ifstream file(path);
		return file.good();
	}

}

