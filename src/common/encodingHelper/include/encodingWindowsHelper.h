/*
 * Wazuh shared modules utils
 * Copyright (C) 2015, Wazuh Inc.
 * February 16, 2021.
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General Public
 * License (version 2) as published by the FSF - Free Software
 * Foundation.
 */


#ifdef WIN32

#ifndef _ENCODING_WINDOWS_HELPER_H
#define _ENCODING_WINDOWS_HELPER_H

#include <winsock2.h>
#include <windows.h>
#include <string>
#include <memory>

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
#pragma GCC diagnostic ignored "-Wreturn-type"
#pragma GCC diagnostic ignored "-Wcast-function-type"
#endif

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4505)
#endif

namespace Utils
{
    class EncodingWindowsHelper final
    {
        public:
            static std::string wstringToStringUTF8(const std::wstring& inputArgument)
            {
                std::string retVal;

                if (!inputArgument.empty())
                {
                    const auto inputArgumentSize{static_cast<int>(inputArgument.size())};
                    const auto sizeNeeded {WideCharToMultiByte(CP_UTF8, 0, inputArgument.data(), inputArgumentSize, nullptr, 0, nullptr, nullptr)};
                    const auto buffer{std::make_unique<char[]>(sizeNeeded)};

                    if (WideCharToMultiByte(CP_UTF8, 0, inputArgument.data(), inputArgumentSize, buffer.get(), sizeNeeded, nullptr, nullptr) > 0)
                    {
                        retVal.assign(buffer.get(), sizeNeeded);
                    }
                }

                return retVal;
            }

            static std::wstring stringToWStringAnsi(const std::string& inputArgument)
            {
                std::wstring retVal;

                if (!inputArgument.empty())
                {
                    const auto inputArgumentSize{static_cast<int>(inputArgument.size())};
                    const auto sizeNeeded {MultiByteToWideChar(CP_ACP, 0, inputArgument.data(), inputArgumentSize, nullptr, 0)};
                    const auto buffer{std::make_unique<wchar_t[]>(sizeNeeded)};

                    if (MultiByteToWideChar(CP_ACP, 0, inputArgument.data(), inputArgumentSize, buffer.get(), sizeNeeded) > 0)
                    {
                        retVal.assign(buffer.get(), sizeNeeded);
                    }
                }

                return retVal;
            }

            static std::string stringAnsiToStringUTF8(const std::string& inputArgument)
            {
                return wstringToStringUTF8(stringToWStringAnsi(inputArgument));
            }
    };
}

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif // _ENCODING_WINDOWS_HELPER_H

#endif //WIN32
