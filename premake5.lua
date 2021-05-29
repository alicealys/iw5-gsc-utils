dependencies = {
	basePath = "./deps"
}

function dependencies.load()
	dir = path.join(dependencies.basePath, "premake/*.lua")
	deps = os.matchfiles(dir)

	for i, dep in pairs(deps) do
		dep = dep:gsub(".lua", "")
		require(dep)
	end
end

function dependencies.imports()
	for i, proj in pairs(dependencies) do
		if type(i) == 'number' then
			proj.import()
		end
	end
end

function dependencies.projects()
	for i, proj in pairs(dependencies) do
		if type(i) == 'number' then
			proj.project()
		end
	end
end

dependencies.load()

workspace "iw5-gsc-utils"
	location "./build"
	objdir "%{wks.location}/obj/%{cfg.buildcfg}"
	targetdir "%{wks.location}/bin/%{cfg.buildcfg}"
	targetname "%{prj.name}"

	language "C++"

	architecture "x86"

	buildoptions "/std:c++latest"
	systemversion "latest"

	flags
	{
		"NoIncrementalLink",
		"MultiProcessorCompile",
	}

	configurations { "Debug", "Release", }

	symbols "On"
	
	configuration "Release"
		optimize "Full"
		defines { "NDEBUG" }
	configuration{}

	configuration "Debug"
		optimize "Debug"
		defines { "DEBUG", "_DEBUG" }
	configuration {}

	startproject "iw5-gsc-utils"

    project "iw5-gsc-utils"
        kind "SharedLib"
        language "C++"

        pchheader "stdinc.hpp"
		pchsource "src/stdinc.cpp"

        includedirs
        {
       		"src"
    	}
        
        files
        {
            "src/**.h",
            "src/**.hpp",
            "src/**.cpp"
		}

		dependencies.imports()
	
	group "Dependencies"
	dependencies.projects()
