#pragma once

#include <Windows.h>
#include <Shlwapi.h>
#include <ShlObj.h>
#include <locale.h>
#include <cctype>
#include <io.h>
#include <stdio.h>
#include <direct.h>  
#include <stdlib.h>

#include <functional>
#include <string>
#include <ostream>
#include <sstream>
#include <ctime>
#include <chrono>

#include "sol.hpp"
#include "pugixml.hpp"
#include "pugiconfig.hpp"
#include "octane_lua_api.h"

#include "zip.h"
#include "unzip.h"
#include "compress.hpp"
#include "decompress.hpp"

#include <gdiplus.h>
#include "easylogging++.h"

#pragma comment(lib, "gdiplus.lib")
#pragma comment(lib, "Shlwapi.lib")

constexpr int BUFSIZE = 1024;

namespace octane_plug_utils {
    
    static int  GetAllFilesPath(const std::string& path, std::vector<std::string>& fullPathNames);

    inline void build_string(std::ostream& o) {}

    template<class First, class... Rest>
    inline void build_string(std::ostream& o,
        const First& value,
        const Rest&... rest)
    {
        o << value;
        build_string(o, rest...);
    }

    template<class... T>
    std::string concat_string(const T&... value)
    {
        std::ostringstream o;
        build_string(o, value...);
        return o.str();
    }

    // 声明以便定义其后的函数调用
    static std::string pair_print(sol::object key, sol::object value);

    static bool CheckType(sol::object v, sol::type target_type)
    {
        if (v.get_type() == target_type)
        {
            return true;
        }
        else
        {
            return false;
        }
    }

    static std::string GetTypeAsString(sol::object v)
    {
        switch (v.get_type())
        {
        case sol::type::table:
            return "table";
        case sol::type::function:
            return "function";
        case sol::type::userdata:
            return "userdata";
        case sol::type::boolean:
            return "boolean";
        case sol::type::lightuserdata:
            return "lightuserdata";
        case sol::type::nil:
            return "nil";
        case sol::type::none:
            return "none";
        case sol::type::string:
            return "string";
        case sol::type::number:
            return "number";
        case sol::type::thread:
            return "thread";
        default:
            return "unknow";
        }
    }

    static std::string TraverseTable(sol::table t, std::function<std::string(sol::object, sol::object)> pair_processor)
    {
        if (!t.valid() || t.empty())
        {
            return "";
        }

        std::string all_in_string{};

        for (auto& kv : t)
        {
            sol::object key = kv.first;
            sol::object value = kv.second;
            all_in_string += pair_processor(key, value);
            all_in_string += "\n";
        }

        return all_in_string;
    }

    static std::string flat_print(sol::object v)
    {
        std::string format_result{};

        sol::type v_type = v.get_type();
        switch (v_type)
        {
        case sol::type::boolean:
            format_result += concat_string<bool>(v.as<bool>());
            return format_result;
        case sol::type::number:
            format_result += concat_string<double>(v.as<double>());
            return format_result;
        case sol::type::string:
            format_result += concat_string<std::string>(v.as<std::string>());
            return format_result;
        case sol::type::function:
            format_result += "function";
            return format_result;
        case sol::type::table:
            format_result += "\n\t";
            format_result += TraverseTable(v, pair_print);
            return format_result;
        case sol::type::userdata:
            format_result += "userdata";
            return format_result;
        case sol::type::nil:
            format_result += "nil";
            return format_result;
        case sol::type::thread:
            format_result += "thread";
            return format_result;
        default:
            format_result += "unknow_value";
            return format_result;
        }
    }

    static std::string pair_print(sol::object key, sol::object value)
    {
        std::string key_value_string{};
        key_value_string += "[";
        key_value_string += flat_print(key);
        key_value_string += " => ";
        key_value_string += flat_print(value);
        key_value_string += "]";
        return key_value_string;
    }

    static std::string get_time_stamp(const std::string& timeFormatString)
    {
        std::array<char, 40> buf{};
        time_t raw_time;
        struct tm* time_info = nullptr;
        time(&raw_time);
        time_info = localtime(&raw_time);
        size_t len = strftime(buf.data(), buf.size(), timeFormatString.c_str(), time_info);
        if (len == 0)
        {
            return "";
        }
        else
        {
            return std::string(std::begin(buf), std::end(buf));
        }
    }

