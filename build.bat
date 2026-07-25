@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars32.bat" >nul

set ROOT=%~dp0
if "%ROOT:~-1%"=="\" set ROOT=%ROOT:~0,-1%
set SDKINC=%ROOT%\deps\plutonium-sdk
set GSLINC=%ROOT%\deps\GSL\include
set MHINC=%ROOT%\deps\minhook\include
set DEPSINC=%ROOT%\deps
set OBJDIR=%ROOT%\build\obj
set INCLUDES=/I "%ROOT%\src" /I "%SDKINC%" /I "%GSLINC%" /I "%MHINC%" /I "%DEPSINC%"
set CPPFLAGS=/nologo /c /std:c++20 /MT /EHsc /W3 /DUNICODE /D_UNICODE %INCLUDES%
set CFLAGS=/nologo /c /W3 /DUNICODE /D_UNICODE /I "%MHINC%"

if not exist "%OBJDIR%" mkdir "%OBJDIR%"

set FAIL=0

cl.exe %CPPFLAGS% "%ROOT%\src\stdinc.cpp" /Fo:"%OBJDIR%\stdinc.obj"
if errorlevel 1 set FAIL=1
cl.exe %CPPFLAGS% "%ROOT%\src\utils\memory.cpp" /Fo:"%OBJDIR%\utils_memory.obj"
if errorlevel 1 set FAIL=1
cl.exe %CPPFLAGS% "%ROOT%\src\utils\string.cpp" /Fo:"%OBJDIR%\utils_string.obj"
if errorlevel 1 set FAIL=1
cl.exe %CPPFLAGS% "%ROOT%\src\utils\hook.cpp" /Fo:"%OBJDIR%\utils_hook.obj"
if errorlevel 1 set FAIL=1
cl.exe %CPPFLAGS% "%ROOT%\src\utils\io.cpp" /Fo:"%OBJDIR%\utils_io.obj"
if errorlevel 1 set FAIL=1
cl.exe %CPPFLAGS% "%ROOT%\src\game\scripting\variable_value.cpp" /Fo:"%OBJDIR%\scripting_variable_value.obj"
if errorlevel 1 set FAIL=1
cl.exe %CPPFLAGS% "%ROOT%\src\game\scripting\vector.cpp" /Fo:"%OBJDIR%\scripting_vector.obj"
if errorlevel 1 set FAIL=1
cl.exe %CPPFLAGS% "%ROOT%\src\game\scripting\entity.cpp" /Fo:"%OBJDIR%\scripting_entity.obj"
if errorlevel 1 set FAIL=1
cl.exe %CPPFLAGS% "%ROOT%\src\game\scripting\array.cpp" /Fo:"%OBJDIR%\scripting_array.obj"
if errorlevel 1 set FAIL=1
cl.exe %CPPFLAGS% "%ROOT%\src\game\scripting\function.cpp" /Fo:"%OBJDIR%\scripting_function.obj"
if errorlevel 1 set FAIL=1
cl.exe %CPPFLAGS% "%ROOT%\src\game\scripting\script_value.cpp" /Fo:"%OBJDIR%\scripting_script_value.obj"
if errorlevel 1 set FAIL=1
cl.exe %CPPFLAGS% "%ROOT%\src\game\scripting\execution.cpp" /Fo:"%OBJDIR%\scripting_execution.obj"
if errorlevel 1 set FAIL=1
cl.exe %CPPFLAGS% "%ROOT%\src\game\scripting\stack_isolation.cpp" /Fo:"%OBJDIR%\scripting_stack_isolation.obj"
if errorlevel 1 set FAIL=1
cl.exe %CPPFLAGS% "%ROOT%\src\component\command.cpp" /Fo:"%OBJDIR%\component_command.obj"
if errorlevel 1 set FAIL=1
cl.exe %CPPFLAGS% "%ROOT%\src\component\gsc.cpp" /Fo:"%OBJDIR%\component_gsc.obj"
if errorlevel 1 set FAIL=1
cl.exe %CPPFLAGS% "%ROOT%\src\component\io.cpp" /Fo:"%OBJDIR%\component_io.obj"
if errorlevel 1 set FAIL=1
cl.exe %CPPFLAGS% "%ROOT%\src\component\json.cpp" /Fo:"%OBJDIR%\component_json.obj"
if errorlevel 1 set FAIL=1
cl.exe %CPPFLAGS% "%ROOT%\src\component\userinfo.cpp" /Fo:"%OBJDIR%\component_userinfo.obj"
if errorlevel 1 set FAIL=1
cl.exe %CPPFLAGS% "%ROOT%\src\plugin.cpp" /Fo:"%OBJDIR%\plugin.obj"
if errorlevel 1 set FAIL=1
cl.exe %CPPFLAGS% "%ROOT%\src\dllmain.cpp" /Fo:"%OBJDIR%\dllmain.obj"
if errorlevel 1 set FAIL=1

cl.exe %CFLAGS% "%ROOT%\deps\minhook\src\buffer.c" /Fo:"%OBJDIR%\mh_buffer.obj"
if errorlevel 1 set FAIL=1
cl.exe %CFLAGS% "%ROOT%\deps\minhook\src\hook.c" /Fo:"%OBJDIR%\mh_hook.obj"
if errorlevel 1 set FAIL=1
cl.exe %CFLAGS% "%ROOT%\deps\minhook\src\trampoline.c" /Fo:"%OBJDIR%\mh_trampoline.obj"
if errorlevel 1 set FAIL=1
cl.exe %CFLAGS% "%ROOT%\deps\minhook\src\hde\hde32.c" /Fo:"%OBJDIR%\mh_hde32.obj"
if errorlevel 1 set FAIL=1
cl.exe %CFLAGS% "%ROOT%\deps\minhook\src\hde\hde64.c" /Fo:"%OBJDIR%\mh_hde64.obj"
if errorlevel 1 set FAIL=1

if %FAIL% NEQ 0 (
    echo BUILD FAILED - see errors above
    exit /b 1
)

echo === All objects compiled OK, linking ===
cl.exe /nologo /LD "%OBJDIR%\stdinc.obj" "%OBJDIR%\utils_memory.obj" "%OBJDIR%\utils_string.obj" "%OBJDIR%\utils_hook.obj" "%OBJDIR%\utils_io.obj" "%OBJDIR%\scripting_variable_value.obj" "%OBJDIR%\scripting_vector.obj" "%OBJDIR%\scripting_entity.obj" "%OBJDIR%\scripting_array.obj" "%OBJDIR%\scripting_function.obj" "%OBJDIR%\scripting_script_value.obj" "%OBJDIR%\scripting_execution.obj" "%OBJDIR%\scripting_stack_isolation.obj" "%OBJDIR%\component_command.obj" "%OBJDIR%\component_gsc.obj" "%OBJDIR%\component_io.obj" "%OBJDIR%\component_json.obj" "%OBJDIR%\component_userinfo.obj" "%OBJDIR%\plugin.obj" "%OBJDIR%\dllmain.obj" "%OBJDIR%\mh_buffer.obj" "%OBJDIR%\mh_hook.obj" "%OBJDIR%\mh_trampoline.obj" "%OBJDIR%\mh_hde32.obj" "%OBJDIR%\mh_hde64.obj" /Fe:"%ROOT%\build\iw5-gsc-utils.dll" /link /MACHINE:X86

exit /b %ERRORLEVEL%
