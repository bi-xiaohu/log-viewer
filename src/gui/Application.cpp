#include "Application.h"
#include "core/LogParser.h"
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <fstream>
#include <algorithm>
#include <sstream>

#ifdef _WIN32
#include <windows.h>
#include <commdlg.h>
#endif

namespace logviewer {

Application::Application() {
    database_ = std::make_unique<Database>();
}

Application::~Application() {
    shutdown();
}

bool Application::initialize() {
    if (!initializeGLFW()) {
        return false;
    }
    
    if (!initializeImGui()) {
        return false;
    }
    
    if (!database_->initialize()) {
        return false;
    }
    
    loadFormats();
    
    return true;
}

bool Application::initializeGLFW() {
    if (!glfwInit()) {
        return false;
    }
    
    // OpenGL 3.3
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    window_ = glfwCreateWindow(1600, 900, "Log Viewer", nullptr, nullptr);
    if (!window_) {
        glfwTerminate();
        return false;
    }
    
    glfwMakeContextCurrent(window_);
    glfwSwapInterval(1); // VSync
    
    return true;
}

bool Application::initializeImGui() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    
    // 初始化平台/渲染器绑定
    ImGui_ImplGlfw_InitForOpenGL(window_, true);
    ImGui_ImplOpenGL3_Init("#version 330");
    
    // 加载中文字体
#ifdef _WIN32
    // 使用系统微软雅黑字体
    io.Fonts->AddFontFromFileTTF("C:/Windows/Fonts/msyh.ttc", 18.0f, nullptr, io.Fonts->GetGlyphRangesChineseFull());
#else
    // Linux/macOS 可以尝试其他中文字体路径
    // io.Fonts->AddFontFromFileTTF("/usr/share/fonts/truetype/droid/DroidSansFallbackFull.ttf", 18.0f, nullptr, io.Fonts->GetGlyphRangesChineseFull());
#endif
    
    // 设置默认主题（白天模式）
    applyLightTheme();
    
    return true;
}

void Application::run() {
    while (!glfwWindowShouldClose(window_)) {
        glfwPollEvents();
        
        // 开始ImGui帧
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        
        render();
        
        // 渲染
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window_, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        
        glfwSwapBuffers(window_);
    }
}

void Application::shutdown() {
    if (database_) {
        database_->close();
    }
    
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    
    if (window_) {
        glfwDestroyWindow(window_);
        glfwTerminate();
    }
}

void Application::render() {
    renderMenuBar();
    
    // 主窗口布局
    ImGui::SetNextWindowPos(ImVec2(0, 20));
    ImGui::SetNextWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x, ImGui::GetIO().DisplaySize.y - 20));
    
    ImGui::Begin("Main", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus);
    
    // 两列布局：格式面板 | 日志表格+粘贴区
    ImGui::Columns(2, "MainColumns");
    ImGui::SetColumnWidth(0, 250);
    
    renderFormatPanel();
    ImGui::NextColumn();
    
    // 右侧分为上下两部分
    float rightHeight = ImGui::GetContentRegionAvail().y;
    
    // 上半部分：日志表格（70%）
    ImGui::BeginChild("LogTableSection", ImVec2(0, rightHeight * 0.7f), true);
    renderLogTableView();
    ImGui::EndChild();
    
    // 下半部分：粘贴区（30%）
    ImGui::BeginChild("PasteSection", ImVec2(0, 0), true);
    renderPasteArea();
    ImGui::EndChild();
    
    ImGui::Columns(1);
    ImGui::End();
    
    // 对话框
    if (showFormatEditor_) {
        renderFormatEditor();
    }
    
    if (showImportDialog_) {
        renderImportDialog();
    }
    
    if (showAboutDialog_) {
        renderAboutDialog();
    }
}

