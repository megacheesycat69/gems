#PR
# GEMS
## Сборка

### Требования
- CMake ≥ 3.16
- Компилятор с поддержкой C++17 (GCC 9+, Clang 9+, MSVC 2019+)
- Git (для загрузки SFML через FetchContent)
### Windows (MSVC)
```powershell
cmake -B build
cmake --build build --config Release
.\build\Release\GEMS.exe
```
