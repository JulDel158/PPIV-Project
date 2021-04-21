cmake_minimum_required(VERSION 3.16)

project(GreenScreen)
# by default CMake selects "ALL_BUILD" as the startup project 
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} 
	PROPERTY VS_STARTUP_PROJECT GreenScreen)

# currently using unicode in some libraries on win32 but will change soon
ADD_DEFINITIONS(-DUNICODE)
ADD_DEFINITIONS(-D_UNICODE)

# CMake FXC shader compilation, add any shaders you want compiled here
set(VERTEX_SHADERS 
	# add vertex shader (.hlsl) files here
	SkyBoxVertexShader.hlsl
	InstanceVertexShader.hlsl
	VertexShader.hlsl
	Wave_VS.hlsl
)
set(PIXEL_SHADERS 
	# add pixel shader (.hlsl) files here
	SkyBoxPixelShader.hlsl
	InstancePixelShader.hlsl
	PixelShader.hlsl
)

# Add any new C/C++ source code here
add_executable (GreenScreen 
	# Header & CPP files go here
	main.cpp
	renderer.h
	test_pyramid.h
	VertexShader.h
	PixelShader.h
	InstancePixelShader.h
	InstanceVertexShader.h
	SkyBoxPixelShader.h
	SkyBoxVertexShader.h
	Renderable.h
	DDSTextureLoader.cpp
	DDSTextureLoader.h
	axe1.h
	axe2.h
	CUBE.h
	Wave_VS.h
	${VERTEX_SHADERS}
	${PIXEL_SHADERS}
)

# This code will tell visual studio to convert above shader .hlsl to .h files
# The produced .h files can be included and contain the compiled shader byte code
set_source_files_properties( ${VERTEX_SHADERS} PROPERTIES 
	VS_SHADER_TYPE Vertex VS_SHADER_MODEL 5.0 VS_SHADER_ENTRYPOINT main 
	VS_SHADER_OUTPUT_HEADER_FILE "%(RelativeDir)%(Filename).h"
	VS_SHADER_VARIABLE_NAME "%(Filename)"
	VS_SHADER_ENABLE_DEBUG $<$<CONFIG:Debug>:true>
)
set_source_files_properties( ${PIXEL_SHADERS} PROPERTIES 
	VS_SHADER_TYPE Pixel VS_SHADER_MODEL 5.0 VS_SHADER_ENTRYPOINT main 
	VS_SHADER_OUTPUT_HEADER_FILE "%(RelativeDir)%(Filename).h"
	VS_SHADER_VARIABLE_NAME "%(Filename)"
	VS_SHADER_ENABLE_DEBUG $<$<CONFIG:Debug>:true>
)
