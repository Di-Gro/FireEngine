# FireEngine

### Core

| Path | Conf. | Value |
| --- | --- | --- |
| VC++ Directories -> Include Directories | All | "$(SolutionDir)\engine\DirectXTK\Inc" |
| VC++ Directories -> Library Directories | All | $(SolutionDir)\engine\DirectXTK\x64\Debug |
| Debugging -> Environment | All | "PATH=C:\Program Files\Mono\bin" |
| General -> Language -> C++ Language Standard | All | C++20 |
| C/C++ -> General -> Additional Include Directories | All | "$(SolutionDir)engine\monopp; C:\Program Files\Mono\include\mono-2.0" |
| Linker -> General -> Additional Library Directories | Debug | "$(SolutionDir)x64\Debug; C:\Program Files\Mono\lib" |
| Linker -> General -> Additional Library Directories | Release | "$(SolutionDir)x64\Release; C:\Program Files\Mono\lib" |
| Linker -> Input -> Additional Dependencies | All | "DirectXTK.lib; monopp.lib; mono-2.0-sgen.lib" |
| Project Dependencies | | monopp |

### monopp

| Path | Conf. | Value |
| --- | --- | --- |
| General -> Language -> C++ Language Standard | All | C++17 |
| Debugging -> Environment | All | "PATH=C:\Program Files\Mono\bin" |
| C/C++ -> General -> Additional Include Directories | All | "C:\Program Files\Mono\include\mono-2.0" |
| Librarian -> General -> Additional Dependencies | All | "mono-2.0-sgen.lib" |
| Librarian -> General -> Additional Library Directories | All | "C:\Program Files\Mono\lib" |
| mono_build_config.h |  | настроить пути если Mono установлена не в "C:/Program Files/Mono" |
| Project Dependencies | | |

### Mono
Нужно установить Mono в "C:\Program Files\Mono"

| Path | Conf. | Value |
| --- | --- | --- |
| Project Dependencies | | vim_math |
