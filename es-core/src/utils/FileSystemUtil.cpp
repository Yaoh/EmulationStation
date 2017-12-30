#include "utils/FileSystemUtil.h"

#include <sys/stat.h>
#include <string.h>

#if defined(_WIN32)
// because windows...
#include <direct.h>
#include <Windows.h>
#define mkdir(x,y) _mkdir(x)
#define snprintf _snprintf
#define unlink _unlink
#define S_ISREG(x) (((x) & S_IFMT) == S_IFREG)
#define S_ISDIR(x) (((x) & S_IFMT) == S_IFDIR)
#else // _WIN32
#include <dirent.h>
#include <unistd.h>
#endif // _WIN32

namespace Utils
{
	namespace FileSystem
	{
		stringList getDirContent(const std::string& _path)
		{
			std::string path = genericPath(_path);
			stringList  contentList;

			// only parse the directory, if it's a directory
			if(isDirectory(path))
			{

#if defined(_WIN32)
				WIN32_FIND_DATA findData;
				HANDLE          hFind = FindFirstFile((path + "/*").c_str(), &findData);

				if(hFind != INVALID_HANDLE_VALUE)
				{
					// loop over all files in the directory
					do
					{
						std::string name(findData.cFileName);

						// ignore "." and ".."
						if((name != ".") && (name != ".."))
							contentList.push_back(name);
					}
					while(FindNextFile(hFind, &findData));

					FindClose(hFind);
				}
#else // _WIN32
				DIR* dir = opendir(path.c_str());

				if(dir != NULL)
				{
					struct dirent* entry;

					// loop over all files in the directory
					while((entry = readdir(dir)) != NULL)
					{
						std::string name(entry->d_name);

						// ignore "." and ".."
						if((name != ".") && (name != ".."))
							contentList.push_back(name);
					}

					closedir(dir);
				}
#endif // _WIN32

			}

			// sort the content list
			contentList.sort();

			// return the content list
			return contentList;

		} // getDirContent

		std::string getHomePath()
		{
			static std::string path;

			// only construct the homepath once
			if(!path.length())
			{
				// this should give us something like "/home/YOUR_USERNAME" on Linux and "C:/Users/YOUR_USERNAME/" on Windows
				std::string envHome(getenv("HOME"));
				if(envHome.length())
					path = genericPath(envHome);

#if defined(_WIN32)
				// but does not seem to work for Windows XP or Vista, so try something else
				if(!path.length())
				{
					std::string envDir(getenv("HOMEDRIVE"));
					std::string envPath(getenv("HOMEPATH"));
					if(envDir.length() && envPath.length())
						path = genericPath(envDir + "/" + envPath);
				}
#endif // _WIN32

			}

			// return constructed homepath
			return path;

		} // getHomePath

		std::string getCWDPath()
		{
			char temp[512];

			// return current working directory path
			return (getcwd(temp, 512) ? genericPath(temp) : "");

		} // getCWDPath

		std::string genericPath(const std::string& _path)
		{
			std::string path   = _path;
			size_t      offset = std::string::npos;

			// remove "\\\\?\\"
			if((path.find("\\\\?\\")) == 0)
				path.erase(0, 4);

			// convert '\\' to '/'
			while((offset = path.find('\\')) != std::string::npos)
				path.replace(offset, 1 ,"/");

			// remove double '/'
			while((offset = path.find("//")) != std::string::npos)
				path.erase(offset, 1);

			// return generic path
			return path;

		} // genericPath

		std::string escapedPath(const std::string& _path)
		{
			std::string path = genericPath(_path);

#if defined(_WIN32)
			// windows escapes stuff by just putting everything in quotes
			return '"' + path + '"';
#else // _WIN32
			// insert a backslash before most characters that would mess up a bash path
			const char* invalidChars = "\\ '\"!$^&*(){}[]?;<>";
			const char* invalidChar  = invalidChars;
			size_t      offset       = std::string::npos;

			while(*invalidChar)
			{
				while((offset = path.find(*invalidChar)) != std::string::npos)
					path.insert(offset, 1, '\\');

				++invalidChar;
			}

			// return escaped path
			return path;
#endif // _WIN32

		} // escapedPath

