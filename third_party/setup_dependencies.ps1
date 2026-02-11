# 自动下载和配置第三方依赖
Write-Host "正在设置第三方依赖..." -ForegroundColor Green

$THIRD_PARTY_DIR = $PSScriptRoot

# 1. 下载 GLFW
Write-Host "`n[1/4] 下载 GLFW..." -ForegroundColor Cyan
if (!(Test-Path "$THIRD_PARTY_DIR\glfw")) {
    git clone --depth 1 --branch 3.3.8 https://github.com/glfw/glfw.git "$THIRD_PARTY_DIR\glfw"
} else {
    Write-Host "GLFW 已存在，跳过" -ForegroundColor Yellow
}

# 2. 下载 ImGui
Write-Host "`n[2/4] 下载 ImGui..." -ForegroundColor Cyan
if (!(Test-Path "$THIRD_PARTY_DIR\imgui")) {
    git clone --depth 1 --branch v1.90.1 https://github.com/ocornut/imgui.git "$THIRD_PARTY_DIR\imgui"
} else {
    Write-Host "ImGui 已存在，跳过" -ForegroundColor Yellow
}

# 3. 下载 nlohmann/json
Write-Host "`n[3/4] 下载 nlohmann/json..." -ForegroundColor Cyan
$JSON_DIR = "$THIRD_PARTY_DIR\json\include\nlohmann"
if (!(Test-Path "$JSON_DIR\json.hpp")) {
    New-Item -ItemType Directory -Force -Path $JSON_DIR | Out-Null
    $url = "https://github.com/nlohmann/json/releases/download/v3.11.3/json.hpp"
    Invoke-WebRequest -Uri $url -OutFile "$JSON_DIR\json.hpp"
    Write-Host "下载完成: json.hpp" -ForegroundColor Green
} else {
    Write-Host "nlohmann/json 已存在，跳过" -ForegroundColor Yellow
}

# 4. 下载 SQLite3
Write-Host "`n[4/4] 下载 SQLite3..." -ForegroundColor Cyan
$SQLITE_DIR = "$THIRD_PARTY_DIR\sqlite"
if (!(Test-Path "$SQLITE_DIR\sqlite3.c")) {
    New-Item -ItemType Directory -Force -Path $SQLITE_DIR | Out-Null
    
    # SQLite 版本
    $SQLITE_VERSION = "3440000"
    $SQLITE_ZIP = "sqlite-amalgamation-$SQLITE_VERSION.zip"
    $SQLITE_URL = "https://www.sqlite.org/2023/$SQLITE_ZIP"
    
    Write-Host "下载 SQLite amalgamation..." -ForegroundColor Cyan
    Invoke-WebRequest -Uri $SQLITE_URL -OutFile "$THIRD_PARTY_DIR\$SQLITE_ZIP"
    
    Write-Host "解压 SQLite..." -ForegroundColor Cyan
    Expand-Archive -Path "$THIRD_PARTY_DIR\$SQLITE_ZIP" -DestinationPath $THIRD_PARTY_DIR -Force
    
    # 复制文件
    Copy-Item "$THIRD_PARTY_DIR\sqlite-amalgamation-$SQLITE_VERSION\sqlite3.c" $SQLITE_DIR
    Copy-Item "$THIRD_PARTY_DIR\sqlite-amalgamation-$SQLITE_VERSION\sqlite3.h" $SQLITE_DIR
    
    # 清理
    Remove-Item "$THIRD_PARTY_DIR\$SQLITE_ZIP"
    Remove-Item "$THIRD_PARTY_DIR\sqlite-amalgamation-$SQLITE_VERSION" -Recurse
    
    Write-Host "SQLite3 安装完成" -ForegroundColor Green
} else {
    Write-Host "SQLite3 已存在，跳过" -ForegroundColor Yellow
}

Write-Host "`n所有依赖安装完成！" -ForegroundColor Green
Write-Host "请运行以下命令构建项目:" -ForegroundColor Cyan
Write-Host "  mkdir build" -ForegroundColor White
Write-Host "  cd build" -ForegroundColor White
Write-Host "  cmake .." -ForegroundColor White
Write-Host "  cmake --build . --config Release" -ForegroundColor White
