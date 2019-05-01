#include "stringUtils.h"
#include "../core/Object.h"
#include <algorithm>
#include <filesystem>
#include <string.h>

#ifdef OXYGINE_SDL
#include "SDL_stdinc.h"
#endif

namespace oxygine
{
    namespace path
    {
        bool unitTest()
        {
            char dest[255];

            normalize("//", dest);
            OX_ASSERT(strcmp(dest, "/") == 0);

            normalize("ab/cd/../qw", dest);
            OX_ASSERT(strcmp(dest, "ab/qw") == 0);
            normalize("ab/cd/ef/../../qw", dest);
            OX_ASSERT(strcmp(dest, "ab/qw") == 0);


            //normalize("rom://abc", dest);
            //OX_ASSERT(strcmp(dest, "rom://abc") == 0);

            normalize("/../", dest);
            OX_ASSERT(strcmp(dest, "/") == 0);

            normalize("../c\\", dest);
            OX_ASSERT(strcmp(dest, "../c/") == 0);

            normalize("a//", dest);
            OX_ASSERT(strcmp(dest, "a/") == 0);


            normalize("\\/\\///\\/", dest);// \n
            OX_ASSERT(strcmp(dest, "/") == 0);

            normalize("a/b/..\\//..///\\/../c\\\\/", dest);
            OX_ASSERT(strcmp(dest, "../c/") == 0);

            normalize("a/../c", dest);
            OX_ASSERT(strcmp(dest, "c") == 0);

            normalize("a/b/../../../c", dest);
            OX_ASSERT(strcmp(dest, "../c") == 0);

            normalize("..a/b/..\\//..///\\/../c\\\\/", dest);
            OX_ASSERT(strcmp(dest, "../c/") == 0);

            normalize("../../a", dest);
            OX_ASSERT(strcmp(dest, "../../a") == 0);

            return true;
        }

#ifdef OX_DEBUG
        bool b = unitTest();
#endif

        void reverse(char* str)
        {
            int len = (int)strlen(str);
            int half = len / 2;
            for (int i = 0; i < half; ++i)
            {
                std::swap(str[i], str[len - i - 1]);
            }
        }

        void split(const char* src, char* destHead, char* destTail)
        {
            OX_ASSERT(src != destHead);
            OX_ASSERT(src != destTail);

            destHead[0] = 0;
            destTail[0] = 0;

            int i = 0;
            int len = (int)strlen(src);
            while (len)
            {
                char c = src[len - 1];
                if (c == '\\' || c == '/')
                {
                    break;
                }

                destTail[i] = c;
                ++i;
                --len;
            }
            destTail[i] = 0;
            reverse(destTail);

            memcpy(destHead, src, len);
            destHead[len] = 0;
        }

        void splitExt(const char* src, char* destHead, char* destTail)
        {
            OX_ASSERT(src != destHead);
            OX_ASSERT(src != destTail);

            destHead[0] = 0;
            destTail[0] = 0;

            int len = (int)strlen(src);
            int i = 0;
            for (i = len - 1; i >= 0; --i)
            {
                if (src[i] == '.')
                    break;
            }

            if (i == -1)
                i = len;
            memcpy(destHead, src, i);
            destHead[i] = 0;

            int n = len - i;
            memcpy(destTail, src + i, n);
            destTail[n] = 0;
        }

        void normalize(const char* src, char* dest)
        {
            std::filesystem::path srcPath = std::filesystem::u8path(src);
            std::string destPath = srcPath.lexically_normal().u8string();
            std::replace(destPath.begin(), destPath.end(), '\\', '/');
            strcpy(dest, destPath.c_str());
        }

        std::string normalize(const std::string& pth)
        {
            char path[512];
            normalize(pth.c_str(), path);
            return path;
        }

        std::string extractFileName(const std::string& path)
        {
            size_t pos = path.find_last_of("\\/") + 1;
            std::string name = path.substr(pos, path.size() - pos);
            return name;
        }

        std::string extractBaseFileName(const std::string& filename)
        {
            size_t pos = filename.find_last_of('.');
            if (pos == std::string::npos)
                pos = filename.size();
            std::string name = filename.substr(0, pos);
            return name;
        }

        std::string extractFileExt(const std::string& filename)
        {
            size_t pos = filename.find_last_of(".");
            if (pos == std::string::npos)
                return "";
            pos += 1;
            std::string name = filename.substr(pos, filename.size() - pos);
            return name;
        }

        std::string extractFolder(const std::string& path)
        {
            size_t pos = path.find_last_of("\\/") + 1;
            std::string name = path.substr(0, pos);
            return name;
        }
    }


