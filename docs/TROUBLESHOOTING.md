# Troubleshooting

## `cmake` or `ninja` is not recognized

- Confirm `mingw-w64-ucrt-x86_64-cmake` and `mingw-w64-ucrt-x86_64-ninja` are installed in MSYS2 UCRT64.
- Confirm `C:\\msys64\\ucrt64\\bin` is in the Windows user `Path`.
- Close every VS Code window and reopen VS Code so it receives the updated environment.

## `Qt6Config.cmake` cannot be found

Install `mingw-w64-ucrt-x86_64-qt6-base` from the MSYS2 UCRT64 terminal. Configure CMake with:

```powershell
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_PREFIX_PATH=C:/msys64/ucrt64
```

## `QApplication` or other Qt headers cannot be found when pressing Run Code

Do not compile an individual `.cpp` file with Code Runner. This project contains multiple C++ source files and Qt headers. Build it with CMake from the folder containing `CMakeLists.txt`:

```powershell
cmake --build build --parallel
.\\build\\ExpenseTracker.exe
```

## CMake says a source file is missing

Check that the VS Code terminal is at the project root (the folder containing `CMakeLists.txt`), not inside `src` or `include`:

```powershell
Get-Location
```

Then run the configure command shown above.

## Old expenses do not appear

- Close the application before copying JSON files.
- Copy your previous `data/expenses.json` into the new project's `data` folder.
- Check that the JSON is valid and contains an `expenses` array.
- Do not delete or overwrite the only copy of your old data.