		std::string canonicalPath(const std::string& _path)
		{
			std::string path = absolutePath(_path);

			// cleanup path
			bool scan = true;
			while(scan)
			{
				stringList  pathList;
				size_t      start = 0;
				size_t      end   = 0;

				// split at '/'
				while((end = path.find("/", start)) != std::string::npos)
				{
					pathList.push_back(std::string(path, start, end - start));
					start = end + 1;
				}

				// add last folder / file to pathList
				if(start != path.size())
					pathList.push_back(std::string(path, start, path.size() - start));

				path.clear();
				scan = false;

				for(stringList::const_iterator it = pathList.cbegin(); it != pathList.cend(); ++it)
				{
					// ignore empty
					if((*it).empty())
						continue;

					// remove "/./"
					if((*it) == ".")
						continue;

					// resolve "/../"
					if((*it) == "..")
					{
						path = getParent(path);
						continue;
					}

#if defined(_WIN32)
					// append folder to path
					path += (path.size() == 0) ? (*it) : ("/" + (*it));
#else // _WIN32
					// append folder to path
					path += ("/" + (*it));
#endif // _WIN32

					// resolve symlink
					if(isSymlink(path))
					{
						std::string resolved = resolveSymlink(path);

						if(resolved.empty())
							return "";

						if(isAbsolute(resolved))
							path = resolved;
						else
							path = getParent(path) + "/" + resolved;

						for( ++it; it != pathList.cend(); ++it)
							path += (path.size() == 0) ? (*it) : ("/" + (*it));

						scan = true;
						break;
					}
				}
			}

			// return canonical path
			return path;

		} // canonicalPath

		std::string absolutePath(const std::string& _path, const std::string& _base)
		{
			std::string path = genericPath(_path);
			std::string base = isAbsolute(_base) ? genericPath(_base) : absolutePath(_base);

			// return absolute path
			return isAbsolute(path) ? path : genericPath(base + "/" + path);

		} // absolutePath

		std::string resolvePath(const std::string& _path, const std::string& _relativeTo, const bool _allowHome)
		{
			std::string path       = genericPath(_path);
			std::string relativeTo = isDirectory(_relativeTo) ? _relativeTo : getParent(_relativeTo);

			// nothing to resolve
			if(!path.length())
				return path;

			// replace '.' with relativeTo
			if(path[0] == '.')
				return genericPath(relativeTo + "/" + &(path[1]));

			// replace '~' with homePath
			if(_allowHome && (path[0] == '~'))
				return genericPath(getHomePath() + "/" + &(path[1]));

			// nothing to resolve
			return path;

		} // resolvePath

		std::string resolveSymlink(const std::string& _path)
		{
			std::string path = genericPath(_path);
			std::string resolved;

#if defined(_WIN32)
			HANDLE hFile = CreateFile(path.c_str(), FILE_READ_ATTRIBUTES, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, 0);

			if(hFile != INVALID_HANDLE_VALUE)
			{
				resolved.resize(GetFinalPathNameByHandle(hFile, nullptr, 0, FILE_NAME_NORMALIZED) + 1);
				if(GetFinalPathNameByHandle(hFile, (LPSTR)resolved.data(), (DWORD)resolved.size(), FILE_NAME_NORMALIZED) > 0)
				{
					resolved.resize(resolved.size() - 1);
					resolved = genericPath(resolved);
				}
				CloseHandle(hFile);
			}
#else // _WIN32
			struct stat info;

			// check if lstat succeeded
			if(lstat(path.c_str(), &info) == 0)
			{
				resolved.resize(info.st_size);
				if(readlink(path.c_str(), (char*)resolved.data(), resolved.size()) > 0)
					resolved = genericPath(resolved);
			}
#endif // _WIN32

			// return resolved path
			return resolved;

		} // resolveSymlink

		std::string getParent(const std::string& _path)
		{
			std::string path   = genericPath(_path);
			size_t      offset = std::string::npos;

			// find last '/' and erase it
			if((offset = path.find_last_of('/')) != std::string::npos)
				return path.erase(offset);

			// no parent found
			return path;

		} // getParent

		std::string getFileName(const std::string& _path)
		{
			std::string path   = genericPath(_path);
			size_t      offset = std::string::npos;

			// find last '/' and return the filename
			if((offset = path.find_last_of('/')) != std::string::npos)
				return ((path[offset + 1] == 0) ? "." : std::string(path, offset + 1));

			// no '/' found, entire path is a filename
			return path;

		} // getFileName

		std::string getStem(const std::string& _path)
		{
			std::string fileName = getFileName(_path);
			size_t      offset   = std::string::npos;

			// empty fileName
			if(fileName == ".")
				return fileName;

			// find last '.' and erase the extension
			if((offset = fileName.find_last_of('.')) != std::string::npos)
				return fileName.erase(offset);

			// no '.' found, filename has no extension
			return fileName;

		} // getStem

