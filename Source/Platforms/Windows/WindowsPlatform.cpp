#include "Header/Platforms/Windows/WindowsPlatform.h"
#include <array>
#include <ShlObj_core.h>
#include <tchar.h>

std::filesystem::path WindowsPlatform::GetSaveFolder()
{
    BROWSEINFO browseInfo{};
    browseInfo.ulFlags = BIF_RETURNONLYFSDIRS | BIF_USENEWUI;
    browseInfo.lpszTitle = _T("Select a folder to save to");
    
    const LPITEMIDLIST pidl(SHBrowseForFolder(&browseInfo));
    if (pidl != nullptr)
    {
        std::array<TCHAR, MAX_PATH> path{};
        SHGetPathFromIDList(pidl, path.data());

        IMalloc* imalloc = nullptr;
        if (SUCCEEDED(SHGetMalloc(&imalloc)))
        {
            imalloc->Free(pidl);
            imalloc->Release();
        }

        return std::filesystem::path(path.data());
    }

    return std::filesystem::path();
}

std::filesystem::path WindowsPlatform::GetSavePath(const std::string_view& filename)
{
    std::filesystem::path savePath(filename);
    std::array<TCHAR, MAX_PATH> path{};

    OPENFILENAME ofn{};
    ofn.lStructSize = sizeof ofn;
    ofn.hwndOwner = nullptr;
    ofn.lpstrFile = path.data();
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_EXPLORER;

#ifdef _UNICODE
    const auto extStr(savePath.extension().wstring());
    ofn.lpstrFilter = extStr.c_str();
    ofn.lpstrDefExt = extStr.substr(1, extStr.length()).c_str();
#else
    const auto extStr(savePath.extension().string());
    ofn.lpstrFilter = extStr.c_str();
    ofn.lpstrDefExt = extStr.substr(1, extStr.length()).c_str();
#endif
    
    if (GetSaveFileName(&ofn) != 0)
    {
        return savePath;
    }

    return std::filesystem::path();
}