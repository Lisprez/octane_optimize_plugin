#pragma once

#include <string>

namespace material {

	class MaterialLibrary {
	public:
		MaterialLibrary(const MaterialLibrary&) = delete;
		MaterialLibrary(const MaterialLibrary&&) = delete;
		MaterialLibrary& operator=(const MaterialLibrary&) = delete;
		MaterialLibrary& operator=(const MaterialLibrary&&) = delete;

		MaterialLibrary(const std::string& libraryPath);
		~MaterialLibrary();

		void Load() const;

	private:
		std::string library_path_;
	};
}