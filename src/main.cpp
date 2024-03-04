#include <filesystem>
#include <functional>
#include <iostream>
#include <vector>
#include <string>

#include <unistd.h>
#include <stdlib.h>
#include <limits.h>

std::ostream &operator<<(std::ostream &os, const std::vector<std::string> &vec) {
	for (int iter = 0; iter < vec.size(); iter++) {
		if (iter == vec.size() - 1) {
			os << vec[iter];
		} else {
			os << vec[iter] << std::endl;
		}
	}

	return os;
}

template <class T> struct Result {
	bool ok;
	T res;

	Result() : ok(false), res() {}

	Result(bool ok, T res) : ok(ok), res(res) {}

	T unwrap(std::function<void()> err) {
		if (ok) return res;
		err();
		exit(1);
	}
};

Result<std::vector<std::string>> ash_ls(const std::filesystem::path &path = std::filesystem::current_path()) {
	std::vector<std::string> entries;
	Result<std::vector<std::string>> result;

	if (std::filesystem::is_directory(path)) {
		for (const auto &entry : std::filesystem::directory_iterator(path)) {
			if (std::filesystem::is_directory(entry)) {
				entries.push_back(entry.path().filename().string() + "/");
			} else {
				entries.push_back(entry.path().filename().string());
			}
		}
		result = {true, entries};
	} else {
		result = {false, {"Error: " + path.string() + " is not a directory"}};
	}

	return result;
}

Result<std::string> ash_chdir(const std::filesystem::path &path) {
	std::filesystem::path new_path = std::filesystem::current_path() / path;
	Result<std::string> result;

	try {
		std::filesystem::current_path(new_path);
		std::string updated_path = std::filesystem::current_path().string();
		setenv("PWD", updated_path.c_str(), 1);
		result = {true, updated_path};
	} catch (std::filesystem::filesystem_error &e) {
		result = {false, "Error: could not change directory to " + new_path.string() + ": " + e.what()};
	}

	return result;
}

Result<std::filesystem::path> ash_pwd() {
	std::filesystem::path current_path = std::filesystem::current_path();
	return {!current_path.empty(), current_path};
}

Result<std::string> ash_getenv(const std::string &name) {
	char *env = getenv(name.c_str());
	bool ok = env != nullptr;
	return {ok, ok ? env : "Error: " + name + " is not set"};
}

int main(int argc, char *argv[]) {
	std::vector<std::string> args(argv, argv + argc);

	std::cout << "Welcome to the Ash shell!" << std::endl;

	auto wrap_pwd = ash_pwd();
	std::string pwd = wrap_pwd.unwrap([&]() { std::cout << wrap_pwd.res << std::endl; });
	std::cout << "Current directory: " << pwd << std::endl;

	auto wrap_ls = ash_ls();
	std::vector<std::string> ls = wrap_ls.unwrap([&]() { std::cout << wrap_ls.res << std::endl; });
	std::cout << "Contents of current directory: " << std::endl << ls << std::endl;
}
