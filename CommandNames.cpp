#pragma once
#include <string>

class CommandNames
{

    public:

        CommandNames() = delete;
        CommandNames(const CommandNames& ptt) = delete;
        CommandNames(const CommandNames&& ptt) = delete;
        CommandNames& operator=(const CommandNames& ptt) = delete;
        CommandNames& operator=(const CommandNames&& ptt) = delete;

        // Command names.
        inline static const std::string PRIV_MSG{"pm"};

};