FOR /R %cd% %%G IN (*.hlsl) DO %VULKAN_SDK%/Bin32/glslc.exe %%G -o "%%~nG.spv"
pause