# Troubleshooting

## `cmake` or `ninja` is not recognized

- Confirm `mingw-w64-ucrt-x86_64-cmake` and `mingw-w64-ucrt-x86_64-ninja` are installed in MSYS2 UCRT64.
- Confirm `C:\msys64\ucrt64\bin` is in the Windows user `Path`.
- Close every VS Code window and reopen VS Code after changing PATH.

## `Qt6Config.cmake` cannot be found

Install `mingw-w64-ucrt-x86_64-qt6-base` in MSYS2 UCRT64. Configure CMake with:

```powershell
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_PREFIX_PATH=C:/msys64/ucrt64
```

## `QApplication` or other Qt headers cannot be found when pressing Run Code

Do not compile an individual `.cpp` file with Code Runner. Build the complete multi-file Qt project with CMake from the folder containing `CMakeLists.txt`:

```powershell
cmake --build build --parallel
.\build\ExpenseTracker.exe
```

## CMake says a source file is missing

The VS Code terminal must be at the project root, not inside `src` or `include`:

```powershell
Get-Location
```

## PowerShell blocks helper scripts

Allow scripts only for the current terminal session:

```powershell
Set-ExecutionPolicy -Scope Process -ExecutionPolicy Bypass
.\scripts\build.ps1
```

## Existing transactions do not appear

- Close the application before copying JSON files.
- Confirm that `data/expenses.json` and `data/incomes.json` contain valid JSON objects with the expected arrays.
- Keep a copy of the original files before making any changes.
- A malformed file is not silently overwritten; read the full error message and repair or restore the file first.

## Budget page shows no budgets

- Select the correct month in the budget page.
- A budget is specific to one month and one category.
- Transactions only contribute when their date and category match that budget.
- The budget is a tracking limit; it does not prevent spending.

## Restore is unavailable

Create a copy of your `data` folder first. Restore requires the current data files to load so the app can create a complete safety backup before replacement. If a JSON file is malformed, preserve it and repair or copy it aside before restoring.

## Test failures

Run:

```powershell
ctest --test-dir build --output-on-failure
```

Check the first failing test and its error output. Reconfigure after changing CMake files:

```powershell
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_PREFIX_PATH=C:/msys64/ucrt64
```

## A recurring rule did not generate a transaction

- Confirm the rule is marked **Active** and its next due date is today or earlier.
- Confirm expense, income, and recurring JSON stores loaded successfully.
- Open the Recurring page and choose **Process due now**.
- A future due date is not generated early. Weekly, monthly, and yearly rules advance from the scheduled due date.
- The app uses deterministic IDs for generated occurrences; processing the same due date again will not create a duplicate.
- If more than 240 occurrences are overdue for one rule, processing is capped per run. Press **Process due now** again to continue catching up.
