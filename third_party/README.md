# 第三方依赖说明

本项目需要以下第三方库，请按照说明放置到对应目录。

## 目录结构
```
third_party/
├── glfw/           # GLFW库（完整源码）
├── imgui/          # ImGui库（源码文件）
├── json/           # nlohmann/json（单头文件）
└── sqlite/         # SQLite3（amalgamation）
```

## 1. GLFW (窗口管理)
**版本**: 3.3.8 或更高
**下载**: https://github.com/glfw/glfw/releases

**安装步骤**:
```bash
cd third_party
git clone https://github.com/glfw/glfw.git
cd glfw
git checkout 3.3.8
```

或直接下载源码包解压到 `third_party/glfw/`

## 2. ImGui (GUI框架)
**版本**: 1.90 或更高  
**下载**: https://github.com/ocornut/imgui/releases

**安装步骤**:
```bash
cd third_party
git clone https://github.com/ocornut/imgui.git
cd imgui
git checkout v1.90.1
```

**必需文件**:
- imgui/*.cpp
- imgui/*.h
- imgui/backends/imgui_impl_glfw.cpp
- imgui/backends/imgui_impl_glfw.h
- imgui/backends/imgui_impl_opengl3.cpp
- imgui/backends/imgui_impl_opengl3.h

## 3. nlohmann/json (JSON解析)
**版本**: 3.11.2 或更高
**下载**: https://github.com/nlohmann/json/releases

**安装步骤**:
```bash
cd third_party
mkdir -p json/include/nlohmann
curl -o json/include/nlohmann/json.hpp https://github.com/nlohmann/json/releases/download/v3.11.3/json.hpp
```

或直接创建目录结构:
```
json/
└── include/
    └── nlohmann/
        └── json.hpp
```

## 4. SQLite3 (嵌入式数据库)
**版本**: 3.42.0 或更高
**下载**: https://www.sqlite.org/download.html

下载 **sqlite-amalgamation** 版本

**安装步骤**:
```bash
cd third_party
mkdir sqlite
# 下载 sqlite-amalgamation-3420000.zip 并解压
# 复制 sqlite3.c 和 sqlite3.h 到 sqlite/ 目录
```

**必需文件**:
- sqlite3.c
- sqlite3.h

## 自动化安装脚本

### Windows (PowerShell)
运行 `setup_dependencies.ps1`:
```powershell
.\setup_dependencies.ps1
```

### Linux/macOS (Bash)
运行 `setup_dependencies.sh`:
```bash
chmod +x setup_dependencies.sh
./setup_dependencies.sh
```

## 验证安装
确保目录结构如下：
```
third_party/
├── glfw/
│   ├── CMakeLists.txt
│   ├── include/
│   └── src/
├── imgui/
│   ├── imgui.cpp
│   ├── imgui.h
│   └── backends/
├── json/
│   └── include/
│       └── nlohmann/
│           └── json.hpp
└── sqlite/
    ├── sqlite3.c
    └── sqlite3.h
```

## 许可证说明
- GLFW: zlib/libpng license
- ImGui: MIT License
- nlohmann/json: MIT License
- SQLite3: Public Domain

所有库均为开源软件，可自由使用。
