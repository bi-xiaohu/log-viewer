#include "Database.h"
#include <sstream>

namespace logviewer {

Database::Database() = default;

Database::~Database() {
    close();
}

bool Database::initialize() {
    // 使用内存数据库
    int rc = sqlite3_open(":memory:", &db_);
    if (rc != SQLITE_OK) {
        setError("Failed to open database: " + std::string(sqlite3_errmsg(db_)));
        return false;
    }
    
    return createTables();
}

void Database::close() {
    if (db_) {
        sqlite3_close(db_);
        db_ = nullptr;
    }
}

bool Database::createTables() {
    // 创建日志表
    const char* createLogsTable = R"(
        CREATE TABLE IF NOT EXISTS logs (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            format_id TEXT NOT NULL,
            raw_text TEXT NOT NULL,
            parsed_data TEXT NOT NULL
        );
        CREATE INDEX IF NOT EXISTS idx_format ON logs(format_id);
    )";
    
    // 创建格式表
    const char* createFormatsTable = R"(
        CREATE TABLE IF NOT EXISTS formats (
            id TEXT PRIMARY KEY,
            name TEXT NOT NULL,
            data TEXT NOT NULL
        );
    )";
    
    return executeSQL(createLogsTable) && executeSQL(createFormatsTable);
}

bool Database::insertLogEntry(LogEntry& entry) {
    const char* sql = "INSERT INTO logs (format_id, raw_text, parsed_data) VALUES (?, ?, ?);";
    
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        setError("Failed to prepare statement: " + std::string(sqlite3_errmsg(db_)));
        return false;
    }
    
    sqlite3_bind_text(stmt, 1, entry.getFormatId().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, entry.getRawText().c_str(), -1, SQLITE_TRANSIENT);
    
    std::string jsonData = entry.fieldsToJson().dump();
    sqlite3_bind_text(stmt, 3, jsonData.c_str(), -1, SQLITE_TRANSIENT);
    
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    if (rc != SQLITE_DONE) {
        setError("Failed to insert log: " + std::string(sqlite3_errmsg(db_)));
        return false;
    }
    
    entry.setId(sqlite3_last_insert_rowid(db_));
    return true;
}

bool Database::insertLogEntries(std::vector<LogEntry>& entries) {
    if (!beginTransaction()) {
        return false;
    }
    
    for (auto& entry : entries) {
        if (!insertLogEntry(entry)) {
            rollbackTransaction();
            return false;
        }
    }
    
    return commitTransaction();
}

bool Database::deleteLogEntry(int64_t id) {
    std::ostringstream oss;
    oss << "DELETE FROM logs WHERE id = " << id << ";";
    return executeSQL(oss.str());
}

bool Database::deleteAllLogs() {
    return executeSQL("DELETE FROM logs;");
}

std::vector<LogEntry> Database::getAllLogs(const std::string& formatId) {
    std::vector<LogEntry> entries;
    
    std::string sql = "SELECT id, format_id, raw_text, parsed_data FROM logs";
    if (!formatId.empty()) {
        sql += " WHERE format_id = '" + formatId + "'";
    }
    sql += ";";
    
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        setError("Failed to prepare statement: " + std::string(sqlite3_errmsg(db_)));
        return entries;
    }
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        LogEntry entry;
        entry.setId(sqlite3_column_int64(stmt, 0));
        entry.setFormatId(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)));
        entry.setRawText(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2)));
        
        std::string jsonData = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        entry.fieldsFromJson(nlohmann::json::parse(jsonData));
        
        entries.push_back(entry);
    }
    
    sqlite3_finalize(stmt);
    return entries;
}

LogEntry Database::getLogById(int64_t id) {
    std::ostringstream oss;
    oss << "SELECT id, format_id, raw_text, parsed_data FROM logs WHERE id = " << id << ";";
    
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db_, oss.str().c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        setError("Failed to prepare statement: " + std::string(sqlite3_errmsg(db_)));
        return LogEntry();
    }
    
    LogEntry entry;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        entry.setId(sqlite3_column_int64(stmt, 0));
        entry.setFormatId(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)));
        entry.setRawText(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2)));
        
        std::string jsonData = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        entry.fieldsFromJson(nlohmann::json::parse(jsonData));
    }
    
    sqlite3_finalize(stmt);
    return entry;
}