void Application::renderMenuBar() {
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Import Log File")) {
                importLogFile();
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Import Format")) {
                importFormatFromFile();
            }
            if (ImGui::MenuItem("Export Format", nullptr, false, currentFormat_ != nullptr)) {
                exportFormatToFile();
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Exit")) {
                glfwSetWindowShouldClose(window_, GLFW_TRUE);
            }
            ImGui::EndMenu();
        }
        
        if (ImGui::BeginMenu("Edit")) {
            if (ImGui::MenuItem("New Format")) {
                createNewFormat();
            }
            if (ImGui::MenuItem("Edit Format", nullptr, false, currentFormat_ != nullptr)) {
                editFormat(currentFormat_);
            }
            if (ImGui::MenuItem("Delete Format", nullptr, false, currentFormat_ != nullptr)) {
                deleteCurrentFormat();
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Clear All Logs")) {
                database_->deleteAllLogs();
                logs_.clear();
                filteredLogs_.clear();
            }
            ImGui::EndMenu();
        }
        
        if (ImGui::BeginMenu("View")) {
            if (ImGui::MenuItem("Toggle Theme", "Ctrl+T")) {
                toggleTheme();
            }
            ImGui::Text("Current: %s", isDarkMode_ ? "Dark Mode" : "Light Mode");
            ImGui::EndMenu();
        }
        
        if (ImGui::BeginMenu("Help")) {
            if (ImGui::MenuItem("About")) {
                showAboutDialog_ = true;
            }
            ImGui::EndMenu();
        }
        
        ImGui::EndMainMenuBar();
    }
}

void Application::renderFormatPanel() {
    ImGui::BeginChild("FormatPanel", ImVec2(0, 0), true);
    
    ImGui::Text("Log Formats");
    ImGui::Separator();
    
    for (size_t i = 0; i < formats_.size(); ++i) {
        auto& format = formats_[i];
        bool isSelected = (currentFormat_ == &format);
        
        if (ImGui::Selectable(format.getName().c_str(), isSelected)) {
            currentFormat_ = &format;
            loadLogs(format.getId());
        }
        
        if (isSelected && ImGui::IsItemHovered()) {
            ImGui::BeginTooltip();
            ImGui::Text("Description: %s", format.getDescription().c_str());
            ImGui::Text("Delimiter: '%s'", format.getDelimiter().c_str());
            ImGui::Text("Fields: %zu", format.getFieldCount());
            ImGui::EndTooltip();
        }
    }
    
    ImGui::Separator();
    if (ImGui::Button("New Format", ImVec2(-1, 0))) {
        createNewFormat();
    }
    
    ImGui::EndChild();
}

void Application::renderLogTableView() {
    ImGui::Text("Log Entries (%zu)", filteredLogs_.empty() ? logs_.size() : filteredLogs_.size());
    ImGui::SameLine();
    
    // 搜索框
    ImGui::SetNextItemWidth(150);
    if (ImGui::InputText("##Search", searchBuffer_, sizeof(searchBuffer_))) {
        applySearch();
    }
    ImGui::SameLine();
    if (ImGui::Button("Clear")) {
        clearSearch();
    }
    ImGui::SameLine();
    if (ImGui::Button("Copy Selected") && selectedLogIndex_ >= 0) {
        copySelectedLogToClipboard();
    }
    
    ImGui::Separator();
    
    // 表格
    const auto& displayLogs = filteredLogs_.empty() ? logs_ : filteredLogs_;
    
    if (currentFormat_ && ImGui::BeginTable("LogTable", currentFormat_->getFieldCount() + 1, 
                                             ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY | ImGuiTableFlags_ScrollX)) {
        // 表头
        ImGui::TableSetupColumn("ID", ImGuiTableColumnFlags_WidthFixed, 50);
        const auto& fields = currentFormat_->getFields();
        for (const auto& field : fields) {
            ImGui::TableSetupColumn(field.name.c_str(), ImGuiTableColumnFlags_WidthFixed, 120);
        }
        ImGui::TableSetupScrollFreeze(1, 1); // 冻结第一列和表头
        ImGui::TableHeadersRow();
        
        // 数据行
        for (size_t i = 0; i < displayLogs.size(); ++i) {
            const auto& entry = displayLogs[i];
            ImGui::TableNextRow();
            
            ImGui::TableSetColumnIndex(0);
            bool isSelected = (static_cast<int>(i) == selectedLogIndex_);
            if (ImGui::Selectable(std::to_string(entry.getId()).c_str(), isSelected, ImGuiSelectableFlags_SpanAllColumns)) {
                selectedLogIndex_ = static_cast<int>(i);
            }
            
            // 显示字段值，并添加悬停提示
            int col = 1;
            for (const auto& field : fields) {
                ImGui::TableSetColumnIndex(col++);
                std::string fieldValue = entry.getField(field.name);
                
                // 截断长文本显示（超过50个字符）
                std::string displayValue = fieldValue;
                bool isTruncated = false;
                if (fieldValue.length() > 50) {
                    displayValue = fieldValue.substr(0, 47) + "...";
                    isTruncated = true;
                }
                
                ImGui::TextUnformatted(displayValue.c_str());
                
                // 鼠标悬停显示完整信息
                if (ImGui::IsItemHovered()) {
                    ImGui::BeginTooltip();
                    ImGui::TextColored(ImVec4(0.4f, 0.7f, 1.0f, 1.0f), "[%s]", field.name.c_str());
                    if (!field.description.empty()) {
                        ImGui::Text("%s", field.description.c_str());
                        ImGui::Separator();
                    }
                    if (isTruncated) {
                        ImGui::TextWrapped("%s", fieldValue.c_str());
                    } else {
                        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Value: %s", fieldValue.c_str());
                    }
                    ImGui::EndTooltip();
                }
            }
        }
        
        ImGui::EndTable();
    }
}

