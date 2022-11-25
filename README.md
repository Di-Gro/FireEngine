# FireEngine
### Нужно установить
- Mono
- DirectXTK

### Настройки проектов

monopp:
- с++14 | General -> C++ Language Standard
- "PATH=C:\Program Files\Mono\bin" | Debugging -> Environment
- "C:\Program Files\Mono\include\mono-2.0" | C/C++ -> General -> Additional Include Directories
- "mono-2.0-sgen.lib" | Librarian -> General -> Additional Dependencies
- "C:\Program Files\Mono\lib" | Librarian -> General -> Additional Library Directories
* настроить "mono_build_config.h", если Mono установлена не в "C:/Program Files/Mono"

Core:
- с++20 | General -> C++ Language Standard
- "PATH=C:\Program Files\Mono\bin" | Debugging -> Environment
- "$(SolutionDir)\engine\monopp; C:\Program Files\Mono\include\mono-2.0" | C/C++ -> General -> Additional Include Directories
- для Debug | "$(SolutionDir)\x64\Debug; C:\Program Files\Mono\lib" | Linker -> General -> Additional Library Directories
- для Release | "$(SolutionDir)\x64\Release; C:\Program Files\Mono\lib" | Linker -> General -> Additional Library Directories
- "DirectXTK.lib; monopp.lib; mono-2.0-sgen.lib;" | Linker -> Input -> Additional Dependencies
* в main.cpp ASSEMBLY_PATH - путь до C# .dll

Mono:
- OutputPath = "$(SolutionDir)\x64\Debug\"
- OutputPath = "$(SolutionDir)\x64\Release\"
