#include "LogParser.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cmath>

namespace logviewer {

LogParser::LogParser(const LogFormat& format) : format_(format) {
}

LogEntry LogParser::parse(const std::string& line) const {
    LogEntry entry(format_.getId(), line);
    
    auto parts = splitByDelimiter(line, format_.getDelimiter());
    const auto& fields = format_.getFields();
    
    // 将解析的部分与字段对应
    size_t minSize = std::min(parts.size(), fields.size());
    for (size_t i = 0; i < minSize; ++i) {
        entry.setField(fields[i].name, parts[i]);
    }
    
    return entry;
}

std::vector<LogEntry> LogParser::parseLines(const std::vector<std::string>& lines) const {
    std::vector<LogEntry> entries;
    entries.reserve(lines.size());
    
    for (const auto& line : lines) {
        if (!line.empty()) {
            entries.push_back(parse(line));
        }
    }
    
    return entries;
}

std::vector<LogEntry> LogParser::parseFile(const std::string& filepath) const {
    std::vector<std::string> lines;
    std::ifstream file(filepath);
    
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file: " + filepath);
    }
    
    std::string line;
    while (std::getline(file, line)) {
        if (!line.empty()) {
            lines.push_back(line);
        }
    }
    
    return parseLines(lines);
}

std::vector<LogParser::FormatCandidate> LogParser::detectFormat(const std::vector<std::string>& sampleLines) {
    if (sampleLines.empty()) {
        return {};
    }
    
    // 常见分隔符
    std::vector<std::string> delimiters = {" ", "\t", ",", "|", ";", ":"};
    std::vector<FormatCandidate> candidates;
    
    for (const auto& delim : delimiters) {
        std::vector<size_t> fieldCounts;
        
        // 统计每行的字段数
        for (const auto& line : sampleLines) {
            size_t count = countOccurrences(line, delim) + 1;
            fieldCounts.push_back(count);
        }
        
        // 计算字段数的一致性
        if (fieldCounts.empty()) continue;
        
        // 找出最常见的字段数
        std::sort(fieldCounts.begin(), fieldCounts.end());
        size_t mostCommonCount = fieldCounts[fieldCounts.size() / 2]; // 中位数
        
        // 计算置信度：字段数一致的行的比例
        size_t consistentLines = 0;
        for (size_t count : fieldCounts) {
            if (count == mostCommonCount) {
                ++consistentLines;
            }
        }
        
        double confidence = static_cast<double>(consistentLines) / fieldCounts.size();
        
        // 只保留置信度较高的候选
        if (confidence > 0.7 && mostCommonCount > 1) {
            FormatCandidate candidate;
            candidate.delimiter = delim;
            candidate.fieldCount = mostCommonCount;
            candidate.confidence = confidence;
            candidates.push_back(candidate);
        }
    }
    
    // 按置信度排序
    std::sort(candidates.begin(), candidates.end(),
              [](const FormatCandidate& a, const FormatCandidate& b) {
                  return a.confidence > b.confidence;
              });
    
    return candidates;
}

LogParser::ParseTest LogParser::testParse(const std::string& line) const {
    ParseTest result;
    
    try {
        result.fields = splitByDelimiter(line, format_.getDelimiter());
        result.success = !result.fields.empty();
        
        if (result.fields.size() != format_.getFieldCount()) {
            result.error = "Field count mismatch: expected " + 
                          std::to_string(format_.getFieldCount()) + 
                          ", got " + std::to_string(result.fields.size());
        }
    } catch (const std::exception& e) {
        result.success = false;
        result.error = e.what();
    }
    
    return result;
}

std::vector<std::string> LogParser::splitByDelimiter(const std::string& line, const std::string& delimiter) const {
    std::vector<std::string> parts;
    
    if (delimiter.empty()) {
        parts.push_back(line);
        return parts;
    }
    
    size_t start = 0;
    size_t end = line.find(delimiter);
    
    while (end != std::string::npos) {
        parts.push_back(line.substr(start, end - start));
        start = end + delimiter.length();
        end = line.find(delimiter, start);
    }
    
    // 添加最后一部分
    parts.push_back(line.substr(start));
    
    return parts;
}

size_t LogParser::countOccurrences(const std::string& str, const std::string& substr) {
    if (substr.empty()) return 0;
    
    size_t count = 0;
    size_t pos = 0;
    
    while ((pos = str.find(substr, pos)) != std::string::npos) {
        ++count;
        pos += substr.length();
    }
    
    return count;
}

} // namespace logviewer
