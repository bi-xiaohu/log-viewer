#pragma once

#include "LogEntry.h"
#include "LogFormat.h"
#include <sqlite3.h>
#include <string>
#include <vector>
#include <memory>

namespace logviewer {

// SQLite数据库封装
class Database {
public:
    Database();
    ~Database();
    
    // 禁止拷贝
    Database(const Database&) = delete;
    Database& operator=(const Database&) = delete;
    
    // 初始化数据库（内存模式）
    bool initialize();
    
    // 关闭数据库
    void close();
    
    // 日志记录操作
    bool insertLogEntry(LogEntry& entry);
    bool insertLogEntries(std::vector<LogEntry>& entries);
    bool deleteLogEntry(int64_t id);
    bool deleteAllLogs();
    
    // 查询操作
    std::vector<LogEntry> getAllLogs(const std::string& formatId = "");
    LogEntry getLogById(int64_t id);
    std::vector<LogEntry> searchLogs(const std::string& keyword, const std::string& fieldName = "");
    
    // 统计信息
    int64_t getLogCount(const std::string& formatId = "");
    
    // 格式管理
    bool saveFormat(const LogFormat& format);
    bool deleteFormat(const std::string& formatId);
    std::vector<LogFormat> getAllFormats();
    LogFormat getFormatById(const std::string& formatId);
    
    // 事务支持
    bool beginTransaction();
    bool commitTransaction();
    bool rollbackTransaction();
    
    // 错误信息
    std::string getLastError() const { return lastError_; }
    
private:
    sqlite3* db_ = nullptr;
    std::string lastError_;
    
    // 辅助函数
    bool executeSQL(const std::string& sql);
    bool createTables();
    void setError(const std::string& error);
};

} // namespace logviewer