void Application::renderPasteArea() {
    ImGui::Text("Paste Log Data (1-10 lines)");
    ImGui::Separator();
    
    // 粘贴区
    if (ImGui::InputTextMultiline("##PasteArea", pasteBuffer_, sizeof(pasteBuffer_), 
                                   ImVec2(-1, -80), ImGuiInputTextFlags_AllowTabInput)) {
        // 输入变化时的处理
    }
    
    ImGui::Separator();
    
    // 按钮行
    if (ImGui::Button("Import from Paste", ImVec2(150, 0))) {
        if (currentFormat_ && strlen(pasteBuffer_) > 0) {
            // 按行分割
            std::vector<std::string> lines;
            std::istringstream stream(pasteBuffer_);
            std::string line;
            
            while (std::getline(stream, line) && lines.size() < 10) {
                if (!line.empty() && line != "\r") {
                    // 移除末尾的\r
                    if (!line.empty() && line.back() == '\r') {
                        line.pop_back();
                    }
                    lines.push_back(line);
                }
            }
            
            if (!lines.empty()) {
                LogParser parser(*currentFormat_);
                auto entries = parser.parseLines(lines);
                database_->insertLogEntries(entries);
                loadLogs(currentFormat_->getId());
                
                // 清空粘贴区
                pasteBuffer_[0] = '\0';
            }
        }
    }
    
    ImGui::SameLine();
    if (ImGui::Button("Import from File", ImVec2(150, 0))) {
        importLogFile();
    }
    
    ImGui::SameLine();
    if (ImGui::Button("Clear", ImVec2(100, 0))) {
        pasteBuffer_[0] = '\0';
    }
    
    ImGui::SameLine();
    ImGui::TextDisabled("Select a format first");
}

