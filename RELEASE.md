# LogViewer 发布打包指南

## Release 构建优化

### 1. 清理之前的构建
```bash
cd log-viewer
rm -rf build
mkdir build && cd build
```

### 2. 配置Release构建
```bash
# MinGW
cmake -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release ..

# Visual Studio
cmake -G "Visual Studio 17 2022" ..
```

### 3. 编译优化版本
```bash
cmake --build . --config Release -j8
```

### 4. 查找生成的exe
```bash
# MinGW
ls bin/LogViewer.exe

# Visual Studio
ls bin/Release/LogViewer.exe
```

## 减小exe体积

### 方法1：Strip符号信息（MinGW）
```bash
strip bin/LogViewer.exe
# 可减小约30-50%体积
```

### 方法2：使用UPX压缩
```bash
# 下载 UPX: https://upx.github.io/
upx --best bin/LogViewer.exe
# 可进一步减小50-70%体积
```

**注意**：UPX压缩后的exe可能被某些杀毒软件误报，建议保留未压缩版本。

## 依赖检查

### 检查动态库依赖（MinGW）
```bash
ldd bin/LogViewer.exe
```

应该只依赖系统库：
- kernel32.dll
- msvcrt.dll
- opengl32.dll
- gdi32.dll
- user32.dll

如果有其他依赖（如libgcc_s、libstdc++），需要：
1. 复制dll到exe同目录
2. 或在CMakeLists.txt中添加静态链接选项

## 打包分发

### 创建发布包
```bash
cd log-viewer
mkdir release
cp build/bin/LogViewer.exe release/
cp -r examples release/
cp README.md release/
cp USER_GUIDE.md release/
cp LICENSE release/  # 如果有
```

### 压缩
```bash
# 创建zip包
cd release
zip -r LogViewer-v1.0.0-win64.zip *

# 或使用7zip
7z a LogViewer-v1.0.0-win64.7z *
```

## 发布清单

**最终发布包应包含：**
```
LogViewer-v1.0.0-win64/
├── LogViewer.exe           # 主程序
├── README.md               # 项目说明
├── USER_GUIDE.md           # 使用指南
├── LICENSE                 # 许可证
└── examples/               # 示例文件
    ├── apache_format.json
    ├── sample_logs.txt
    ├── csv_format.json
    └── sample_csv_logs.txt
```

## 测试清单

发布前务必测试：

- [ ] 在干净的Windows系统上运行
- [ ] 导入示例格式
- [ ] 导入示例日志
- [ ] 搜索功能
- [ ] 格式导出/导入
- [ ] 界面中文显示正常
- [ ] 无运行时错误或崩溃

## 版本号管理

在 `CMakeLists.txt` 中修改版本号：
```cmake
project(LogViewer VERSION 1.0.0 LANGUAGES C CXX)
```

在 `Application.cpp` 的 About 对话框中同步更新版本号。

## 发布渠道

- **GitHub Releases**: 上传zip包和changelog
- **官网下载**: 提供直接下载链接
- **病毒扫描**: 使用 VirusTotal 扫描并附上报告链接

## Changelog 模板

```markdown
# LogViewer v1.0.0 (2026-02-11)

## 新功能
- ✨ 首次发布
- ✨ 支持自定义日志格式定义
- ✨ 分隔符解析引擎
- ✨ 实时搜索和过滤
- ✨ 格式导入/导出
- ✨ SQLite内存数据库

## 已知问题
- 大文件（>100MB）可能加载较慢
- 暂不支持正则表达式解析

## 系统要求
- Windows 10/11 (64-bit)
- OpenGL 3.0+
- 4GB RAM 推荐
```

## 许可证说明

项目使用的第三方库：
- **GLFW**: zlib/libpng License
- **ImGui**: MIT License
- **nlohmann/json**: MIT License
- **SQLite**: Public Domain

确保在README中声明所有第三方库的许可证。
