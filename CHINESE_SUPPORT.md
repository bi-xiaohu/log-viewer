# 中文输入支持说明

## 问题原因
ImGui默认字体不包含中文字符，因此无法显示中文。虽然可以输入中文（Windows IME支持），但显示为方块。

## 解决方案

### 方案1：添加中文字体（推荐）

1. **下载中文字体**
   - 推荐：[思源黑体 (Noto Sans CJK)](https://github.com/googlefonts/noto-cjk/releases)
   - 或使用系统字体：`C:\Windows\Fonts\msyh.ttc`（微软雅黑）

2. **放置字体文件**
   ```
   log-viewer/
   └── assets/
       └── fonts/
           └── NotoSansSC-Regular.otf
   ```

3. **修改代码**
   在 `Application.cpp` 的 `initializeImGui()` 函数中取消注释：
   ```cpp
   // 当前（第70行左右）
   // io.Fonts->AddFontFromFileTTF("assets/fonts/NotoSansSC-Regular.otf", 18.0f, nullptr, io.Fonts->GetGlyphRangesChineseFull());
   
   // 改为
   io.Fonts->AddFontFromFileTTF("assets/fonts/NotoSansSC-Regular.otf", 18.0f, nullptr, io.Fonts->GetGlyphRangesChineseFull());
   ```

4. **重新编译**
   ```bash
   cd build
   cmake --build . --config Release
   ```

### 方案2：使用系统字体（简单）

修改代码使用系统自带的微软雅黑：
```cpp
// Windows系统
io.Fonts->AddFontFromFileTTF("C:/Windows/Fonts/msyh.ttc", 18.0f, nullptr, io.Fonts->GetGlyphRangesChineseFull());

// 或使用宋体
io.Fonts->AddFontFromFileTTF("C:/Windows/Fonts/simsun.ttc", 18.0f, nullptr, io.Fonts->GetGlyphRangesChineseFull());
```

### 方案3：仅支持ASCII（当前状态）

如果不需要中文显示，保持当前状态即可：
- 可以输入中文字符
- 但界面上显示为方块
- 数据库中正确存储UTF-8编码
- 导出JSON时显示正常

## 测试中文支持

1. 启动程序
2. 创建新格式：`Edit` → `New Format`
3. 在Name字段输入中文
4. 检查是否正确显示

## 字体文件大小说明

- Noto Sans SC Regular: ~7MB
- 微软雅黑: ~16MB（包含多个字重）
- 仅包含常用汉字: ~3-4MB

如果关心exe大小，可以：
1. 使用字体子集工具减小字体文件
2. 运行时动态加载字体（不打包进exe）
3. 使用更小的中文字体

## 相关资源

- [ImGui中文支持文档](https://github.com/ocornut/imgui/blob/master/docs/FONTS.md)
- [思源黑体下载](https://github.com/googlefonts/noto-cjk/releases)
- [字体子集工具](https://github.com/fonttools/fonttools)

## 当前状态

✅ 程序已支持UTF-8编码
✅ 数据库正确存储中文
✅ JSON导入导出支持中文
⚠️ 界面显示需要加载中文字体

**建议**：如果经常使用中文，按方案2修改使用系统字体最简单。