    static bool IsFileExists(const std::string& filePath)
    {
        DWORD dwAttrib = GetFileAttributes(filePath.c_str());
        return dwAttrib != INVALID_FILE_ATTRIBUTES && !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY);
    }

    static bool IsDirExist(const std::string& directoryName)
    {
        DWORD file_type = GetFileAttributesA(directoryName.c_str());
        if (file_type == INVALID_FILE_ATTRIBUTES)
        {
            return false;
        }

        if (file_type & FILE_ATTRIBUTE_DIRECTORY)
        {
            return true;
        }

        return false;
    }

    static std::string preprocess(const std::string& foldPath)
    {
        size_t len = foldPath.length();
        if (foldPath[len - 1] == '\\')
        {
            return foldPath.substr(0, len - 1);
        }
        else
        {
            return foldPath;
        }
    }

    static std::string get_parent_path(const std::string& folderPath)
    {
        std::string folder_path = preprocess(folderPath);
        if (folder_path.empty())
        {
            return "";
        }
        size_t index = folder_path.find_last_of('\\');
        if (index == std::string::npos)
        {
            return "";
        }
        return folder_path.substr(0, index);
    }

    static bool CreateFolder(const std::string& foldPath)
    {
        if (foldPath.empty())
        {
            return false;
        }

        std::string inner_folder_path = preprocess(foldPath);
        size_t last_backslash_index = inner_folder_path.find_last_of("\\");

        if (last_backslash_index == std::string::npos)
        {
            if (!IsDirExist(inner_folder_path))
            {
                CreateDirectory(inner_folder_path.c_str(), nullptr);
            }

        }
        else
        {
            std::string parent_dir = inner_folder_path.substr(0, last_backslash_index);
            std::cout << parent_dir << std::endl;
            if (IsDirExist(parent_dir))
            {
                CreateDirectory(inner_folder_path.c_str(), nullptr);
            }
            else
            {
                CreateFolder(parent_dir);
                CreateDirectory(foldPath.c_str(), nullptr);
            }
        }

        return true;
    }


    static BOOL GetDriverInfo(LPSTR szDrive, std::vector<std::string>& driverRootPaths)
    {
        UINT uDriverType;

        uDriverType = GetDriveType(szDrive);

        switch (uDriverType)
        {
        case DRIVE_UNKNOWN:
            break;
        case DRIVE_NO_ROOT_DIR:
            break;
        case DRIVE_REMOVABLE:
            break;
        case DRIVE_FIXED:
            driverRootPaths.push_back(std::string(szDrive));
            break;
        case DRIVE_REMOTE:
            break;
        case DRIVE_CDROM:
            break;
        case DRIVE_RAMDISK:
            break;
        default:
            break;
        }

        return TRUE;

    }

    static std::string find_first_available_driver()
    {
        CHAR szLogicDriveStrings[BUFSIZE];
        PCHAR szDrive;

        ZeroMemory(szLogicDriveStrings, BUFSIZE);

        GetLogicalDriveStrings(BUFSIZE - 1, szLogicDriveStrings);
        szDrive = (PCHAR)szLogicDriveStrings;
        std::vector<std::string> diskDrivePaths{};
        do
        {
            if (!GetDriverInfo(szDrive, diskDrivePaths))
            {
                printf("\nGet Volume Information Error:%d", GetLastError());
            }
            szDrive += (lstrlen(szDrive) + 1);
        } while (*szDrive != '\x00');

        if (!diskDrivePaths.empty())
        {
            return diskDrivePaths[0].c_str();
        }

        return "";
    }

    static std::string GetUTC()
    {
        auto timestamp = std::chrono::seconds(std::time(nullptr));
        long long seconds = std::chrono::seconds(timestamp).count();
        return std::to_string(seconds);
    }

    static int create_multi_level_dir(char* pDir)
    {
        int i = 0;
        int iRet;
        std::size_t iLen;
        char* pszDir;

        if (NULL == pDir)
        {
            return 0;
        }

        pszDir = _strdup(pDir);
        iLen = strlen(pszDir);

        for (i = 0; i < iLen; i++)
        {
            if (pszDir[i] == '\\' || pszDir[i] == '/')
            {
                pszDir[i] = '\0';

                iRet = _access(pszDir, 0);
                if (iRet != 0)
                {
                    iRet = _mkdir(pszDir);
                    if (iRet != 0)
                    {
                        return -1;
                    }
                }
                pszDir[i] = '/';
            }
        }

        iRet = _mkdir(pszDir);
        free(pszDir);
        return iRet;
    }

    static std::string extract_zip_file(const std::string& src_file)
    {
        ziputils::unzipper zipFile;
        if (zipFile.open(src_file.c_str()))
        {
            std::string dir_name = src_file.substr(0, src_file.length() - 4);
            if (_mkdir(dir_name.c_str()) != ENOENT) 
            {
                auto filenames = zipFile.getFilenames();
                for (auto it = filenames.begin(); it != filenames.end(); it++) 
                {
                    std::string item = *it;
                    std::ofstream middleFile;
                    std::size_t index = item.rfind("/");
                    if (index != std::string::npos) 
                    {
                        std::string sub_folder = item.substr(0, index);
                        std::replace(sub_folder.begin(), sub_folder.end(), '/', '\\');
                        std::string full_folder = dir_name + "\\" + sub_folder;
                        char folder[128];
                        memset(folder, 0, sizeof(folder));
                        strncpy(folder, full_folder.c_str(), 128);
                        create_multi_level_dir(folder);
                        std::replace(item.begin(), item.end(), '/', '\\');
                        middleFile.open((dir_name + "\\" + item).c_str(), std::ostream::binary);
                    }
                    else 
                    {
                        middleFile.open((dir_name + "\\" + item).c_str(), std::ostream::binary);
                    }

                    if (zipFile.openEntry((*it).c_str())) 
                    {
                        zipFile >> middleFile;
                        middleFile.close();
                    }
                    else 
                    {
                        return "";
                    }
                }
                zipFile.close();
                return dir_name;
            }
            else 
            {
                zipFile.close();
                return "";
            }
        }
        else 
        {
            return "";
        }
    }

    static std::string compress_folder(const std::string& sourceFolder, const std::string& fileName)
    {
        struct stat info;
        if (stat(sourceFolder.c_str(), &info) == 0 && info.st_mode & S_IFDIR) 
        {
            std::vector<std::string> file_name_list;
            GetAllFilesPath(sourceFolder.c_str(), file_name_list);
            std::size_t file_counter = file_name_list.size();

            ziputils::zipper zipFile;
            std::size_t index = sourceFolder.find_last_of("/\\");
            std::string father_folder = sourceFolder.substr(0, index + 1);
            //zipFile.open(father_folder.append("item.zip").c_str());
            std::string zip_file_path = father_folder.append(fileName);
            zipFile.open(zip_file_path.c_str());
            // add files to the zip file
            for (auto i = 0; i < file_counter; i++) 
            {
                if (stat(file_name_list[i].c_str(), &info) == 0 && info.st_mode & S_IFDIR)
                {
                    continue;
                }

                std::ifstream file(file_name_list[i], std::ios::in | std::ios::binary);
                if (file.is_open()) 
                {
                    std::string entryItem = file_name_list[i].substr(sourceFolder.length(), file_name_list[i].length());
                    std::replace(entryItem.begin(), entryItem.end(), '\\', '/');
                    zipFile.addEntry(entryItem.c_str());
                    zipFile << file;
                }
            }
            zipFile.close();
            return zip_file_path;
        }
        else 
        {
            LOG(INFO) << "The folder for compress is error!!!";
            return "";
        }

    }

    static bool IsDirContained(const std::string& dirPath, const std::string& toCheckDirName)
    {
        auto dir_path = preprocess(dirPath);
        auto to_check_dir_path = dir_path + "\\" + toCheckDirName;
        return IsDirExist(to_check_dir_path);
    }

    static int  GetAllFilesPath(const std::string& path, std::vector<std::string>& fullPathNames)
    {
        WIN32_FIND_DATAA FindData;
        HANDLE hFileSearch;
        int FileCount = 0;
        char FilePathName[BUFSIZE];
        char FullPathName[BUFSIZE];
        strcpy_s(FilePathName, path.c_str());
        strcat_s(FilePathName, "\\*.*");
        hFileSearch = FindFirstFileA(FilePathName, &FindData);
        if (hFileSearch == INVALID_HANDLE_VALUE) 
        {
            //std::cout << "Open file handle error !!!" << std::endl;
            return -1;
        }

        while (::FindNextFileA(hFileSearch, &FindData)) 
        {
            if (strcmp(FindData.cFileName, ".") == 0
                || strcmp(FindData.cFileName, "..") == 0)
            {
                continue;
            }

            sprintf_s(FullPathName, "%s\\%s", path.c_str(), FindData.cFileName);
            FileCount++;
            fullPathNames.push_back(FullPathName);

            if (FindData.dwFileAttributes& FILE_ATTRIBUTE_DIRECTORY) 
            {
                int res = GetAllFilesPath(FullPathName, fullPathNames);
                if (res >= 0)
                {
                    FileCount += res;
                }
                else
                {
                    return -1;
                }
            }
        }

        return FileCount;
    }

    static int CreateAndMove(const std::string& srcDir, const std::string& toCheckDirName)
    {
        if (IsDirContained(srcDir, toCheckDirName))
        {
            //LOG(INFO) << "Fuck dir contains the Octane dir" << std::endl;
        }
        else
        {
            //LOG(INFO) << "Not contain the octane dir, so create and move" << std::endl;
            std::vector<std::string> file_names;
            GetAllFilesPath(srcDir, file_names);

            CreateDirectory((srcDir + "/" + toCheckDirName).c_str(), NULL);
            for (auto& file : file_names)
            {
                auto second_level_name = file.substr(srcDir.length(), file.length());
                std::string new_name = srcDir + "/" + toCheckDirName + second_level_name;
                MoveFile(file.c_str(), new_name.c_str());
            }
        }

        return 0;
    }

    static struct tm* GmTime(time_t t, struct tm* tm_result)
    {
        gmtime_s(tm_result, &t);
        return tm_result;
    }

    static std::string get_gmt_time(time_t t)
    {
        char buffer[128] = "";
        struct tm tm_result;
        GmTime(t, &tm_result);
        strftime(buffer, sizeof(buffer), "%a, %d %b %Y %H:%M:%S GMT", &tm_result);
        return buffer;
    }

    // trim from start
    static inline std::string& ltrim(std::string& s)
    {
        s.erase(s.begin(), std::find_if(s.begin(), s.end(),
            std::not1(std::ptr_fun<int, int>(std::isspace))));
        return s;
    }

    // trim from end
    static inline std::string& rtrim(std::string& s)
    {
        s.erase(std::find_if(s.rbegin(), s.rend(),
            std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
        return s;
    }

    // trim from both ends
    static inline std::string& trim(std::string& s)
    {
        return ltrim(rtrim(s));
    }

    static inline int rename_file(const std::string& oldFullPathName, const std::string& newFullPathName)
    {
        return MoveFileEx(oldFullPathName.c_str(), newFullPathName.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_COPY_ALLOWED);
    }

    static BOOL ApplyFunctionToFile(const std::string& srcFolder,
        std::map<std::string, std::string> modifyContent,
        std::function<void(const std::string& filePath,
            std::map<std::string, std::string>)> modifier)
    {
        WIN32_FIND_DATA FindFileData;
        HANDLE hFind;
        char l_szTmp[1025] = { 0 };
        std::cout << srcFolder.c_str() << std::endl;
        memcpy(l_szTmp, srcFolder.data(), srcFolder.size());


        char l_szSrcPath[1025] = { 0 };
        memcpy(l_szSrcPath, srcFolder.data(), srcFolder.size());

        char l_szNewSrcPath[1025] = { 0 };

        strcat(l_szTmp, "\\*");

        hFind = FindFirstFile(l_szTmp, &FindFileData);

        if (hFind == NULL || hFind == INVALID_HANDLE_VALUE)
        {
            return FALSE;
        }

        do
        {
            if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
                if (strcmp(FindFileData.cFileName, "."))
                {
                    if (strcmp(FindFileData.cFileName, ".."))
                    {
                        printf("The Directory found is %s\n", FindFileData.cFileName);

                        sprintf(l_szNewSrcPath, "%s\\%s", l_szSrcPath, FindFileData.cFileName);
                        ApplyFunctionToFile(l_szNewSrcPath, modifyContent, modifier);
                    }
                }
            }
            else
            {
                printf("The File found is %s\n", FindFileData.cFileName);
                char l_szSrcFile[1025] = { 0 };
                char l_szDesFile[1025] = { 0 };
                sprintf(l_szSrcFile, "%s\\%s", l_szSrcPath, FindFileData.cFileName);
                modifier(l_szSrcFile, modifyContent);
            }
        } while (FindNextFile(hFind, &FindFileData));

        FindClose(hFind);
        return TRUE;
    }

    static std::string get_file_name_from_fullpath(const std::string& fullPathName)
    {
        if (fullPathName.empty())
        {
            return "";
        }
        std::string formal_path = preprocess(fullPathName);
        size_t index = formal_path.find_last_of('\\');
        if (index == std::string::npos)
        {
            return "";
        }
        return formal_path.substr(index + 1);
    }

    static std::string get_file_ext_with_dot(const std::string& fileName)
    {
        if (fileName.empty())
        {
            return "";
        }
        size_t index = fileName.find_last_of('.');
        if (index == std::string::npos)
        {
            return "";
        }
        return fileName.substr(index);
    }

    static std::map<std::wstring, std::wstring> MimeTypeMap{
        { L".jpeg", L"image/jpeg" },
        { L".jpe", L"image/jpeg" },
        { L".jpg", L"image/jpeg" },
        { L".png", L"image/png" },
        { L".gif", L"image/gif" },
        { L".tiff", L"image/tiff" },
        { L".tif", L"image/tiff" },
        { L".bmp", L"image/bmp" }
    };


    static Gdiplus::Bitmap* ResizeClone(Gdiplus::Bitmap* bmp, int width, int height)
    {
        UINT image_height = bmp->GetHeight();
        UINT image_width = bmp->GetWidth();
        UINT target_height = 0;
        UINT target_width = 0;

        double ratio = static_cast<double>(image_height) / static_cast<double>(image_width);
        if (image_height > image_width)
        {
            target_height = 1024;
            target_width = static_cast<UINT>(static_cast<double>(1024) / ratio);
        }
        else
        {
            target_width = 1024;
            target_height = static_cast<UINT>(static_cast<double>(target_width) * ratio);
        }
        Gdiplus::Bitmap* new_bmp = new Gdiplus::Bitmap(target_width, target_height, bmp->GetPixelFormat());
        Gdiplus::Graphics graphics(new_bmp);
        graphics.DrawImage(bmp, 0, 0, target_width, target_height);

        return new_bmp;
    }

    static int GetGdiplusEncoder(const std::wstring& form, CLSID* clsid)
    {
        UINT num;
        UINT size;
        Gdiplus::ImageCodecInfo* imageCodecInfo = nullptr;
        Gdiplus::GetImageEncodersSize(&num, &size);
        if (size == 0)
        {
            return -1;
        }
        imageCodecInfo = static_cast<Gdiplus::ImageCodecInfo*>(malloc(size));
        if (imageCodecInfo == nullptr)
        {
            return -1;
        }
        Gdiplus::GetImageEncoders(num, size, imageCodecInfo);

        for (UINT i = 0; i < num; i++)
        {
            if (wcscmp(imageCodecInfo[i].MimeType, form.c_str()) == 0)
            {
                *clsid = imageCodecInfo[i].Clsid;
                return i;
            }
        }
        free(imageCodecInfo);

        return -1;
    }

    static int convert_image(const std::string& source_file_path, const std::string& target_file_path)
    {
        ULONG_PTR gdiplusToken;
        Gdiplus::GdiplusStartupInput gdiplusStartupInput;
        Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

        std::wstring image_file_name;
        image_file_name.assign(source_file_path.begin(), source_file_path.end());
        std::wstring target_image_file_name;
        target_image_file_name.assign(target_file_path.begin(), target_file_path.end());
        Gdiplus::Bitmap* bmp = new Gdiplus::Bitmap(image_file_name.c_str());
        if (bmp->GetHeight() <= 1024 && bmp->GetWidth() <= 1024)
        {
            LOG(INFO) << "Image is small not to process.";
            if (CopyFileA(source_file_path.c_str(), target_file_path.c_str(), FALSE) == 0)
            {
                LOG(INFO) << "Copy the source image error!";
                delete bmp;
                Gdiplus::GdiplusShutdown(gdiplusToken);
                return -1;
            }
            delete bmp;
            Gdiplus::GdiplusShutdown(gdiplusToken);

            return 0;
        }
        Gdiplus::Bitmap* resized_bmp = ResizeClone(bmp, 0, 0);
        LPWSTR ext = PathFindExtensionW(image_file_name.c_str());
        std::wstring form = MimeTypeMap[ext];
        CLSID encId;
        if (GetGdiplusEncoder(form, &encId) > -1)
        {
            resized_bmp->Save(target_image_file_name.c_str(), &encId, nullptr);
        }
        else
        {
            return -1;
        }

        delete bmp;
        delete resized_bmp;
        Gdiplus::GdiplusShutdown(gdiplusToken);

        return 0;
    }
}