#pragma once

#include <string>
#include <vector>
#include <nlohmann/json.hpp>

namespace logviewer {

// 日志字段定义
struct LogField {
    std::string name;        // 字段名称
    std::string description; // 字段描述
    
    // JSON序列化
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(LogField, name, description)
};

// 日志格式定义
class LogFormat {
public:
    LogFormat() = default;
    LogFormat(std::string name, std::string delimiter);
    
    // Getters
    const std::string& getName() const { return name_; }
    const std::string& getDescription() const { return description_; }
    const std::string& getDelimiter() const { return delimiter_; }
    const std::vector<LogField>& getFields() const { return fields_; }
    const std::string& getId() const { return id_; }
    
    // Setters
    void setName(const std::string& name) { name_ = name; }
    void setDescription(const std::string& desc) { description_ = desc; }
    void setDelimiter(const std::string& delim) { delimiter_ = delim; }
    
    // 字段管理
    void addField(const LogField& field);
    void removeField(size_t index);
    void updateField(size_t index, const LogField& field);
    void clearFields();
    
    size_t getFieldCount() const { return fields_.size(); }
    
    // JSON序列化
    nlohmann::json toJson() const;
    static LogFormat fromJson(const nlohmann::json& j);
    
    // 保存和加载
    bool saveToFile(const std::string& filepath) const;
    static LogFormat loadFromFile(const std::string& filepath);
    
    // 验证
    bool isValid() const;
    
private:
    std::string id_;          // 唯一标识符（自动生成）
    std::string name_;        // 格式名称
    std::string description_; // 格式描述
    std::string delimiter_;   // 分隔符
    std::vector<LogField> fields_; // 字段列表
    
    void generateId();
};

} // namespace logviewer