		std::string getExtension(const std::string& _path)
		{
			std::string fileName = getFileName(_path);
			size_t      offset   = std::string::npos;

			// empty fileName
			if(fileName == ".")
				return fileName;

			// find last '.' and return the extension
			if((offset = fileName.find_last_of('.')) != std::string::npos)
				return std::string(fileName, offset);

			// no '.' found, filename has no extension
			return ".";

		} // getExtension

		bool removeFile(const std::string& _path)
		{
			std::string path = genericPath(_path);

			// don't remove if it doesn't exists
			if(!exists(path))
				return true;

			// try to remove file
			return (unlink(path.c_str()) == 0);

		} // removeFile

		bool createDirectory(const std::string& _path)
		{
			std::string path = genericPath(_path);

			// don't create if it already exists
			if(exists(path))
				return true;

			// try to create directory
			if(mkdir(path.c_str(), 0755) == 0)
				return true;

			// failed to create directory, try to create the parent
			std::string parent = getParent(path);

			// only try to create parent if it's not identical to path
			if(parent != path)
				createDirectory(parent);

			// try to create directory again now that the parent should exist
			return (mkdir(path.c_str(), 0755) == 0);

		} // createDirectory

		bool exists(const std::string& _path)
		{
			std::string path = genericPath(_path);
			struct stat info;

			// check if stat succeeded
			return (stat(path.c_str(), &info) == 0);

		} // exists

		bool isAbsolute(const std::string& _path)
		{
			std::string path = genericPath(_path);

#if defined(_WIN32)
			return ((path.size() > 1) && (path[1] == ':'));
#else // _WIN32
			return ((path.size() > 0) && (path[0] == '/'));
#endif // _WIN32

		} // isAbsolute

		bool isRegularFile(const std::string& _path)
		{
			std::string path = genericPath(_path);
			struct stat info;

			// check if stat succeeded
			if(stat(path.c_str(), &info) != 0)
				return false;

			// check for S_IFREG attribute
			return (S_ISREG(info.st_mode));

		} // isRegularFile

		bool isDirectory(const std::string& _path)
		{
			std::string path = genericPath(_path);
			struct stat info;

			// check if stat succeeded
			if(stat(path.c_str(), &info) != 0)
				return false;

			// check for S_IFDIR attribute
			return (S_ISDIR(info.st_mode));

		} // isDirectory

		bool isSymlink(const std::string& _path)
		{
			std::string path = genericPath(_path);

#if defined(_WIN32)
			// check for symlink attribute
			const DWORD Attributes = GetFileAttributes(path.c_str());
			if((Attributes != INVALID_FILE_ATTRIBUTES) && (Attributes & FILE_ATTRIBUTE_REPARSE_POINT))
				return true;
#else // _WIN32
			struct stat info;

			// check if lstat succeeded
			if(lstat(path.c_str(), &info) != 0)
				return false;

			// check for S_IFLNK attribute
			return (S_ISLNK(info.st_mode));
#endif // _WIN32

			// not a symlink
			return false;

		} // isSymlink

		bool isHidden(const std::string& _path)
		{
			std::string path = genericPath(_path);

#if defined(_WIN32)
			// check for hidden attribute
			const DWORD Attributes = GetFileAttributes(path.c_str());
			if((Attributes != INVALID_FILE_ATTRIBUTES) && (Attributes & FILE_ATTRIBUTE_HIDDEN))
				return true;
#endif // _WIN32

			// filenames starting with . are hidden in linux, we do this check for windows as well
			if(getFileName(path)[0] == '.')
				return true;

			// not hidden
			return false;

		} // isHidden

		bool isEquivalent(const std::string& _path1, const std::string& _path2)
		{
			std::string path1 = genericPath(_path1);
			std::string path2 = genericPath(_path2);
			struct stat info1;
			struct stat info2;

			// check if stat succeeded
			if((stat(path1.c_str(), &info1) != 0) || (stat(path2.c_str(), &info2) != 0))
				return false;

			// check if attributes are identical
			return ((info1.st_dev == info2.st_dev) && (info1.st_ino == info2.st_ino) && (info1.st_size == info2.st_size) && (info1.st_mtime == info2.st_mtime));

		} // isEquivalent

	} // FileSystem::

} // Utils::
