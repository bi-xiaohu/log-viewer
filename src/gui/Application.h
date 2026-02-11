#pragma once

#include "core/Database.h"
#include "core/LogFormat.h"
#include "core/LogEntry.h"
#include <GLFW/glfw3.h>
#include <string>
#include <memory>
#include <vector>

namespace logviewer {

// 主应用程序
class Application {
public:
    Application();
    ~Application();
    
    // 初始化
    bool initialize();
    
    // 主循环
    void run();
    
    // 关闭
    void shutdown();
    
private:
    GLFWwindow* window_ = nullptr;
    std::unique_ptr<Database> database_;
    
    // 应用状态
    std::vector<LogFormat> formats_;
    std::vector<LogEntry> logs_;
    LogFormat* currentFormat_ = nullptr;
    int selectedLogIndex_ = -1;
    
    // UI状态
    bool showFormatEditor_ = false;
    bool showImportDialog_ = false;
    bool showAboutDialog_ = false;
    bool isDarkMode_ = false;  // 主题模式，默认白天模式
    
    // 搜索状态
    char searchBuffer_[256] = "";
    std::vector<LogEntry> filteredLogs_;
    
    // 粘贴区
    char pasteBuffer_[4096] = "";  // 支持粘贴多条日志
    
    // Format编辑器扩展
    int fieldCountInput_ = 1;  // 字段数量输入
    char formatPatternBuffer_[512] = "";  // 格式模式输入（如 %field1|%field2）
    
    // 临时编辑状态
    LogFormat editingFormat_;
    bool isEditingFormat_ = false;
    
    // 初始化函数
    bool initializeGLFW();
    bool initializeImGui();
    
    // 渲染函数
    void render();
    void renderMenuBar();
    void renderFormatPanel();
    void renderLogTableView();
    void renderPasteArea();
    void renderFormatEditor();
    void renderImportDialog();
    void renderAboutDialog();
    
    // 主题设置
    void applyLightTheme();
    void applyDarkTheme();
    void toggleTheme();
    
    // 业务逻辑
    void loadFormats();
    void loadLogs(const std::string& formatId);
    void createNewFormat();
    void editFormat(LogFormat* format);
    void saveCurrentFormat();
    void deleteCurrentFormat();
    void importFormatFromFile();
    void exportFormatToFile();
    void importLogFile();
    void importSingleLog();
    void applySearch();
    void clearSearch();
    void copySelectedLogToClipboard();  // 新增：复制功能
    
    // 工具函数
    std::string openFileDialog(const char* filter, bool save = false);
    void showMessageBox(const std::string& title, const std::string& message);
};

} // namespace logviewer
