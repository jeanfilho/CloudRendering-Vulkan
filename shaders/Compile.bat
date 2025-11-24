for /F %%i in ('dir /b ^| findstr /v /i "\.bat$" ^| findstr /v /i "\.spv$"') do %VULKAN_SDK%\Bin\glslc.exe "%%i" -o "%cd%\%%i.spv"
pause