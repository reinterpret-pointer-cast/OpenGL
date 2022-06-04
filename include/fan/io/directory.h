#pragma once

#include _FAN_PATH(types/types.h)
#include _FAN_PATH(types/vector.h)

#include _FAN_PATH(graphics/webp.h)

#include <fstream>
#include <string>
#include <filesystem>

namespace fan {
  namespace io {
		static bool directory_exists(const std::string& directory) {
			return std::filesystem::exists(directory);
		}

		struct iterate_sort_t {
			std::string path;
			uint64_t area;

			static bool comp_cb(const iterate_sort_t& a,const iterate_sort_t& b) { return a.area > b.area; }
		};

		static void iterate_directory_by_image_size_(
			const std::string& path,
			std::vector<iterate_sort_t>* sorted,
			const std::function<void(const std::string& path)>& function
		) {

			for (const auto& entry : std::filesystem::directory_iterator(path)) {
				if (entry.is_directory()) {
					iterate_directory_by_image_size_(entry.path().string(), sorted, function);
					continue;
				}
				std::string str = entry.path().string();
				std::replace(str.begin(), str.end(), '\\', '/');
				fan::vec2i image_size;
				if (fan::webp::get_image_size(str, &image_size)) {
					fan::throw_error("failed to get image size:" + str);
				}
				iterate_sort_t sort;
				sort.path = str;
				sort.area = image_size.multiply();
				sorted->push_back(sort);
			}
		}

		static void iterate_directory_by_image_size(
			const std::string& path,
			const std::function<void(const std::string& path)>& function
		) {
			std::vector<iterate_sort_t> sorted;
			iterate_directory_by_image_size_(path, &sorted, function);
			std::sort(sorted.begin(), sorted.end(), iterate_sort_t::comp_cb);
			for (const auto& i : sorted) {
				function(i.path);
			}
		}

		static void iterate_directory(
			const std::string& path,
			const std::function<void(const std::string& path)>& function
		) {

			if (!directory_exists(path)) {
				fan::throw_error("directory does not exist");
			}

			for (const auto& entry : std::filesystem::directory_iterator(path)) {
				if (entry.is_directory()) {
					iterate_directory(entry.path().string(), function);
					continue;
				}
				std::string str = entry.path().string();
				std::replace(str.begin(), str.end(), '\\', '/');
				function(str);
			}
		}
  }
}