# nlohmann/json 라이브러리 다운로드 스크립트
$version = "3.11.3"
$url = "https://github.com/nlohmann/json/releases/download/v$version/json.hpp"
$dest = "SampleOrderSystem\include\nlohmann\json.hpp"

New-Item -ItemType Directory -Force -Path "SampleOrderSystem\include\nlohmann" | Out-Null

Write-Host "nlohmann/json v$version 다운로드 중..." -ForegroundColor Cyan
Invoke-WebRequest -Uri $url -OutFile $dest
Write-Host "완료: $dest" -ForegroundColor Green
