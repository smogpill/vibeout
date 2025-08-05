SetProjectDefaults("Shaders")
kind "Utility"
files { "**.vert", "**.frag", "**.comp", "**.tesc", "**.tese", "**.geom", "**.glsl", "**.h" }
shaderFilter = 'files:**.vert or **.frag or **.comp or **.tesc or **.tese or **.geom'
shaderOutPath = "$(OutDir)/Shaders/%{file.name}.spv"

filter {shaderFilter}
	buildmessage 'Compiling %{file.relpath}'
	--buildcommands { '$(GLSLANG)/glslangValidator.exe -V --target-env spirv1.3 -d -o "'.. shaderOutPath ..'" %{file.relpath}' }
	buildoutputs { shaderOutPath }
filter {shaderFilter, "configurations:Debug or Dev"}
	buildcommands { '$(GLSLANG)/glslangValidator.exe -V -gVS --target-env spirv1.3 -d -o "'.. shaderOutPath ..'" %{file.relpath}' }
	--buildcommands { '$(VULKAN_SDK)/bin/glslc.exe -g --target-spv=spv1.3 -o "'.. shaderOutPath ..'" %{file.relpath}' }
filter {shaderFilter, "configurations:release"}
	buildcommands { '$(GLSLANG)/glslangValidator.exe -V -g0 --target-env spirv1.3 -d -o "'.. shaderOutPath ..'" %{file.relpath}' }
	--buildcommands { '$(VULKAN_SDK)/bin/glslc.exe -O --target-spv=spv1.3 -o "'.. shaderOutPath ..'" %{file.relpath}' }