void Application::renderFormatEditor() {
    ImGui::SetNextWindowSize(ImVec2(700, 700), ImGuiCond_FirstUseEver);
    ImGui::Begin("Format Editor", &showFormatEditor_);
    
    char nameBuffer[128];
    char descBuffer[256];
    char delimBuffer[32];
    
    strncpy_s(nameBuffer, editingFormat_.getName().c_str(), sizeof(nameBuffer) - 1);
    strncpy_s(descBuffer, editingFormat_.getDescription().c_str(), sizeof(descBuffer) - 1);
    strncpy_s(delimBuffer, editingFormat_.getDelimiter().c_str(), sizeof(delimBuffer) - 1);
    
    ImGui::InputText("Name", nameBuffer, sizeof(nameBuffer));
    ImGui::InputText("Description", descBuffer, sizeof(descBuffer));
    ImGui::InputText("Delimiter", delimBuffer, sizeof(delimBuffer));
    
    editingFormat_.setName(nameBuffer);
    editingFormat_.setDescription(descBuffer);
    editingFormat_.setDelimiter(delimBuffer);
    
    ImGui::Separator();
    
    // 快速生成字段区域
    ImGui::Text("Quick Field Generation:");
    ImGui::SetNextItemWidth(150);
    ImGui::InputInt("Number of Fields", &fieldCountInput_, 1, 10);
    ImGui::SameLine();
    if (ImGui::Button("Generate Fields")) {
        if (fieldCountInput_ > 0 && fieldCountInput_ <= 50) {
            editingFormat_.clearFields();
            for (int i = 0; i < fieldCountInput_; ++i) {
                editingFormat_.addField({"field" + std::to_string(i + 1), ""});
            }
        }
    }
    
    ImGui::Separator();
    
    // 格式模式解析区域
    ImGui::Text("Parse Format Pattern (e.g., %%timestamp|%%level|%%message):");
    ImGui::InputText("##FormatPattern", formatPatternBuffer_, sizeof(formatPatternBuffer_));
    ImGui::SameLine();
    if (ImGui::Button("Parse Pattern")) {
        std::string pattern = formatPatternBuffer_;
        if (!pattern.empty()) {
            editingFormat_.clearFields();
            std::string delimiter = editingFormat_.getDelimiter();
            
            // 按分隔符切割
            size_t start = 0;
            size_t end = pattern.find(delimiter);
            
            while (end != std::string::npos) {
                std::string token = pattern.substr(start, end - start);
                // 移除 % 前缀
                if (!token.empty() && token[0] == '%') {
                    token = token.substr(1);
                }
                if (!token.empty()) {
                    editingFormat_.addField({token, ""});
                }
                start = end + delimiter.length();
                end = pattern.find(delimiter, start);
            }
            
            // 添加最后一个字段
            std::string token = pattern.substr(start);
            if (!token.empty() && token[0] == '%') {
                token = token.substr(1);
            }
            if (!token.empty()) {
                editingFormat_.addField({token, ""});
            }
            
            formatPatternBuffer_[0] = '\0';
        }
    }
    
    ImGui::Separator();
    ImGui::Text("Fields (scroll to view all):");
    
    // 字段列表 - 使用滚动区域
    ImGui::BeginChild("FieldsList", ImVec2(0, 300), true);
    
    auto fields = editingFormat_.getFields();
    for (size_t i = 0; i < fields.size(); ++i) {
        ImGui::PushID(static_cast<int>(i));
        
        char fieldName[64];
        char fieldDesc[128];
        strncpy_s(fieldName, fields[i].name.c_str(), sizeof(fieldName) - 1);
        strncpy_s(fieldDesc, fields[i].description.c_str(), sizeof(fieldDesc) - 1);
        
        ImGui::SetNextItemWidth(150);
        ImGui::InputText("##Name", fieldName, sizeof(fieldName));
        ImGui::SameLine();
        ImGui::SetNextItemWidth(300);
        ImGui::InputText("##Desc", fieldDesc, sizeof(fieldDesc));
        ImGui::SameLine();
        
        if (ImGui::Button("X")) {
            editingFormat_.removeField(i);
        }
        
        fields[i].name = fieldName;
        fields[i].description = fieldDesc;
        editingFormat_.updateField(i, fields[i]);
        
        ImGui::PopID();
    }
    
    ImGui::EndChild();
    
    // 底部按钮 - 固定位置
    ImGui::Separator();
    if (ImGui::Button("Add Field", ImVec2(120, 0))) {
        editingFormat_.addField({"field" + std::to_string(fields.size() + 1), ""});
    }
    ImGui::SameLine();
    if (ImGui::Button("Save", ImVec2(120, 0))) {
        saveCurrentFormat();
        showFormatEditor_ = false;
    }
    ImGui::SameLine();
    if (ImGui::Button("Cancel", ImVec2(120, 0))) {
        showFormatEditor_ = false;
    }
    
    ImGui::End();
}

