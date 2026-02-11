# Log Viewer - 现代C++日志查看器

一个基于C++20和ImGui的轻量级日志查看工具，支持自定义格式解析和可视化展示。

## ✨ 最新更新 (v1.1.0)

- 🎨 **白天/夜间主题** - 灰白色舒适主题，支持一键切换
- 📋 **粘贴区导入** - 快速测试1-10条日志
- 💡 **智能提示** - 鼠标悬停显示字段说明
- 🖼️ **布局优化** - 两列布局，更大显示区域

详见 [CHANGELOG_v1.1.0.md](CHANGELOG_v1.1.0.md)

## 功能特性

- ✅ **双主题界面** - 白天模式（灰白）+ 夜间模式（深色）
- ✅ **快速导入** - 粘贴区+文件导入双通道
- ✅ **自定义日志格式** - 灵活定义字段和分隔符
- ✅ **智能格式检测** - 自动识别常见日志格式
- ✅ **格式导入/导出** - JSON格式，易于分享
- ✅ **增强提示** - 字段悬停显示名称、描述、值
- ✅ **批量导入** - 支持大型日志文件
- ✅ **实时搜索** - SQLite全文搜索
- ✅ **单exe文件** - 无需安装，开箱即用
- ✅ **跨平台** - 支持Windows、Linux、macOS

## 截图

### 主界面布局（白天模式）
```
┌─────────────────────────────────────────────────────────────┐
│ File  Edit  View  Help                      [Theme: Light]  │
├──────────┬──────────────────────────────────────────────────┤
│ Formats  │       Log Table                                  │
│          │ ┌─────────────────────────────────────────────┐  │
│ Apache   │ │ID | IP        | User  | Status | Size      │  │
│ CSV      │ │1  | 192.168.1 | admin | 200    | 1024     │  │
│ Custom   │ │2  | 10.0.0.1  | guest | 404    | 512      │  │
│          │ └─────────────────────────────────────────────┘  │
│ [New]    ├──────────────────────────────────────────────────┤
│          │       Paste Area (Quick Import)                  │
│          │ ┌─────────────────────────────────────────────┐  │
│          │ │ 192.168.1.1 - admin [timestamp] GET ...     │  │
│          │ │ 192.168.1.2 - guest [timestamp] POST ...    │  │
│          │ └─────────────────────────────────────────────┘  │
│          │ [Import from Paste][From File][Clear]           │
└──────────┴──────────────────────────────────────────────────┘
```

### 鼠标悬停提示
```
┌─────────────────────┐
│ [ip] ← 字段名       │
│ 客户端IP地址        │
│ ─────────────────── │
│ Value: 192.168.1.1 │
└─────────────────────┘
```

## 构建说明

### 环境要求
- CMake 3.20+
- C++20编译器（MSVC 2022 / GCC 11+ / Clang 14+）
- OpenGL 3.0+
- Git（用于下载依赖）

### 快速构建（推荐）

#### Windows (MSYS2/MinGW)
```bash
# 1. 克隆项目
git clone <repo-url>
cd log-viewer

# 2. 下载依赖（自动化脚本）
cd third_party
./setup_dependencies.ps1

# 3. 构建项目
cd ..
mkdir build && cd build
cmake -G "MinGW Makefiles" ..
cmake --build . --config Release

# 4. 运行程序
./bin/LogViewer.exe
```

#### Windows (Visual Studio)
```bash
# 步骤1-2 同上

# 3. 构建项目
mkdir build && cd build
cmake -G "Visual Studio 17 2022" ..
cmake --build . --config Release

# 4. 运行程序
./bin/Release/LogViewer.exe
```

#### Linux
```bash
# 1. 安装依赖
sudo apt-get install cmake g++ libgl1-mesa-dev libx11-dev libxrandr-dev libxinerama-dev libxcursor-dev libxi-dev

# 2-4. 同 Windows MinGW 步骤
```

## 使用指南

### 快速开始

1. **运行程序**
   ```bash
   cd build/bin
   ./LogViewer.exe
   ```

2. **切换主题（可选）**
   - View → Toggle Theme
   - 或按 Ctrl+T

3. **导入格式定义**
   - 方式1：File → Import Format → 选择 `examples/apache_format.json`
   - 方式2：Edit → New Format 手动创建

4. **导入日志数据**
   
   **快速测试（粘贴区）：**
   - 复制1-10条日志
   - 粘贴到底部粘贴区
   - 点击 "Import from Paste"
   
   **批量导入（文件）：**
   - 点击 "Import from File"
   - 选择日志文件（如 `examples/sample_logs.txt`）

5. **查看和分析**
   - 表格显示所有日志记录
   - 鼠标悬停在字段上查看说明
   - 使用搜索框过滤日志

### 示例数据

项目提供了示例格式和日志数据：

```bash
examples/
├── apache_format.json      # Apache日志格式
├── sample_logs.txt         # Apache日志示例
├── csv_format.json         # CSV格式
└── sample_csv_logs.txt     # CSV日志示例
```

**试用步骤：**
1. 导入 `apache_format.json` 格式
2. 导入 `sample_logs.txt` 日志文件
3. 查看解析结果

详细使用说明见 [USER_GUIDE.md](USER_GUIDE.md)

## 项目结构
```
log-viewer/
├── src/
│   ├── core/        # 核心逻辑（解析、存储）
│   ├── gui/         # GUI界面
│   └── utils/       # 工具类
├── third_party/     # 第三方库
├── assets/          # 资源文件
└── CMakeLists.txt   # 构建配置
```

## 许可证
MIT License

## 贡献
欢迎提交Issue和Pull Request！
