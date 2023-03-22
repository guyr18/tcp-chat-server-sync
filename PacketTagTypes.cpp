#pragma once
#include <string>

class PacketTagTypes
{

    public:

        PacketTagTypes() = delete;
        PacketTagTypes(const PacketTagTypes& ptt) = delete;
        PacketTagTypes(const PacketTagTypes&& ptt) = delete;
        PacketTagTypes& operator=(const PacketTagTypes& ptt) = delete;
        PacketTagTypes& operator=(const PacketTagTypes&& ptt) = delete;

        // Packet tag types
        inline static const std::string PKT_NICKNAME{"%n%"};
        inline const static std::string PKT_MESSAGE{"%m%"};
        inline const static std::string PKT_PING{"%p%"};
        inline const static std::string PKT_PM{"%v%"};

};