void Application::renderImportDialog() {
    ImGui::SetNextWindowSize(ImVec2(500, 300), ImGuiCond_FirstUseEver);
    ImGui::Begin("Import Single Log", &showImportDialog_);
    
    static char logText[1024] = "";
    ImGui::InputTextMultiline("##LogText", logText, sizeof(logText), ImVec2(-1, 200));
    
    if (ImGui::Button("Import")) {
        if (currentFormat_) {
            LogParser parser(*currentFormat_);
            auto entry = parser.parse(logText);
            database_->insertLogEntry(entry);
            loadLogs(currentFormat_->getId());
            showImportDialog_ = false;
        }
    }
    ImGui::SameLine();
    if (ImGui::Button("Cancel")) {
        showImportDialog_ = false;
    }
    
    ImGui::End();
}

void Application::renderAboutDialog() {
    ImGui::SetNextWindowSize(ImVec2(400, 200), ImGuiCond_FirstUseEver);
    ImGui::Begin("About", &showAboutDialog_);
    
    ImGui::Text("Log Viewer v1.0.0");
    ImGui::Separator();
    ImGui::Text("A modern C++20 log viewing tool");
    ImGui::Text("Built with ImGui and SQLite");
    ImGui::Separator();
    ImGui::Text("(c) 2026");
    
    if (ImGui::Button("OK", ImVec2(120, 0))) {
        showAboutDialog_ = false;
    }
    
    ImGui::End();
}

void Application::loadFormats() {
    formats_ = database_->getAllFormats();
}

void Application::loadLogs(const std::string& formatId) {
    logs_ = database_->getAllLogs(formatId);
    filteredLogs_.clear();
    selectedLogIndex_ = -1;
}

void Application::createNewFormat() {
    editingFormat_ = LogFormat("New Format", " ");
    isEditingFormat_ = false;
    showFormatEditor_ = true;
}

void Application::editFormat(LogFormat* format) {
    if (format) {
        editingFormat_ = *format;
        isEditingFormat_ = true;
        showFormatEditor_ = true;
    }
}

void Application::saveCurrentFormat() {
    database_->saveFormat(editingFormat_);
    loadFormats();
    
    // 更新当前格式指针
    for (auto& fmt : formats_) {
        if (fmt.getId() == editingFormat_.getId()) {
            currentFormat_ = &fmt;
            break;
        }
    }
}

void Application::deleteCurrentFormat() {
    if (currentFormat_) {
        database_->deleteFormat(currentFormat_->getId());
        currentFormat_ = nullptr;
        loadFormats();
        logs_.clear();
        filteredLogs_.clear();
    }
}

void Application::importFormatFromFile() {
    std::string filepath = openFileDialog("JSON Files\0*.json\0All Files\0*.*\0");
    if (!filepath.empty()) {
        try {
            auto format = LogFormat::loadFromFile(filepath);
            database_->saveFormat(format);
            loadFormats();
        } catch (const std::exception& e) {
            showMessageBox("Error", "Failed to import format: " + std::string(e.what()));
        }
    }
}

void Application::exportFormatToFile() {
    if (!currentFormat_) return;
    
    std::string filepath = openFileDialog("JSON Files\0*.json\0All Files\0*.*\0", true);
    if (!filepath.empty()) {
        if (!currentFormat_->saveToFile(filepath)) {
            showMessageBox("Error", "Failed to export format");
        }
    }
}

void Application::importLogFile() {
    std::string filepath = openFileDialog("Log Files\0*.log;*.txt\0All Files\0*.*\0");
    if (filepath.empty() || !currentFormat_) return;
    
    try {
        LogParser parser(*currentFormat_);
        auto entries = parser.parseFile(filepath);
        database_->insertLogEntries(entries);
        loadLogs(currentFormat_->getId());
    } catch (const std::exception& e) {
        showMessageBox("Error", "Failed to import log file: " + std::string(e.what()));
    }
}

void Application::importSingleLog() {
    showImportDialog_ = true;
}

void Application::applySearch() {
    if (strlen(searchBuffer_) == 0) {
        filteredLogs_.clear();
        return;
    }
    
    filteredLogs_ = database_->searchLogs(searchBuffer_);
}

void Application::clearSearch() {
    searchBuffer_[0] = '\0';
    filteredLogs_.clear();
}

