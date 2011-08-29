--
-- XreaL build configuration script
-- 
solution "XreaL"
	--configurations { "Release", "ReleaseWithSymbols", "Debug" }
	configurations { "Release", "Debug" }
	platforms {"native", "x32", "x64"}
	
	--
	-- Release/Debug Configurations
	--
	configuration "Release"
		defines     "NDEBUG"
		flags      
		{
			"OptimizeSpeed",
			"EnableSSE",
			--"StaticRuntime"
		}
		
	--configuration "ReleaseReleaseWithSymbols"
	--	defines     "NDEBUG"
	--	flags
	--	{
	--		"OptimizeSpeed",
	--		"EnableSSE",
	--		"Symbols",
	--		"StaticRuntime"
	--	}
	
	configuration "Debug"
		defines     "_DEBUG"
		flags
		{
			"Symbols",
			--"StaticRuntime",
			--"NoRuntimeChecks"
		}
	
--
-- Options
--
newoption
{
	trigger = "with-java",
	description = "Compile with Java game code support"
}

newoption
{
	trigger = "with-mono",
	description = "Compile with Mono C# game code support"
}

--newoption
--{
--	trigger = "with-omnibot",
--	description = "Compile with Omni-bot support"
--}

--newoption
--{
--	trigger = "with-freetype",
--	description = "Compile with freetype support"
--}
		
--newoption
--{
--	trigger = "with-openal",
--	value = "TYPE",
--	description = "Specify which OpenAL library",
--	allowed = 
--	{
--		{ "none", "No support for OpenAL" },
--		{ "dlopen", "Dynamically load OpenAL library if available" },
--		{ "link", "Link the OpenAL library as normal" },
--		{ "openal-dlopen", "Dynamically load OpenAL library if available" },
--		{ "openal-link", "Link the OpenAL library as normal" }
--	}
--}

--		
-- Platform specific defaults
--

-- We don't support freetype on VS platform
--if _ACTION and string.sub(_ACTION, 2) == "vs" then
--	_OPTIONS["with-freetype"] = false
--end

-- Default to dlopen version of OpenAL
--if not _OPTIONS["with-openal"] then
--	_OPTIONS["with-openal"] = "dlopen"
--end
--if _OPTIONS["with-openal"] then
--	_OPTIONS["with-openal"] = "openal-" .. _OPTIONS["with-openal"]
--end

include "code/engine"

-- Don't build the the C game code if Java is enabled
if not _OPTIONS["with-java"] and not _OPTIONS["with-mono"] then
	include "base/code/game"
	include "base/code/cgame"
	include "base/code/ui"
end

include "code/tools/xmap2"
include "code/tools/master"