std::vector<LogEntry> Database::searchLogs(const std::string& keyword, const std::string& fieldName) {
    std::vector<LogEntry> entries;
    
    std::string sql = "SELECT id, format_id, raw_text, parsed_data FROM logs WHERE ";
    if (fieldName.empty()) {
        sql += "raw_text LIKE '%" + keyword + "%'";
    } else {
        sql += "parsed_data LIKE '%\"" + fieldName + "\":%\"" + keyword + "\"%'";
    }
    sql += ";";
    
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        setError("Failed to prepare statement: " + std::string(sqlite3_errmsg(db_)));
        return entries;
    }
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        LogEntry entry;
        entry.setId(sqlite3_column_int64(stmt, 0));
        entry.setFormatId(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)));
        entry.setRawText(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2)));
        
        std::string jsonData = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        entry.fieldsFromJson(nlohmann::json::parse(jsonData));
        
        entries.push_back(entry);
    }
    
    sqlite3_finalize(stmt);
    return entries;
}

int64_t Database::getLogCount(const std::string& formatId) {
    std::string sql = "SELECT COUNT(*) FROM logs";
    if (!formatId.empty()) {
        sql += " WHERE format_id = '" + formatId + "'";
    }
    sql += ";";
    
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        setError("Failed to prepare statement: " + std::string(sqlite3_errmsg(db_)));
        return 0;
    }
    
    int64_t count = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        count = sqlite3_column_int64(stmt, 0);
    }
    
    sqlite3_finalize(stmt);
    return count;
}

bool Database::saveFormat(const LogFormat& format) {
    const char* sql = "INSERT OR REPLACE INTO formats (id, name, data) VALUES (?, ?, ?);";
    
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        setError("Failed to prepare statement: " + std::string(sqlite3_errmsg(db_)));
        return false;
    }
    
    sqlite3_bind_text(stmt, 1, format.getId().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, format.getName().c_str(), -1, SQLITE_TRANSIENT);
    
    std::string jsonData = format.toJson().dump();
    sqlite3_bind_text(stmt, 3, jsonData.c_str(), -1, SQLITE_TRANSIENT);
    
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    if (rc != SQLITE_DONE) {
        setError("Failed to save format: " + std::string(sqlite3_errmsg(db_)));
        return false;
    }
    
    return true;
}

bool Database::deleteFormat(const std::string& formatId) {
    std::string sql = "DELETE FROM formats WHERE id = '" + formatId + "';";
    return executeSQL(sql);
}

std::vector<LogFormat> Database::getAllFormats() {
    std::vector<LogFormat> formats;
    
    const char* sql = "SELECT data FROM formats;";
    
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        setError("Failed to prepare statement: " + std::string(sqlite3_errmsg(db_)));
        return formats;
    }
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        std::string jsonData = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        formats.push_back(LogFormat::fromJson(nlohmann::json::parse(jsonData)));
    }
    
    sqlite3_finalize(stmt);
    return formats;
}

LogFormat Database::getFormatById(const std::string& formatId) {
    std::string sql = "SELECT data FROM formats WHERE id = '" + formatId + "';";
    
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        setError("Failed to prepare statement: " + std::string(sqlite3_errmsg(db_)));
        return LogFormat();
    }
    
    LogFormat format;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        std::string jsonData = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        format = LogFormat::fromJson(nlohmann::json::parse(jsonData));
    }
    
    sqlite3_finalize(stmt);
    return format;
}

bool Database::beginTransaction() {
    return executeSQL("BEGIN TRANSACTION;");
}

bool Database::commitTransaction() {
    return executeSQL("COMMIT;");
}

bool Database::rollbackTransaction() {
    return executeSQL("ROLLBACK;");
}

bool Database::executeSQL(const std::string& sql) {
    char* errMsg = nullptr;
    int rc = sqlite3_exec(db_, sql.c_str(), nullptr, nullptr, &errMsg);
    
    if (rc != SQLITE_OK) {
        setError("SQL error: " + std::string(errMsg));
        sqlite3_free(errMsg);
        return false;
    }
    
    return true;
}

void Database::setError(const std::string& error) {
    lastError_ = error;
}

} // namespace logviewer