void Application::copySelectedLogToClipboard() {
    if (selectedLogIndex_ < 0 || selectedLogIndex_ >= static_cast<int>(logs_.size())) {
        return;
    }
    
    const auto& entry = logs_[selectedLogIndex_];
    const std::string& rawText = entry.getRawText();
    
#ifdef _WIN32
    if (OpenClipboard(nullptr)) {
        EmptyClipboard();
        HGLOBAL hGlob = GlobalAlloc(GMEM_FIXED, rawText.size() + 1);
        if (hGlob) {
            memcpy(hGlob, rawText.c_str(), rawText.size() + 1);
            SetClipboardData(CF_TEXT, hGlob);
        }
        CloseClipboard();
    }
#else
    // Linux/macOS 可以使用 ImGui 的剪贴板API
    ImGui::SetClipboardText(rawText.c_str());
#endif
}

std::string Application::openFileDialog(const char* filter, bool save) {
#ifdef _WIN32
    OPENFILENAMEA ofn;
    char szFile[260] = {0};
    
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = nullptr;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = filter;
    ofn.nFilterIndex = 1;
    ofn.Flags = save ? (OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT) : (OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST);
    
    if (save ? GetSaveFileNameA(&ofn) : GetOpenFileNameA(&ofn)) {
        return std::string(szFile);
    }
#endif
    
    return "";
}

void Application::showMessageBox(const std::string& title, const std::string& message) {
#ifdef _WIN32
    MessageBoxA(nullptr, message.c_str(), title.c_str(), MB_OK | MB_ICONERROR);
#endif
}

