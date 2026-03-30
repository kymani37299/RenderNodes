#pragma once

#include <string>
#include <fstream>
#include <filesystem>
#include <stdexcept>

#if defined(_WIN32)
#include <windows.h>
#elif defined(__APPLE__)
#include <cstdlib>
#else // Linux / Unix
#include <cstdlib>
#endif

namespace FileUtils {

    namespace fs = std::filesystem;

	inline bool FileExists(const std::string& path)
	{
		return fs::exists(path);
	}

    inline bool CopyFile(const std::string& src, const std::string& dst) 
    {
        try 
        {
            fs::create_directories(fs::path(dst).parent_path());
            fs::copy_file(src, dst, fs::copy_options::overwrite_existing);
            return true;
        }
        catch (const std::exception&) 
        {
            return false;
        }
    }

    inline bool CopyDirectory(const std::string& src, const std::string& dst) 
    {
        try
        {
            fs::create_directories(dst);
            fs::copy(src, dst, fs::copy_options::recursive | fs::copy_options::overwrite_existing);
            return true;
        }
        catch (const std::exception&) 
        {
            return false;
        }
    }

    inline bool DeleteDirectory(const std::string& path) 
    {
        try 
        {
            if (fs::exists(path) && fs::is_directory(path)) 
            {
                fs::remove_all(path);
                return true;
            }
            return false;
        }
        catch (const std::exception&) 
        {
            return false;
        }
    }

    inline bool DeleteFile(const std::string& path) {
        try 
        {
            if (fs::exists(path) && fs::is_regular_file(path)) 
            {
                fs::remove(path);
                return true;
            }
            return false;
        }
        catch (const std::exception&) 
        {
            return false;
        }
    }

	inline bool MakeDirectory(const std::string& path)
	{
		try
		{
			fs::create_directories(path);
			return true;
		}
		catch (const std::exception&)
		{
			return false;
		}
	}

    inline std::string MakeRelativePath(const std::string& from, const std::string& to) 
    {
        try 
        {
            fs::path fromPath(from);
            fs::path toPath(to);
            return fs::relative(toPath, fromPath).generic_string();
        }
        catch (const std::exception&) 
        {
            return "";
        }
    }

	inline bool HasForbiddenSymbols(const std::string& path) 
    {
		static const std::string ForbiddenSymbols = "<>:\"|?*";

		for (char c : path) 
        {
			// Control characters (Ascii)
			if (c >= 0 && c <= 31) 
				return true;

			if (ForbiddenSymbols.find(c) != std::string::npos)
				return true;
		}
		return false;
	}

	inline bool GetFileNameAndExtension(const std::string& path, std::string& outFileName, std::string& outExtension)
	{
		try
		{
			fs::path p(path);

			if (!p.has_filename())
				return false;

			outFileName = p.stem().string();
			outExtension = p.extension().string(); // includes the dot: ".txt"

			return true;
		}
		catch (const std::exception&)
		{
			return false;
		}
	}

	inline bool DecomposePath(const std::string& path, std::string& outRootPath, std::string& outFileName, std::string& outExtension)
	{
		try
		{
			fs::path p(path);
			if (!p.has_filename())
				return false;

			outRootPath = p.parent_path().string();
			outFileName = p.stem().string();
			outExtension = p.extension().string(); // includes the dot: ".txt"
			return true;
		}
		catch (const std::exception&)
		{
			return false;
		}
	}

	inline std::string GetWorkingDirectory()
	{
		return fs::current_path().generic_string();
	}

	inline std::string GetRelativePath(const std::string& absolutePath)
	{
		const fs::path base{ absolutePath };
		const fs::path ref{ GetWorkingDirectory() };
		const fs::path relative = fs::relative(base, ref);
		return relative.generic_string();
	}

	inline std::string GetAbsolutePath(const std::string& relativePath)
	{
		const fs::path base{ relativePath };
		const fs::path ref{ GetWorkingDirectory() };

		const fs::path absolute = fs::weakly_canonical(ref / base);
		return absolute.generic_string();
	}

	inline bool ReadFile(const std::string& path, std::vector<std::string>& content)
	{
		std::ifstream fileStream(path, std::ios::in);

		if (!fileStream.is_open()) {
			return false;
		}

		std::string line = "";
		while (!fileStream.eof()) {
			std::getline(fileStream, line);
			content.push_back(line);
		}

		fileStream.close();
		return true;
	}

	inline void OpenPathInFileManager(const std::string& path)
	{
		fs::path p(path);

		if (!fs::exists(p))
			return;

#if defined(_WIN32)
		std::wstring wPath(path.begin(), path.end());
		ShellExecuteW(nullptr, L"open", wPath.c_str(), nullptr, nullptr, SW_SHOWDEFAULT);
#elif defined(__APPLE__)
		std::string command = "open \"" + path + "\"";
		system(command.c_str());
#else
		std::string command = "xdg-open \"" + path + "\"";
		system(command.c_str());
#endif
	}
}