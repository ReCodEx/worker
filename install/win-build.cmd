:: ***************************************************************************
::              Windows compilation script for ReCodEx worker
:: ***************************************************************************
:: Configuration: Release; Platform: x86
:: Should be run from 32bit version of Developer Command Prompt for VS2015
:: Usage: win-build.cmd
::   -build - builds recodex-worker
::   -clean - cleans built application and all temporary/downloaded files
::   -test - run tests
::   -package - creates Windows installer using NSIS


:: turn off outputting of executed commands
@echo off
:: delayed environment variable expansion
setlocal enabledelayedexpansion
:: all changes to variables will be local
setlocal

SET CURRENT_DIR=%cd%
SET DEFAULT_DIR=%CURRENT_DIR%\..
SET BUILD_DIR=%CURRENT_DIR%\..\build
SET BIN_DIR=%BUILD_DIR%\Release
SET TESTS_BUILD_DIR=%BUILD_DIR%\tests
SET TESTS_DIR=%DEFAULT_DIR%\tests
SET LIBS_DIR=%CURRENT_DIR%\..\libs
SET NUGET=%LIBS_DIR%\nuget.exe
SET BOOST_DIR=%LIBS_DIR%\boost
SET CURL_DIR=%LIBS_DIR%\libcurl
SET ZMQ_DIR=%LIBS_DIR%\libzmq
SET CA_BUNDLE=curl-ca-bundle.crt
SET CA_BUNDLE_PATH=%BUILD_DIR%\%CA_BUNDLE%


:: parameter was provided
if NOT [%1]==[] (
	if "%1"=="-clean" ( call :cleanup & goto exit )
	if "%1"=="-test" ( call :build & call :test & goto exit )
	if "%1"=="-package" ( call :build & call :package & goto exit )
	if "%1"=="-build" ( call :build & goto exit )
	
	echo Unknown option...
	goto exit
)

:: no parameter was provided, default is building source code
call :build
goto exit


:exit
:: at the end return to directory where we started
cd %CURRENT_DIR%
goto :EOF

:error
cd %CURRENT_DIR%
echo Failed!
exit /b 1


:: *******************************
:: *** Definition of functions ***
:: *******************************

:: *** CLEANUP ***
:cleanup
echo Cleaning...
rmdir /S /Q %BUILD_DIR%
rmdir /S /Q %LIBS_DIR%
goto :EOF

:: *** TEST ***
:test
echo Running tests...
:: set path variable to location with dlls
SET "PATH=%PATH%;%BIN_DIR%"
:: execute tests
cd %TESTS_BUILD_DIR%
copy %CA_BUNDLE_PATH% %TESTS_BUILD_DIR%\%CA_BUNDLE%
ctest -C Release --output-on-failure
goto :EOF

:: *** PACKAGE ***
:package
echo Creating package...
cd %BUILD_DIR%
cmake --build . --config Release --target PACKAGE
goto :EOF

:: *** COMPILATION ***
:build
:: download git submodules if necessary
cd %DEFAULT_DIR%
git submodule update --init

:: create libraries directory with downloaded ones using NuGet, can fail
if not exist %LIBS_DIR% ( mkdir %LIBS_DIR% )
:: directory for building binaries
if not exist %BUILD_DIR% ( mkdir %BUILD_DIR% )

:: download nuget to current directory
if not exist %NUGET% (
	bitsadmin /transfer nugetDownload /download /priority high https://dist.nuget.org/win-x86-commandline/latest/nuget.exe %NUGET%
)

:: change directory to folder with libraries and download them with NuGet
cd %LIBS_DIR%
echo Downloading NuGet packages...
%NUGET% install boost -ExcludeVersion -Version 1.60.0
%NUGET% install boost_system-vc140 -ExcludeVersion -Version 1.60.0
%NUGET% install boost_filesystem-vc140 -ExcludeVersion -Version 1.60.0
%NUGET% install boost_program_options-vc140 -ExcludeVersion -Version 1.60.0
%NUGET% install rmt_zlib -ExcludeVersion -Version 1.2.8.6
%NUGET% install rmt_libssh2 -ExcludeVersion -Version 1.6.0.2
%NUGET% install rmt_curl -ExcludeVersion -Version 7.47.1
%NUGET% install fix8.dependencies.zmq -ExcludeVersion

:: move downloaded libraries to folders with more logical structure
echo Moving downloaded libraries...
move %BOOST_DIR%\lib\native\include %BOOST_DIR%
move %LIBS_DIR%\boost_system-vc140\lib\native\address-model-32\lib\*.lib %BOOST_DIR%\lib
move %LIBS_DIR%\boost_filesystem-vc140\lib\native\address-model-32\lib\*.lib %BOOST_DIR%\lib
move %LIBS_DIR%\boost_program_options-vc140\lib\native\address-model-32\lib\*.lib %BOOST_DIR%\lib
if not exist %CURL_DIR%\lib ( mkdir %CURL_DIR%\lib )
move %LIBS_DIR%\rmt_curl\build\native\include %CURL_DIR%
move %LIBS_DIR%\rmt_curl\build\native\lib\v140\Win32\Release\dynamic\*.lib %CURL_DIR%\lib 
if not exist %ZMQ_DIR%\lib ( mkdir %ZMQ_DIR%\lib )
move %LIBS_DIR%\fix8.dependencies.zmq\build\native\include %ZMQ_DIR%
move %LIBS_DIR%\fix8.dependencies.zmq\build\native\lib\Win32\v140\Release\Desktop\*.lib %ZMQ_DIR%\lib 

:: download certificate bundle
echo Downloading certificate bundle...
if not exist %CA_BUNDLE_PATH% (
	bitsadmin /transfer certDownload /download /priority high http://curl.haxx.se/ca/cacert.pem %CA_BUNDLE_PATH%
)

:: run cmake and generate nmake file in build directory
echo Running cmake...
cd %BUILD_DIR%
cmake -G "Visual Studio 14 2015" -DBOOST_ROOT=%BOOST_DIR% -DCURL_LIBRARY=%CURL_DIR%\lib\libcurl.lib -DCURL_INCLUDE_DIR=%CURL_DIR%\include -DZMQ_LIBRARY_DIR=%ZMQ_DIR%\lib -DZMQ_INCLUDE_DIR=%ZMQ_DIR%\include %DEFAULT_DIR% || goto error

:: run generated makefile in build directory
echo Compiling all targets...
cd %BUILD_DIR%
cmake --build . --config Release --target ALL_BUILD || goto error

:: copy dll libraries to build directory
copy /Y %LIBS_DIR%\fix8.dependencies.zmq\build\native\bin\Win32\v140\Release\Desktop\*.dll %BIN_DIR%
copy /Y %LIBS_DIR%\rmt_curl\build\native\bin\v140\Win32\Release\dynamic\*.dll %BIN_DIR%
copy /Y %LIBS_DIR%\rmt_libssh2\build\native\bin\v140\Win32\Release\dynamic\*.dll %BIN_DIR%
copy /Y %LIBS_DIR%\rmt_zlib\build\native\bin\v140\Win32\Release\dynamic\*.dll %BIN_DIR%

goto :EOF
