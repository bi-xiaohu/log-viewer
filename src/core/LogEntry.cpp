#include "LogEntry.h"

namespace logviewer {

LogEntry::LogEntry(std::string formatId, std::string rawText)
    : formatId_(std::move(formatId)), rawText_(std::move(rawText)) {
}

void LogEntry::setField(const std::string& name, const std::string& value) {
    fields_[name] = value;
}

std::string LogEntry::getField(const std::string& name) const {
    auto it = fields_.find(name);
    return it != fields_.end() ? it->second : "";
}

bool LogEntry::hasField(const std::string& name) const {
    return fields_.find(name) != fields_.end();
}

void LogEntry::clearFields() {
    fields_.clear();
}

nlohmann::json LogEntry::fieldsToJson() const {
    nlohmann::json j;
    for (const auto& [key, value] : fields_) {
        j[key] = value;
    }
    return j;
}

void LogEntry::fieldsFromJson(const nlohmann::json& j) {
    fields_.clear();
    for (auto it = j.begin(); it != j.end(); ++it) {
        fields_[it.key()] = it.value().get<std::string>();
    }
}

} // namespace logviewer
