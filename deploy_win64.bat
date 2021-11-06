set UnrealPluginDir="D:\Projects\VPDemo\Plugins\VideoPlayer"
set UnrealPluginModuleName="VideoPlayer"
set DeployTarget=Debug

rmdir /s /q %UnrealPluginDir%\Intermediate

del /f /s /q %UnrealPluginDir%\Binaries\Win64\UE4Editor*

echo y | xcopy .\build\win64\%DeployTarget%\*.dll %UnrealPluginDir%\Binaries\Win64\ /s /e /y

echo y | xcopy .\build\win64\%DeployTarget%\*.pdb %UnrealPluginDir%\Binaries\Win64\ /s /e /y

echo y | xcopy .\build\win64\%DeployTarget%\*.lib %UnrealPluginDir%\Source\%UnrealPluginModuleName%\Lib\ /s /e /y

echo y | xcopy .\src\*.h %UnrealPluginDir%\Source\%UnrealPluginModuleName%\Public\ /s /e /y
