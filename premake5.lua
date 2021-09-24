project "imgui-node-editor"
   kind "StaticLib"
	language "C++"
	systemversion "latest"
	staticruntime "On"

	targetdir (vartargetdir .. "/%{prj.name}")
	objdir (varobjdir .. "/%{prj.name}")

	files {
		"crude_json.cpp", 
		"crude_json.h", 
		"imgui_bezier_math.h", 
		"imgui_bezier_math.inl", 
		"imgui_canvas.cpp", 
		"imgui_canvas.h", 
		"imgui_extra_math.h", 
		"imgui_extra_math.inl", 
		"imgui_node_editor.cpp", 
		"imgui_node_editor.h", 
		"imgui_node_editor_api.cpp", 
		"imgui_node_editor_internal.h",
		"imgui_node_editor_internal.inl"
	}

	includedirs {
		"../imgui"
	}

	filter "configurations:Debug"
		runtime "Debug"
		symbols "On"

	filter "configurations:Release"
		runtime "Release"
		optimize "On"
