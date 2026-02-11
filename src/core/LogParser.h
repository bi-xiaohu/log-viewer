#pragma once

#include "LogFormat.h"
#include "LogEntry.h"
#include <string>
#include <vector>
#include <memory>

namespace logviewer {

// 日志解析器
class LogParser {
public:
    explicit LogParser(const LogFormat& format);
    
    // 解析单条日志
    LogEntry parse(const std::string& line) const;
    
    // 批量解析
    std::vector<LogEntry> parseLines(const std::vector<std::string>& lines) const;
    
    // 从文件解析
    std::vector<LogEntry> parseFile(const std::string& filepath) const;
    
    // 自动检测格式
    struct FormatCandidate {
        std::string delimiter;
        size_t fieldCount;
        double confidence;  // 置信度 0-1
    };
    
    static std::vector<FormatCandidate> detectFormat(const std::vector<std::string>& sampleLines);
    
    // 测试解析结果
    struct ParseTest {
        bool success;
        std::vector<std::string> fields;
        std::string error;
    };
    
    ParseTest testParse(const std::string& line) const;
    
private:
    LogFormat format_;
    
    // 辅助函数
    std::vector<std::string> splitByDelimiter(const std::string& line, const std::string& delimiter) const;
    static size_t countOccurrences(const std::string& str, const std::string& substr);
};

} // namespace logviewer
