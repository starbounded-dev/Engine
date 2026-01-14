#include "FileSystem.h"
#include <fstream>
#include <sstream>
#include <algorithm>

// NFD (Native File Dialog) integration - will be linked when NFD is properly set up
// For now, we provide stub implementations that can be replaced with NFD calls
#ifdef USE_NFD
#include <nfd.h>
#endif

namespace Core {
namespace Utilities {

	std::optional<std::string> FileSystem::OpenFileDialog(const char* filterList)
	{
#ifdef USE_NFD
		nfdchar_t* outPath = nullptr;
		nfdresult_t result = NFD_OpenDialog(filterList, nullptr, &outPath);
		
		if (result == NFD_OKAY)
		{
			std::string path(outPath);
			free(outPath);
			return path;
		}
		else if (result == NFD_CANCEL)
		{
			return std::nullopt;
		}
		else
		{
			// Error occurred
			return std::nullopt;
		}
#else
		// Stub implementation - return empty when NFD not available
		return std::nullopt;
#endif
	}

	std::optional<std::string> FileSystem::SaveFileDialog(const char* filterList)
	{
#ifdef USE_NFD
		nfdchar_t* outPath = nullptr;
		nfdresult_t result = NFD_SaveDialog(filterList, nullptr, &outPath);
		
		if (result == NFD_OKAY)
		{
			std::string path(outPath);
			free(outPath);
			return path;
		}
		else if (result == NFD_CANCEL)
		{
			return std::nullopt;
		}
		else
		{
			return std::nullopt;
		}
#else
		return std::nullopt;
#endif
	}

	std::optional<std::string> FileSystem::SelectFolderDialog(const char* defaultPath)
	{
#ifdef USE_NFD
		nfdchar_t* outPath = nullptr;
		nfdresult_t result = NFD_PickFolder(defaultPath, &outPath);
		
		if (result == NFD_OKAY)
		{
			std::string path(outPath);
			free(outPath);
			return path;
		}
		else if (result == NFD_CANCEL)
		{
			return std::nullopt;
		}
		else
		{
			return std::nullopt;
		}
#else
		return std::nullopt;
#endif
	}

	std::vector<std::string> FileSystem::OpenMultipleFilesDialog(const char* filterList)
	{
#ifdef USE_NFD
		nfdpathset_t pathSet;
		nfdresult_t result = NFD_OpenDialogMultiple(filterList, nullptr, &pathSet);
		
		std::vector<std::string> paths;
		if (result == NFD_OKAY)
		{
			size_t count = NFD_PathSet_GetCount(&pathSet);
			paths.reserve(count);
			
			for (size_t i = 0; i < count; i++)
			{
				nfdchar_t* path = NFD_PathSet_GetPath(&pathSet, i);
				paths.emplace_back(path);
			}
			
			NFD_PathSet_Free(&pathSet);
		}
		
		return paths;
#else
		return std::vector<std::string>();
#endif
	}

	bool FileSystem::FileExists(const std::string& filepath)
	{
		namespace fs = std::filesystem;
		return fs::exists(filepath) && fs::is_regular_file(filepath);
	}

	bool FileSystem::DirectoryExists(const std::string& directory)
	{
		namespace fs = std::filesystem;
		return fs::exists(directory) && fs::is_directory(directory);
	}

	bool FileSystem::CreateDirectory(const std::string& directory)
	{
		namespace fs = std::filesystem;
		try
		{
			return fs::create_directories(directory);
		}
		catch (const std::exception&)
		{
			return false;
		}
	}

	bool FileSystem::DeleteFile(const std::string& filepath)
	{
		namespace fs = std::filesystem;
		try
		{
			return fs::remove(filepath);
		}
		catch (const std::exception&)
		{
			return false;
		}
	}

	bool FileSystem::CopyFile(const std::string& source, const std::string& destination)
	{
		namespace fs = std::filesystem;
		try
		{
			fs::copy_file(source, destination, fs::copy_options::overwrite_existing);
			return true;
		}
		catch (const std::exception&)
		{
			return false;
		}
	}

	bool FileSystem::MoveFile(const std::string& source, const std::string& destination)
	{
		namespace fs = std::filesystem;
		try
		{
			fs::rename(source, destination);
			return true;
		}
		catch (const std::exception&)
		{
			return false;
		}
	}

	std::string FileSystem::GetFileName(const std::string& filepath)
	{
		namespace fs = std::filesystem;
		return fs::path(filepath).filename().string();
	}

	std::string FileSystem::GetFileNameWithoutExtension(const std::string& filepath)
	{
		namespace fs = std::filesystem;
		return fs::path(filepath).stem().string();
	}

	std::string FileSystem::GetFileExtension(const std::string& filepath)
	{
		namespace fs = std::filesystem;
		std::string ext = fs::path(filepath).extension().string();
		// Remove the leading dot
		if (!ext.empty() && ext[0] == '.')
			ext = ext.substr(1);
		return ext;
	}

	std::string FileSystem::GetParentPath(const std::string& filepath)
	{
		namespace fs = std::filesystem;
		return fs::path(filepath).parent_path().string();
	}

	std::string FileSystem::GetAbsolutePath(const std::string& filepath)
	{
		namespace fs = std::filesystem;
		try
		{
			return fs::absolute(filepath).string();
		}
		catch (const std::exception&)
		{
			return filepath;
		}
	}

