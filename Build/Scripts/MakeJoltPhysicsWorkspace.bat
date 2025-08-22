@echo off
cd ../../Extern/JoltPhysics/Build

del /f "VS2022_CL_Double/CMakeCache.txt"
call cmake_vs2022_cl_double.bat -DUSE_STATIC_MSVC_RUNTIME_LIBRARY=OFF -DDISABLE_CUSTOM_ALLOCATOR=ON ^
	-DUSE_AVX2=OFF -DUSE_AVX512=OFF -DUSE_F16C=OFF -DUSE_FMADD=OFF
pause