    const char* getNextCode(int& code, const char* utf8str)
    {
        const char* utfstr = utf8str;
        code = 0;
        char* p = (char*)&code;

        const char& c = *utfstr++;
        p[0] = c;

        if ((c & 0xE0) == 0xE0)
        {
            p[1] = *utfstr++;
            p[2] = *utfstr++;
        }
        else if ((c & 0xC0) == 0xC0)
        {
            p[1] = *utfstr++;
        }

        return utfstr;
    }

    void charCode2Bytes(std::string& res, int code)
    {
        const char* p = (char*)&code;
        const char& c = *p;

        res.push_back(*p++);
        if ((c & 0xE0) == 0xE0)
        {
            res.push_back(*p++);
            res.push_back(*p++);
        }
        else if ((c & 0xC0) == 0xC0)
        {
            res.push_back(*p++);
        }
    }

    std::string lower(const std::string& str)
    {
        std::string data = str;
        std::transform(data.begin(), data.end(), data.begin(), ::tolower);//todo optimize
        return data;
    }

    std::wstring utf8tows_simple(const char* utf8str)
    {
        static bool warned = false;
        if (warned)
        {
            warned = true;
            logs::warning("utf8tows not implemented correctly!");
        }

        std::wstring ws;

        int code = 0;
        while (true)
        {
            utf8str = getNextCode(code, utf8str);
            if (!code)
                break;
            ws.push_back(code);
        }

        return ws;
    }

    std::wstring utf8tows(const char* utf8str)
    {
        if (!utf8str)
            return L"";


        int n = (int)strlen(utf8str) + 1;
        if (n == 1)
            return L"";


#if defined(OXYGINE_SDL) && !defined(EMSCRIPTEN)
        wchar_t* s = 0;
        if (sizeof(wchar_t) == 2)
            s = (wchar_t*)SDL_iconv_string("UCS-2-INTERNAL", "UTF-8", utf8str, n);
        else
            s = (wchar_t*)SDL_iconv_string("UCS-4-INTERNAL", "UTF-8", utf8str, n);

        if (s == 0)
            return utf8tows_simple(utf8str);

        std::wstring str = s;
        str.reserve(n);
        SDL_free(s);
        return str;
#else
        return utf8tows_simple(utf8str);
#endif
    }

    std::string ws2utf8_simple(const wchar_t* wstr)
    {
        static bool warned = false;
        if (warned)
        {
            warned = true;
            logs::warning("utf8tows not implemented correctly!");
        }

        std::string s;
        int i = 0;
        while (wchar_t t = wstr[i])
        {
            ++i;
            charCode2Bytes(s, t);
        }
        return s;
    }

    std::string ws2utf8(const wchar_t* wstr)
    {
        if (!wstr)
            return "";

        int n = (int)wcslen(wstr) + 1;
        if (n == 1)
            return "";

#if defined(OXYGINE_SDL) && !defined(EMSCRIPTEN)
        char*  s = 0;
        if (sizeof(wchar_t) == 2)
            s = SDL_iconv_string("UTF-8", "UCS-2-INTERNAL", (const char*)wstr, n * sizeof(wchar_t));
        else
            s = SDL_iconv_string("UTF-8", "UCS-4-INTERNAL", (const char*)wstr, n * sizeof(wchar_t));

        if (s == 0)
            return ws2utf8_simple(wstr);

        std::string str = s;
        SDL_free(s);
        return str;
#else
        return ws2utf8_simple(wstr);
#endif
    }

    Color   hex2color(const char* str)
    {
        int len = (int)strlen(str);
        if (len >= 6)
        {

            int rgba = 0;
            sscanf(str, "%x", &rgba);
            if (len == 6)
                rgba = (rgba << 8) | 0xff;

            return Color(rgba);
        }

        return Color(0xffffffff);
    }

    std::string color2hex(const Color& c)
    {
        char str[255];
        safe_sprintf(str, "%02x%02x%02x%02x", c.r, c.g, c.b, c.a);
        return std::string(str);
    }

    int strcmp_cns(const char* fileName1, const char* fileName2)
    {
        for (;;)
        {
            char c1 = *(fileName1++);
            char c2 = *(fileName2++);
            if ((c1 >= 'a') && (c1 <= 'z'))
                c1 -= 0x20;
            if ((c2 >= 'a') && (c2 <= 'z'))
                c2 -= 0x20;
            if (c1 == '\0')
                return ((c2 == '\0') ? 0 : -1);
            if (c2 == '\0')
                return 1;
            if (c1 < c2)
                return -1;
            if (c1 > c2)
                return 1;
        }
        return 1;
    }
}