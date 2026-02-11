#include "LogFormat.h"
#include <fstream>
#include <sstream>
#include <chrono>
#include <iomanip>

namespace logviewer {

LogFormat::LogFormat(std::string name, std::string delimiter)
    : name_(std::move(name)), delimiter_(std::move(delimiter)) {
    generateId();
}

void LogFormat::addField(const LogField& field) {
    fields_.push_back(field);
}

void LogFormat::removeField(size_t index) {
    if (index < fields_.size()) {
        fields_.erase(fields_.begin() + index);
    }
}

void LogFormat::updateField(size_t index, const LogField& field) {
    if (index < fields_.size()) {
        fields_[index] = field;
    }
}

void LogFormat::clearFields() {
    fields_.clear();
}

nlohmann::json LogFormat::toJson() const {
    nlohmann::json j;
    j["id"] = id_;
    j["name"] = name_;
    j["description"] = description_;
    j["delimiter"] = delimiter_;
    j["fields"] = nlohmann::json::array();
    
    for (const auto& field : fields_) {
        j["fields"].push_back({
            {"name", field.name},
            {"description", field.description}
        });
    }
    
    return j;
}

LogFormat LogFormat::fromJson(const nlohmann::json& j) {
    LogFormat format;
    
    if (j.contains("id")) {
        format.id_ = j["id"].get<std::string>();
    } else {
        format.generateId();
    }
    
    format.name_ = j.value("name", "Unnamed Format");
    format.description_ = j.value("description", "");
    format.delimiter_ = j.value("delimiter", " ");
    
    if (j.contains("fields") && j["fields"].is_array()) {
        for (const auto& fieldJson : j["fields"]) {
            LogField field;
            field.name = fieldJson.value("name", "");
            field.description = fieldJson.value("description", "");
            format.fields_.push_back(field);
        }
    }
    
    return format;
}

bool LogFormat::saveToFile(const std::string& filepath) const {
    try {
        std::ofstream file(filepath);
        if (!file.is_open()) {
            return false;
        }
        
        nlohmann::json j = toJson();
        file << j.dump(4); // 格式化输出，缩进4空格
        
        return true;
    } catch (...) {
        return false;
    }
}

LogFormat LogFormat::loadFromFile(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file: " + filepath);
    }
    
    nlohmann::json j;
    file >> j;
    
    return fromJson(j);
}

bool LogFormat::isValid() const {
    return !name_.empty() && !delimiter_.empty() && !fields_.empty();
}

void LogFormat::generateId() {
    // 生成基于时间戳的唯一ID
    auto now = std::chrono::system_clock::now();
    auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()).count();
    
    std::ostringstream oss;
    oss << "fmt_" << timestamp;
    id_ = oss.str();
}

} // namespace logviewer