	std::string FileSystem::GetRelativePath(const std::string& filepath, const std::string& base)
	{
		namespace fs = std::filesystem;
		try
		{
			return fs::relative(filepath, base).string();
		}
		catch (const std::exception&)
		{
			return filepath;
		}
	}

	std::optional<std::string> FileSystem::ReadFileToString(const std::string& filepath)
	{
		std::ifstream file(filepath, std::ios::in | std::ios::binary);
		if (!file.is_open())
			return std::nullopt;

		std::stringstream buffer;
		buffer << file.rdbuf();
		return buffer.str();
	}

	bool FileSystem::WriteStringToFile(const std::string& filepath, const std::string& content)
	{
		std::ofstream file(filepath, std::ios::out | std::ios::binary);
		if (!file.is_open())
			return false;

		file << content;
		return true;
	}

	std::optional<std::vector<uint8_t>> FileSystem::ReadFileToBytes(const std::string& filepath)
	{
		std::ifstream file(filepath, std::ios::in | std::ios::binary);
		if (!file.is_open())
			return std::nullopt;

		file.seekg(0, std::ios::end);
		size_t size = file.tellg();
		file.seekg(0, std::ios::beg);

		std::vector<uint8_t> data(size);
		file.read(reinterpret_cast<char*>(data.data()), size);
		return data;
	}

	bool FileSystem::WriteBytesToFile(const std::string& filepath, const std::vector<uint8_t>& data)
	{
		std::ofstream file(filepath, std::ios::out | std::ios::binary);
		if (!file.is_open())
			return false;

		file.write(reinterpret_cast<const char*>(data.data()), data.size());
		return true;
	}

	std::vector<std::string> FileSystem::GetFilesInDirectory(const std::string& directory, bool recursive)
	{
		namespace fs = std::filesystem;
		std::vector<std::string> files;

		try
		{
			if (recursive)
			{
				for (const auto& entry : fs::recursive_directory_iterator(directory))
				{
					if (entry.is_regular_file())
						files.push_back(entry.path().string());
				}
			}
			else
			{
				for (const auto& entry : fs::directory_iterator(directory))
				{
					if (entry.is_regular_file())
						files.push_back(entry.path().string());
				}
			}
		}
		catch (const std::exception&)
		{
			// Return empty vector on error
		}

		return files;
	}

	std::vector<std::string> FileSystem::GetDirectoriesInDirectory(const std::string& directory)
	{
		namespace fs = std::filesystem;
		std::vector<std::string> directories;

		try
		{
			for (const auto& entry : fs::directory_iterator(directory))
			{
				if (entry.is_directory())
					directories.push_back(entry.path().string());
			}
		}
		catch (const std::exception&)
		{
			// Return empty vector on error
		}

		return directories;
	}

	size_t FileSystem::GetFileSize(const std::string& filepath)
	{
		namespace fs = std::filesystem;
		try
		{
			return static_cast<size_t>(fs::file_size(filepath));
		}
		catch (const std::exception&)
		{
			return 0;
		}
	}

	uint64_t FileSystem::GetFileModificationTime(const std::string& filepath)
	{
		namespace fs = std::filesystem;
		try
		{
			auto ftime = fs::last_write_time(filepath);
			auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
				ftime - fs::file_time_type::clock::now() + std::chrono::system_clock::now());
			return std::chrono::duration_cast<std::chrono::seconds>(sctp.time_since_epoch()).count();
		}
		catch (const std::exception&)
		{
			return 0;
		}
	}

	bool FileSystem::IsAbsolutePath(const std::string& path)
	{
		namespace fs = std::filesystem;
		return fs::path(path).is_absolute();
	}

	std::string FileSystem::NormalizePath(const std::string& path)
	{
		namespace fs = std::filesystem;
		try
		{
			return fs::path(path).lexically_normal().string();
		}
		catch (const std::exception&)
		{
			return path;
		}
	}

	std::string FileSystem::GetExecutablePath()
	{
		namespace fs = std::filesystem;
#ifdef _WIN32
		char buffer[MAX_PATH];
		GetModuleFileNameA(NULL, buffer, MAX_PATH);
		return fs::path(buffer).parent_path().string();
#elif defined(__linux__)
		char buffer[1024];
		ssize_t len = readlink("/proc/self/exe", buffer, sizeof(buffer) - 1);
		if (len != -1)
		{
			buffer[len] = '\0';
			return fs::path(buffer).parent_path().string();
		}
		return "";
#elif defined(__APPLE__)
		char buffer[1024];
		uint32_t size = sizeof(buffer);
		if (_NSGetExecutablePath(buffer, &size) == 0)
			return fs::path(buffer).parent_path().string();
		return "";
#else
		return "";
#endif
	}

	std::string FileSystem::GetWorkingDirectory()
	{
		namespace fs = std::filesystem;
		return fs::current_path().string();
	}

	bool FileSystem::SetWorkingDirectory(const std::string& directory)
	{
		namespace fs = std::filesystem;
		try
		{
			fs::current_path(directory);
			return true;
		}
		catch (const std::exception&)
		{
			return false;
		}
	}

} // namespace Utilities
} // namespace Core
