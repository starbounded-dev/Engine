#pragma once

#include <string>
#include <vector>
#include <filesystem>
#include <optional>

namespace Core {
namespace Utilities {

	class FileSystem
	{
	public:
		// File dialog operations (NFD integration)
		static std::optional<std::string> OpenFileDialog(const char* filterList = nullptr);
		static std::optional<std::string> SaveFileDialog(const char* filterList = nullptr);
		static std::optional<std::string> SelectFolderDialog(const char* defaultPath = nullptr);
		
		// Multiple file selection
		static std::vector<std::string> OpenMultipleFilesDialog(const char* filterList = nullptr);

		// File operations
		static bool FileExists(const std::string& filepath);
		static bool DirectoryExists(const std::string& directory);
		static bool CreateDirectory(const std::string& directory);
		static bool DeleteFile(const std::string& filepath);
		static bool CopyFile(const std::string& source, const std::string& destination);
		static bool MoveFile(const std::string& source, const std::string& destination);
		
		// Path operations
		static std::string GetFileName(const std::string& filepath);
		static std::string GetFileNameWithoutExtension(const std::string& filepath);
		static std::string GetFileExtension(const std::string& filepath);
		static std::string GetParentPath(const std::string& filepath);
		static std::string GetAbsolutePath(const std::string& filepath);
		static std::string GetRelativePath(const std::string& filepath, const std::string& base);
		
		// File content operations
		static std::optional<std::string> ReadFileToString(const std::string& filepath);
		static bool WriteStringToFile(const std::string& filepath, const std::string& content);
		static std::optional<std::vector<uint8_t>> ReadFileToBytes(const std::string& filepath);
		static bool WriteBytesToFile(const std::string& filepath, const std::vector<uint8_t>& data);
		
		// Directory operations
		static std::vector<std::string> GetFilesInDirectory(const std::string& directory, bool recursive = false);
		static std::vector<std::string> GetDirectoriesInDirectory(const std::string& directory);
		
		// Utility
		static size_t GetFileSize(const std::string& filepath);
		static uint64_t GetFileModificationTime(const std::string& filepath);
		static bool IsAbsolutePath(const std::string& path);
		static std::string NormalizePath(const std::string& path);
		
		// Application paths
		static std::string GetExecutablePath();
		static std::string GetWorkingDirectory();
		static bool SetWorkingDirectory(const std::string& directory);
		
		// Common filter patterns for file dialogs
		static constexpr const char* FILTER_ALL = nullptr;
		static constexpr const char* FILTER_IMAGES = "png,jpg,jpeg,bmp,tga,hdr";
		static constexpr const char* FILTER_MODELS = "obj,fbx,gltf,glb,dae,3ds";
		static constexpr const char* FILTER_SHADERS = "glsl,vert,frag,comp,geom,tesc,tese";
		static constexpr const char* FILTER_MATERIALS = "mat,material";
		static constexpr const char* FILTER_SCENES = "scene";
	};

} // namespace Utilities
} // namespace Core
