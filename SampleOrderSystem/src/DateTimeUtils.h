#pragma once
#include <string>
#include <ctime>

namespace DateTimeUtils {

    inline std::string nowDate() {
        std::time_t t = std::time(nullptr);
        struct tm ts = {};
        localtime_s(&ts, &t);
        char buf[16];
        std::strftime(buf, sizeof(buf), "%Y-%m-%d", &ts);
        return buf;
    }

    inline std::string nowDateCompact() {
        std::time_t t = std::time(nullptr);
        struct tm ts = {};
        localtime_s(&ts, &t);
        char buf[12];
        std::strftime(buf, sizeof(buf), "%Y%m%d", &ts);
        return buf;
    }

    inline std::string nowDateTime() {
        std::time_t t = std::time(nullptr);
        struct tm ts = {};
        localtime_s(&ts, &t);
        char buf[32];
        std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &ts);
        return buf;
    }

    // "YYYY-MM-DD HH:MM:SS" 두 문자열 사이의 경과 시간을 분 단위로 반환
    inline double elapsedMinutes(const std::string& startedAt, const std::string& now) {
        if (startedAt.empty()) return 0.0;

        auto parse = [](const std::string& s) -> std::time_t {
            int Y = 0, M = 0, D = 0, h = 0, m = 0, sec = 0;
            sscanf_s(s.c_str(), "%d-%d-%d %d:%d:%d", &Y, &M, &D, &h, &m, &sec);
            struct tm tm = {};
            tm.tm_year = Y - 1900; tm.tm_mon = M - 1; tm.tm_mday = D;
            tm.tm_hour = h;        tm.tm_min  = m;     tm.tm_sec  = sec;
            tm.tm_isdst = -1;
            return std::mktime(&tm);
        };

        return std::difftime(parse(now), parse(startedAt)) / 60.0;
    }

    // dt에 minutes 분을 더한 "YYYY-MM-DD HH:MM" 문자열 반환
    inline std::string addMinutes(const std::string& dt, double minutes) {
        int Y = 0, M = 0, D = 0, h = 0, m = 0, sec = 0;
        sscanf_s(dt.c_str(), "%d-%d-%d %d:%d:%d", &Y, &M, &D, &h, &m, &sec);
        struct tm tm = {};
        tm.tm_year = Y - 1900; tm.tm_mon = M - 1; tm.tm_mday = D;
        tm.tm_hour = h;        tm.tm_min  = m;     tm.tm_sec  = sec;
        tm.tm_isdst = -1;
        std::time_t t = std::mktime(&tm);
        t += static_cast<std::time_t>(minutes * 60.0);
        struct tm result = {};
        localtime_s(&result, &t);
        char buf[20];
        std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M", &result);
        return buf;
    }

}
