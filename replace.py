
paths = [
    r"FireEngine.sln",
    r"engine\core\CGLab2.vcxproj",
    r"engine\core\CGLab2.vcxproj.user",
    r"engine\core\main.cpp",
    r"engine\mono\Mono.csproj",
]

# "исходная строка": "замена"
strings = {
# Путь до DirectXTK-main\Inc
    r"C:\Users\Dmitry\Desktop\DirectXTK-main\DirectXTK-main\Inc":
    r"C:\Users\Dmitry\Desktop\DirectXTK-main\DirectXTK-main\Inc",

# Путь до собранной DirectXTK.lib
    r"C:\Users\Dmitry\Desktop\DirectXTK-main\DirectXTK-main\Bin\Desktop_2019_Win10\x64\Debug":
    r"C:\Users\Dmitry\Desktop\DirectXTK-main\DirectXTK-main\Bin\Desktop_2019_Win10\x64\Debug",

# Путь до Mono\include\mono-2.0
    r"C:\Program Files\Mono\include\mono-2.0;":
    r"C:\Program Files\Mono\include\mono-2.0;",

# Путь до Mono\lib
    r"C:\Program Files\Mono\lib":
    r"C:\Program Files\Mono\lib",

# Путь до Mono\bin
    r"PATH=C:\Program Files\Mono\bin":
    r"PATH=C:\Program Files\Mono\bin",

# Выходной путь FireEngine.Mono
    r"C:/GitHub/FireEngine/x64/Debug/":
    r"C:/GitHub/FireEngine/x64/Debug/",

# Путь до выходных файлов FireEngine.Mono
    r"C:/GitHub/FireEngine/x64/Debug/netstandard2.1/Mono.dll":
    r"C:/GitHub/FireEngine/x64/Debug/netstandard2.1/Mono.dll",
}

for path in paths:
    orig = ""
    with open(path, "r") as inp:
        orig = inp.read()
    text = orig
    for source, target in strings.items():
        text = text.replace(source, target)
    with open(path + ".orig", "w") as out:
        out.write(orig)
    with open(path, "w") as out:
        out.write(text)
