for /F %%i in ('dir /b ^| findstr /v /i "\.bat$" ^| findstr /v /i "\.spv$"') do %VULKAN_SDK%/Bin32/glslc.exe "%%i" -o "%cd%\%%~ni.spv"
pause