void Application::applyLightTheme() {
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;
    
    // 灰白色主题
    colors[ImGuiCol_Text]                   = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
    colors[ImGuiCol_TextDisabled]           = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
    colors[ImGuiCol_WindowBg]               = ImVec4(0.94f, 0.94f, 0.94f, 1.00f);
    colors[ImGuiCol_ChildBg]                = ImVec4(0.97f, 0.97f, 0.97f, 1.00f);
    colors[ImGuiCol_PopupBg]                = ImVec4(1.00f, 1.00f, 1.00f, 0.98f);
    colors[ImGuiCol_Border]                 = ImVec4(0.70f, 0.70f, 0.70f, 0.40f);
    colors[ImGuiCol_BorderShadow]           = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_FrameBg]                = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    colors[ImGuiCol_FrameBgHovered]         = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
    colors[ImGuiCol_FrameBgActive]          = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
    colors[ImGuiCol_TitleBg]                = ImVec4(0.96f, 0.96f, 0.96f, 1.00f);
    colors[ImGuiCol_TitleBgActive]          = ImVec4(0.82f, 0.82f, 0.82f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed]       = ImVec4(1.00f, 1.00f, 1.00f, 0.51f);
    colors[ImGuiCol_MenuBarBg]              = ImVec4(0.86f, 0.86f, 0.86f, 1.00f);
    colors[ImGuiCol_ScrollbarBg]            = ImVec4(0.98f, 0.98f, 0.98f, 0.53f);
    colors[ImGuiCol_ScrollbarGrab]          = ImVec4(0.69f, 0.69f, 0.69f, 0.80f);
    colors[ImGuiCol_ScrollbarGrabHovered]   = ImVec4(0.49f, 0.49f, 0.49f, 0.80f);
    colors[ImGuiCol_ScrollbarGrabActive]    = ImVec4(0.49f, 0.49f, 0.49f, 1.00f);
    colors[ImGuiCol_CheckMark]              = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    colors[ImGuiCol_SliderGrab]             = ImVec4(0.26f, 0.59f, 0.98f, 0.78f);
    colors[ImGuiCol_SliderGrabActive]       = ImVec4(0.46f, 0.54f, 0.80f, 0.60f);
    colors[ImGuiCol_Button]                 = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
    colors[ImGuiCol_ButtonHovered]          = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    colors[ImGuiCol_ButtonActive]           = ImVec4(0.06f, 0.53f, 0.98f, 1.00f);
    colors[ImGuiCol_Header]                 = ImVec4(0.26f, 0.59f, 0.98f, 0.31f);
    colors[ImGuiCol_HeaderHovered]          = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
    colors[ImGuiCol_HeaderActive]           = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    colors[ImGuiCol_Separator]              = ImVec4(0.39f, 0.39f, 0.39f, 0.62f);
    colors[ImGuiCol_SeparatorHovered]       = ImVec4(0.14f, 0.44f, 0.80f, 0.78f);
    colors[ImGuiCol_SeparatorActive]        = ImVec4(0.14f, 0.44f, 0.80f, 1.00f);
    colors[ImGuiCol_ResizeGrip]             = ImVec4(0.80f, 0.80f, 0.80f, 0.56f);
    colors[ImGuiCol_ResizeGripHovered]      = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
    colors[ImGuiCol_ResizeGripActive]       = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
    colors[ImGuiCol_Tab]                    = ImVec4(0.76f, 0.80f, 0.84f, 0.93f);
    colors[ImGuiCol_TabHovered]             = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
    colors[ImGuiCol_TabActive]              = ImVec4(0.60f, 0.73f, 0.88f, 1.00f);
    colors[ImGuiCol_TabUnfocused]           = ImVec4(0.92f, 0.93f, 0.94f, 0.99f);
    colors[ImGuiCol_TabUnfocusedActive]     = ImVec4(0.74f, 0.82f, 0.91f, 1.00f);
    colors[ImGuiCol_PlotLines]              = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
    colors[ImGuiCol_PlotLinesHovered]       = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
    colors[ImGuiCol_PlotHistogram]          = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
    colors[ImGuiCol_PlotHistogramHovered]   = ImVec4(1.00f, 0.45f, 0.00f, 1.00f);
    colors[ImGuiCol_TableHeaderBg]          = ImVec4(0.78f, 0.87f, 0.98f, 1.00f);
    colors[ImGuiCol_TableBorderStrong]      = ImVec4(0.57f, 0.57f, 0.64f, 1.00f);
    colors[ImGuiCol_TableBorderLight]       = ImVec4(0.68f, 0.68f, 0.74f, 1.00f);
    colors[ImGuiCol_TableRowBg]             = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_TableRowBgAlt]          = ImVec4(0.30f, 0.30f, 0.30f, 0.09f);
    colors[ImGuiCol_TextSelectedBg]         = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
    colors[ImGuiCol_DragDropTarget]         = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
    colors[ImGuiCol_NavHighlight]           = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
    colors[ImGuiCol_NavWindowingHighlight]  = ImVec4(0.70f, 0.70f, 0.70f, 0.70f);
    colors[ImGuiCol_NavWindowingDimBg]      = ImVec4(0.20f, 0.20f, 0.20f, 0.20f);
    colors[ImGuiCol_ModalWindowDimBg]       = ImVec4(0.20f, 0.20f, 0.20f, 0.35f);
    
    style.WindowRounding    = 4.0f;
    style.FrameRounding     = 2.0f;
    style.GrabRounding      = 2.0f;
    style.ScrollbarRounding = 4.0f;
    style.TabRounding       = 4.0f;
}

