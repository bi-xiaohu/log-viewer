# Copilot Instructions for Log Viewer

## Build Commands

### Dependencies Setup
```bash
# First-time setup: Download third-party libraries
cd third_party
./setup_dependencies.ps1  # Windows
```

### Build
```bash
# Configure (MinGW/MSYS2)
mkdir build && cd build
cmake -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release ..

# Or configure (Visual Studio)
cmake -G "Visual Studio 17 2022" ..

# Build
cmake --build . --config Release -j8

# Output: build/bin/LogViewer.exe
```

### Run
```bash
cd build/bin
./LogViewer.exe
```

## Architecture Overview

### Three-Layer Design
1. **Core Layer** (`src/core/`) - Business logic, data models, parsing
   - `LogFormat` - Log format definition with JSON serialization
   - `LogEntry` - Individual log record with field mapping
   - `LogParser` - Delimiter-based parser with auto-detection
   - `Database` - SQLite wrapper for in-memory storage

2. **GUI Layer** (`src/gui/`) - ImGui interface
   - `Application` - Main application class, owns window and database
   - Handles two-column layout: Format panel | Log table + Paste area
   - Theme system with Light/Dark modes

3. **Main** (`src/main.cpp`) - Entry point, minimal logic

### Data Flow
```
User Action → Application → Parser → Database → UI Render
                ↓              ↓         ↓
            LogFormat     LogEntry   SQLite
```

### Key Dependencies
- **ImGui** - Immediate-mode GUI (all rendering)
- **GLFW** - Window management and OpenGL context
- **nlohmann/json** - JSON parsing for format definitions
- **SQLite3** - In-memory database for log storage and search

## Code Conventions

### Namespace
All code uses `namespace logviewer` - never use global namespace.

### Naming
- Classes: `PascalCase` (e.g., `LogFormat`, `Application`)
- Methods: `camelCase` (e.g., `parseLines`, `loadFormats`)
- Private members: `camelCase_` with trailing underscore (e.g., `database_`, `isDarkMode_`)
- Constants: No specific convention, use context

### Memory Management
- Use `std::unique_ptr` for owned resources (e.g., `database_`)
- Use raw pointers for non-owning references (e.g., `currentFormat_`)
- ImGui window lifecycle managed by GLFW callbacks

### ImGui Patterns

#### Buffer Management
Fixed-size char arrays for ImGui inputs:
```cpp
char searchBuffer_[256] = "";
char pasteBuffer_[4096] = "";
```

#### Window Layout
```cpp
ImGui::Begin("Main", nullptr, ImGuiWindowFlags_NoTitleBar | ...);
ImGui::Columns(2, "MainColumns");
// Left column
ImGui::NextColumn();
// Right column
ImGui::Columns(1);
ImGui::End();
```

#### Theme Application
Call `applyLightTheme()` or `applyDarkTheme()` - modifies 50+ ImGuiCol_* values.

### JSON Format Schema
Log formats stored as JSON with this structure:
```json
{
  "id": "fmt_1234567890",
  "name": "Format Name",
  "description": "Optional description",
  "delimiter": " ",
  "fields": [
    {"name": "field1", "description": "Field description"}
  ]
}
```

### SQLite Tables
```sql
-- In-memory database, no persistence between runs
CREATE TABLE logs (
  id INTEGER PRIMARY KEY,
  format_id TEXT,
  raw_text TEXT,
  parsed_data TEXT  -- JSON blob
);

CREATE TABLE formats (
  id TEXT PRIMARY KEY,
  name TEXT,
  data TEXT  -- JSON blob
);
```

## Common Tasks

### Adding a New Core Class
1. Create `.h` and `.cpp` in `src/core/`
2. Use `namespace logviewer`
3. Add to `CMakeLists.txt` (auto-included via GLOB)
4. Include in `Application.h` if needed

### Adding UI Elements
- Add render function to `Application` class: `renderNewFeature()`
- Call from `render()` method
- Use ImGui immediate-mode API (no state objects)

### Modifying Log Parsing
- Edit `LogParser::parse()` for single-line logic
- Edit `LogParser::detectFormat()` for auto-detection
- Parsers are stateless - create per-operation

### Theme Customization
Edit `applyLightTheme()` or `applyDarkTheme()` in `Application.cpp`.
Colors use `ImVec4(r, g, b, a)` with 0.0-1.0 range.

## Windows-Specific Notes
- Use Windows-style paths with backslashes: `D:\path\to\file`
- File dialogs use Win32 API (`GetOpenFileNameA`)
- Static linking configured for MinGW: `-static-libgcc -static-libstdc++`

## Third-Party Library Structure
```
third_party/
├── glfw/           # Full source, CMake subdirectory
├── imgui/          # Source files, manual build
├── json/include/   # Header-only library
└── sqlite/         # Amalgamation (single .c file)
```

Do not modify third-party code. If updates needed, re-run `setup_dependencies.ps1`.

## Testing Strategy
No automated tests currently. Manual testing workflow:
1. Import format from `examples/apache_format.json`
2. Import logs from `examples/sample_logs.txt`
3. Verify parsing, search, and theme switching
4. Test paste area with 1-10 log lines

## Debugging
- Use `ImGui::Text()` for debug output in render loops
- Check console for SQLite errors from `Database::getLastError()`
- GLFW errors print to stderr
- ImGui validation enabled in Debug builds
