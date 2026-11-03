# Personal Expense Tracker

A Windows desktop personal-finance application written in **C++17** with **Qt 6 Widgets**. It stores records locally as JSON files; no account, cloud service, or database is required.

## Features

### Dashboard
- Total income, total expenses, current balance, and this month's expenses
- Six-month income-versus-expense bar chart
- Current-month expense category breakdown chart

### Transactions
- Add, edit, and delete income and expense records
- Built-in and custom categories
- Search by category or description
- Filter by transaction type, category, and date range
- Export all transactions to CSV (UTF-8 with BOM for spreadsheet compatibility)
- Confirmation before permanent deletion

### Local data
- Expenses: `data/expenses.json`
- Income: `data/incomes.json`
- Atomic file writes via Qt's `QSaveFile`
- Existing Version 1 expense JSON records are still readable; records without IDs receive an ID when loaded.
- Personal JSON files are excluded from Git by `.gitignore`.

## Technology

- C++17
- Qt 6 Widgets
- CMake
- Ninja
- JSON (Qt JSON classes)
- Git and GitHub

Charts are drawn with `QPainter`, so the app does **not** need the optional Qt Charts module.

## Windows 11 setup (MSYS2 UCRT64)

Install MSYS2 to `C:\msys64`, open **MSYS2 UCRT64**, then install the build tools and Qt:

```bash
pacman -Syu
```

If the terminal asks to close, reopen **MSYS2 UCRT64**, then run:

```bash
pacman -S --needed mingw-w64-ucrt-x86_64-toolchain mingw-w64-ucrt-x86_64-cmake mingw-w64-ucrt-x86_64-ninja mingw-w64-ucrt-x86_64-qt6-base
```

In Windows user `Path`, add:

```text
C:\msys64\ucrt64\bin
```

Install these VS Code extensions:
- **C/C++** (Microsoft)
- **CMake Tools** (Microsoft)

## Build and run from VS Code PowerShell

Open the project **root folder** (the folder containing `CMakeLists.txt`) in VS Code. Do not run individual `.cpp` files with Code Runner; this is a multi-file Qt/CMake application.

```powershell
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH=C:/msys64/ucrt64
cmake --build build --parallel
.\build\ExpenseTracker.exe
```

For a Debug build, replace `Release` with `Debug`.

You can also use the included PowerShell helpers from the project root:

```powershell
.\scripts\build.ps1
.\scripts\run.ps1
```

## Project structure

```text
Personal-Expense-Tracker/
├── .github/workflows/windows-build.yml
├── .vscode/settings.json
├── data/.gitkeep
├── include/
│   ├── Expense.h
│   ├── ExpenseStorage.h
│   ├── Income.h
│   ├── IncomeStorage.h
│   └── MainWindow.h
├── src/
│   ├── Expense.cpp
│   ├── ExpenseStorage.cpp
│   ├── Income.cpp
│   ├── IncomeStorage.cpp
│   ├── MainWindow.cpp
│   └── main.cpp
├── tests/MANUAL_TEST_PLAN.md
├── CMakeLists.txt
├── LICENSE
└── README.md
```

## GitHub upload

Create an empty GitHub repository, for example `cpp-personal-expense-tracker`. From the project root in PowerShell:

```powershell
git init
git add .
git status
git commit -m "Release Personal Finance Tracker 2.1"
git branch -M main
git remote add origin https://github.com/YOUR_USERNAME/cpp-personal-expense-tracker.git
git push -u origin main
```

Replace `YOUR_USERNAME` with your GitHub username. Check `git status` before committing: `data/expenses.json`, `data/incomes.json`, and `build/` must not be listed. Never publish personal financial records.

## Notes

- The program stores data in the project's `data` folder when run from the normal `build` directory. If that folder cannot be created, it falls back to the operating system's application-data directory.
- Back up your `data/*.json` files before manually editing them.
- The GitHub Actions workflow builds the project on Windows using MSYS2 UCRT64.
- See `docs/MIGRATING_FROM_V2.md` before moving your existing expense JSON into this version.
- See `docs/TROUBLESHOOTING.md` for common CMake, Qt, and VS Code issues.

## License

MIT. See [LICENSE](LICENSE).