void Application::applyDarkTheme() {
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;
    
    // 深色主题
    colors[ImGuiCol_Text]                   = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    colors[ImGuiCol_TextDisabled]           = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
    colors[ImGuiCol_WindowBg]               = ImVec4(0.13f, 0.14f, 0.15f, 1.00f);
    colors[ImGuiCol_ChildBg]                = ImVec4(0.13f, 0.14f, 0.15f, 1.00f);
    colors[ImGuiCol_PopupBg]                = ImVec4(0.13f, 0.14f, 0.15f, 1.00f);
    colors[ImGuiCol_Border]                 = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
    colors[ImGuiCol_BorderShadow]           = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_FrameBg]                = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
    colors[ImGuiCol_FrameBgHovered]         = ImVec4(0.38f, 0.38f, 0.38f, 1.00f);
    colors[ImGuiCol_FrameBgActive]          = ImVec4(0.67f, 0.67f, 0.67f, 0.39f);
    colors[ImGuiCol_TitleBg]                = ImVec4(0.08f, 0.08f, 0.09f, 1.00f);
    colors[ImGuiCol_TitleBgActive]          = ImVec4(0.08f, 0.08f, 0.09f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed]       = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
    colors[ImGuiCol_MenuBarBg]              = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
    colors[ImGuiCol_ScrollbarBg]            = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
    colors[ImGuiCol_ScrollbarGrab]          = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabHovered]   = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabActive]    = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
    colors[ImGuiCol_CheckMark]              = ImVec4(0.11f, 0.64f, 0.92f, 1.00f);
    colors[ImGuiCol_SliderGrab]             = ImVec4(0.11f, 0.64f, 0.92f, 1.00f);
    colors[ImGuiCol_SliderGrabActive]       = ImVec4(0.08f, 0.50f, 0.72f, 1.00f);
    colors[ImGuiCol_Button]                 = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
    colors[ImGuiCol_ButtonHovered]          = ImVec4(0.38f, 0.38f, 0.38f, 1.00f);
    colors[ImGuiCol_ButtonActive]           = ImVec4(0.67f, 0.67f, 0.67f, 0.39f);
    colors[ImGuiCol_Header]                 = ImVec4(0.22f, 0.22f, 0.22f, 1.00f);
    colors[ImGuiCol_HeaderHovered]          = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
    colors[ImGuiCol_HeaderActive]           = ImVec4(0.67f, 0.67f, 0.67f, 0.39f);
    colors[ImGuiCol_Separator]              = ImVec4(0.28f, 0.28f, 0.28f, 0.29f);
    colors[ImGuiCol_SeparatorHovered]       = ImVec4(0.44f, 0.44f, 0.44f, 0.29f);
    colors[ImGuiCol_SeparatorActive]        = ImVec4(0.40f, 0.44f, 0.47f, 1.00f);
    colors[ImGuiCol_ResizeGrip]             = ImVec4(0.28f, 0.28f, 0.28f, 0.29f);
    colors[ImGuiCol_ResizeGripHovered]      = ImVec4(0.44f, 0.44f, 0.44f, 0.29f);
    colors[ImGuiCol_ResizeGripActive]       = ImVec4(0.40f, 0.44f, 0.47f, 1.00f);
    colors[ImGuiCol_Tab]                    = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
    colors[ImGuiCol_TabHovered]             = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
    colors[ImGuiCol_TabActive]              = ImVec4(0.20f, 0.20f, 0.20f, 0.36f);
    colors[ImGuiCol_TabUnfocused]           = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
    colors[ImGuiCol_TabUnfocusedActive]     = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
    colors[ImGuiCol_PlotLines]              = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
    colors[ImGuiCol_PlotLinesHovered]       = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
    colors[ImGuiCol_PlotHistogram]          = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
    colors[ImGuiCol_PlotHistogramHovered]   = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
    colors[ImGuiCol_TableHeaderBg]          = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
    colors[ImGuiCol_TableBorderStrong]      = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
    colors[ImGuiCol_TableBorderLight]       = ImVec4(0.28f, 0.28f, 0.28f, 0.29f);
    colors[ImGuiCol_TableRowBg]             = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_TableRowBgAlt]          = ImVec4(1.00f, 1.00f, 1.00f, 0.06f);
    colors[ImGuiCol_TextSelectedBg]         = ImVec4(0.20f, 0.22f, 0.23f, 1.00f);
    colors[ImGuiCol_DragDropTarget]         = ImVec4(0.33f, 0.67f, 0.86f, 1.00f);
    colors[ImGuiCol_NavHighlight]           = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
    colors[ImGuiCol_NavWindowingHighlight]  = ImVec4(1.00f, 0.00f, 0.00f, 0.70f);
    colors[ImGuiCol_NavWindowingDimBg]      = ImVec4(1.00f, 0.00f, 0.00f, 0.20f);
    colors[ImGuiCol_ModalWindowDimBg]       = ImVec4(1.00f, 0.00f, 0.00f, 0.35f);
    
    style.WindowRounding    = 4.0f;
    style.FrameRounding     = 2.0f;
    style.GrabRounding      = 2.0f;
    style.ScrollbarRounding = 4.0f;
    style.TabRounding       = 4.0f;
}

void Application::toggleTheme() {
    isDarkMode_ = !isDarkMode_;
    if (isDarkMode_) {
        applyDarkTheme();
    } else {
        applyLightTheme();
    }
}

} // namespace logviewer
