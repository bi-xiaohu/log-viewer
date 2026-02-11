#pragma once

#include <string>
#include <vector>
#include <map>
#include <nlohmann/json.hpp>

namespace logviewer {

// 日志记录
class LogEntry {
public:
    LogEntry() = default;
    LogEntry(std::string formatId, std::string rawText);
    
    // Getters
    int64_t getId() const { return id_; }
    const std::string& getFormatId() const { return formatId_; }
    const std::string& getRawText() const { return rawText_; }
    const std::map<std::string, std::string>& getFields() const { return fields_; }
    
    // Setters
    void setId(int64_t id) { id_ = id; }
    void setFormatId(const std::string& formatId) { formatId_ = formatId; }
    void setRawText(const std::string& rawText) { rawText_ = rawText; }
    
    // 字段操作
    void setField(const std::string& name, const std::string& value);
    std::string getField(const std::string& name) const;
    bool hasField(const std::string& name) const;
    void clearFields();
    
    // JSON序列化
    nlohmann::json fieldsToJson() const;
    void fieldsFromJson(const nlohmann::json& j);
    
private:
    int64_t id_ = 0;                               // 数据库ID
    std::string formatId_;                          // 关联的格式ID
    std::string rawText_;                           // 原始日志文本
    std::map<std::string, std::string> fields_;     // 解析后的字段
};

} // namespace logviewer
