#pragma once

#include <string>
#include <fstream>
#include <filesystem>
#include <stdexcept>
#include <windows.h>

namespace FileUtils {

    namespace fs = std::filesystem;

    inline bool CopyFile(const std::string& src, const std::string& dst) 
    {
        try 
        {
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
            fs::copy(src, dst, fs::copy_options::recursive | fs::copy_options::overwrite_existing);
            return true;
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

    inline bool DeleteDirectoryIfExists(const std::string& path) 
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

    inline bool DeleteFileIfExists(const std::string& path) {
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

    inline std::string MakeRelativePath(const std::string& from, const std::string& to) 
    {
        try 
        {
            fs::path fromPath(from);
            fs::path toPath(to);
            return fs::relative(toPath, fromPath).string();
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

	inline std::string GetWorkingDirectory()
	{
		CHAR buffer[MAX_PATH] = { 0 };
		GetModuleFileNameA(NULL, buffer, MAX_PATH);
		std::string::size_type pos = std::string(buffer).find_last_of("\\/");
		return std::string(buffer).substr(0, pos);
	}

	inline std::string GetRelativePath(const std::string& absolutePath)
	{
		const fs::path base{ absolutePath };
		const fs::path ref{ GetWorkingDirectory() };
		const fs::path relative = fs::relative(base, ref);
		return relative.generic_string();
	}

}