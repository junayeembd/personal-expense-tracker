<div align="center">

# Personal Finance Tracker

<p>
  <img src="https://readme-typing-svg.demolab.com?font=Fira+Code&weight=600&size=20&pause=1100&color=2563EB&center=true&vCenter=true&width=760&lines=Track+your+money%2C+not+your+memory.;Understand+your+income+and+expenses.;Built+with+C%2B%2B17+%E2%80%A2+Qt+6+%E2%80%A2+JSON" alt="Animated project tagline" />
</p>

**A clean, offline-first desktop app for tracking income, expenses, and your financial trends.**

<br />

![C++17](https://img.shields.io/badge/C%2B%2B-17-00599C?logo=cplusplus&logoColor=white)
![Qt 6](https://img.shields.io/badge/Qt-6-41CD52?logo=qt&logoColor=white)
![CMake](https://img.shields.io/badge/CMake-Build-064F8C?logo=cmake&logoColor=white)
![Platform](https://img.shields.io/badge/Platform-Windows%2011-0078D6?logo=windows&logoColor=white)
![Storage](https://img.shields.io/badge/Storage-Local%20JSON-orange?logo=json&logoColor=white)
![License](https://img.shields.io/badge/License-MIT-green.svg)

[Features](#-features) · [Screenshots](#-screenshots) · [Install](#-installation) · [Build](#-build-and-run) · [Data & Privacy](#-data-and-privacy) · [Contributing](#-contributing)

</div>

---

## ✨ Overview

**Personal Finance Tracker** is a Windows desktop application built with **C++17** and **Qt 6 Widgets**. It brings income and expense records together in one place, calculates your balance, and visualizes recent financial activity.

The app is designed to work locally. Your records are stored as JSON files on your computer—no account, cloud service, server, or database is required.

> **Current release:** `v2.1.0`  
> **Currency shown in the interface:** BDT (৳)

## 🚀 Features

### 📊 Financial dashboard

- View **total income**, **total expenses**, **current balance**, and **this month's expenses**.
- Compare income and expenses across the **last six months** with a bar chart.
- See a **current-month expense breakdown by category** with a donut chart.
- Refresh dashboard figures automatically when transaction records change.

### 💳 Transaction management

- Add, edit, and delete both **income** and **expense** records.
- Record an amount, date, category/source, and optional description.
- Use built-in categories or type a **custom category**.
- Browse income and expenses together in one transaction list.
- Search by category or description.
- Filter by transaction type, category, and an inclusive date range.
- Confirm before permanently deleting a record.
- Export transactions to **CSV**, encoded as UTF-8 with a BOM for improved compatibility with spreadsheet applications.

### 💾 Local JSON storage

- Expense records: `data/expenses.json`
- Income records: `data/incomes.json`
- Save operations use Qt's `QSaveFile` for safer, atomic file replacement.
- Existing Version 1 expense records remain compatible. If an older record has no ID, the app generates one when it is loaded.
- Personal JSON data is excluded from version control through `.gitignore`.
- If the project's `data` directory cannot be created, the app falls back to the operating system's application-data location.

### 🔒 Privacy-first by design

- Your financial records stay on your device unless you deliberately export or share them.
- No sign-up, telemetry service, cloud sync, or online account is needed by the application.
- CSV exports may contain private information: store and share them carefully.

## 🖼️ Screenshots

Add screenshots of your own app build to `docs/screenshots/` and use the image links below. Use sample transactions rather than real personal financial records before publishing screenshots publicly.

<!--
Create docs/screenshots/ and add these files when ready:

<p align="center">
  <img src="docs/screenshots/dashboard.png" alt="Personal Finance Tracker dashboard" width="900" />
</p>
<p align="center">
  <img src="docs/screenshots/transactions.png" alt="Transactions page" width="900" />
</p>
-->

## 🧰 Technology stack

| Technology | Purpose |
|---|---|
| C++17 | Application logic and data models |
| Qt 6 Widgets | Windows desktop interface |
| Qt JSON classes | Read and write local JSON records |
| Qt `QPainter` | Draw dashboard charts |
| Qt `QSaveFile` | Safer atomic JSON writes |
| CMake | Configure the multi-file C++ project |
| Ninja | Fast build generation |
| MSYS2 UCRT64 / MinGW-w64 | Windows compiler and toolchain |

**Chart dependency note:** charts are painted with `QPainter`; the optional Qt Charts module is not required.

## 🖥️ Installation

These instructions are for **Windows 11** using the **MSYS2 UCRT64** toolchain.

### 1. Install MSYS2

1. Download MSYS2 from the [official website](https://www.msys2.org/).
2. Install it to the default location: `C:\msys64`.
3. Open **MSYS2 UCRT64** from the Start menu. Make sure you open UCRT64, not the plain MSYS terminal.

### 2. Update MSYS2

In the **MSYS2 UCRT64** terminal, run:

```bash
pacman -Syu
```

If MSYS2 asks you to close the terminal, close it, open **MSYS2 UCRT64** again, and complete any remaining update steps.

### 3. Install the compiler and Qt build tools

Run this command in **MSYS2 UCRT64**:

```bash
pacman -S --needed \
  mingw-w64-ucrt-x86_64-toolchain \
  mingw-w64-ucrt-x86_64-cmake \
  mingw-w64-ucrt-x86_64-ninja \
  mingw-w64-ucrt-x86_64-qt6-base
```

When prompted, accept the default package selection and confirm the installation.

### 4. Add UCRT64 tools to Windows PATH

Add this directory to your **Windows user Path**:

```text
C:\msys64\ucrt64\bin
```

To edit it: Windows Search → **Edit environment variables for your account** → select **Path** under User variables → **Edit** → **New** → paste the path → confirm with **OK**.

Close and reopen VS Code after changing PATH.

### 5. Verify the installation

Open a new VS Code PowerShell terminal and run:

```powershell
gcc --version
g++ --version
cmake --version
ninja --version
qmake6 --version
```

Each command should print its version. If a command is not recognized, confirm the packages are installed in UCRT64 and that the PATH entry is correct.

### 6. Install VS Code extensions

Install these extensions from the VS Code Marketplace:

- **C/C++** — Microsoft
- **CMake Tools** — Microsoft

Open the project root folder (the folder containing `CMakeLists.txt`) in VS Code. Do not use **Run Code** from Code Runner on an individual `.cpp` file; this is a multi-file Qt/CMake application and must be built as a project.

## ▶️ Build and run

### Option A: Use the included PowerShell scripts (recommended)

Open VS Code's **PowerShell** terminal at the project root and run:

```powershell
.\scripts\build.ps1
.\scripts\run.ps1
```

The build script configures a Release build and compiles the application. The run script starts `build/ExpenseTracker.exe`; if the executable is missing, it invokes the build script first.

If PowerShell blocks local scripts because of the current execution policy, allow scripts only for the current terminal session, then retry:

```powershell
Set-ExecutionPolicy -Scope Process -ExecutionPolicy Bypass
.\scripts\build.ps1
.\scripts\run.ps1
```

### Option B: Build manually with CMake

Make sure the terminal is in the **project root** (not inside `src/` or `include/`). Then run:

```powershell
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH=C:/msys64/ucrt64
cmake --build build --parallel
.\build\ExpenseTracker.exe
```

For a Debug build, replace `Release` with `Debug`:

```powershell
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_PREFIX_PATH=C:/msys64/ucrt64
cmake --build build --parallel
.\build\ExpenseTracker.exe
```

### Build output

After a successful build, the executable is created here:

```text
build/ExpenseTracker.exe
```

## 🗂️ Project structure

```text
Personal-Expense-Tracker/
├── .vscode/
│   └── settings.json
├── data/
│   └── .gitkeep
├── docs/
│   ├── ARCHITECTURE.md
│   ├── MIGRATING_FROM_V2.md
│   └── TROUBLESHOOTING.md
├── include/
│   ├── Expense.h
│   ├── ExpenseStorage.h
│   ├── Income.h
│   ├── IncomeStorage.h
│   └── MainWindow.h
├── scripts/
│   ├── build.ps1
│   └── run.ps1
├── src/
│   ├── Expense.cpp
│   ├── ExpenseStorage.cpp
│   ├── Income.cpp
│   ├── IncomeStorage.cpp
│   ├── MainWindow.cpp
│   └── main.cpp
├── tests/
│   └── MANUAL_TEST_PLAN.md
├── .gitattributes
├── .gitignore
├── CHANGELOG.md
├── CMakeLists.txt
├── LICENSE
└── README.md
```

## 🧪 Test the application

Use disposable test records while testing. Back up your real JSON files before copying, editing, or migrating data.

Recommended checks:

1. Launch with no JSON files and confirm the dashboard opens with zero totals.
2. Add an expense and an income; confirm both appear in Transactions.
3. Confirm balance = total income − total expenses.
4. Edit a record and verify the changes remain after restarting the app.
5. Delete a record; test both **Yes** and **No** in the confirmation dialog.
6. Search by category and description.
7. Filter by type, category, and date range.
8. Export CSV and open it in a spreadsheet application.
9. Copy a Version 1 `expenses.json` file and confirm the legacy records load.
10. Add a custom category and confirm it can be saved and filtered.

See [`tests/MANUAL_TEST_PLAN.md`](tests/MANUAL_TEST_PLAN.md) for the full test checklist.

## 🛠️ Troubleshooting

| Problem | What to check |
|---|---|
| `cmake` or `ninja` is not recognized | Recheck installation and `C:\msys64\ucrt64\bin` in Windows user PATH; reopen VS Code. |
| `Qt6Config.cmake` cannot be found | Install `mingw-w64-ucrt-x86_64-qt6-base` in MSYS2 UCRT64 and configure with `-DCMAKE_PREFIX_PATH=C:/msys64/ucrt64`. |
| `QApplication` or Qt headers are missing | Do not use Code Runner on one `.cpp` file; configure and build the complete CMake project. |
| CMake says a source file is missing | Run `Get-Location` and confirm the terminal is at the directory containing `CMakeLists.txt`. |
| PowerShell says scripts are disabled | Use `Set-ExecutionPolicy -Scope Process -ExecutionPolicy Bypass` for the current terminal session. |
| Old expenses are missing | Close the app and copy the old `data/expenses.json` into the new project's `data` directory; keep a backup first. |

For more help, see [`docs/TROUBLESHOOTING.md`](docs/TROUBLESHOOTING.md) and [`docs/MIGRATING_FROM_V2.md`](docs/MIGRATING_FROM_V2.md).

## 🧱 Architecture

The application keeps its main responsibilities separate:

```text
MainWindow (Qt Widgets interface)
   ├── Expense model ── ExpenseStorage ── data/expenses.json
   ├── Income model  ── IncomeStorage  ── data/incomes.json
   └── Dashboard charts (QPainter)
```

See [`docs/ARCHITECTURE.md`](docs/ARCHITECTURE.md) for more detail.

## 🗺️ Roadmap

- [x] Expense tracking and JSON persistence
- [x] Income tracking and separate JSON persistence
- [x] Financial summary dashboard
- [x] Six-month income/expense chart
- [x] Category breakdown chart
- [x] Search, filters, and CSV export
- [ ] Automated unit and integration tests
- [ ] Optional data backup and restore interface
- [ ] Optional installer/package for easier Windows distribution

## 🤝 Contributing

Suggestions and bug reports are welcome. Before sharing a change:

1. Build the project with the instructions above.
2. Run the manual checks in `tests/MANUAL_TEST_PLAN.md`.
3. Keep personal JSON data, CSV exports, and build output out of commits.
4. Update the README or changelog when user-visible behavior changes.

## 📄 License

This project is distributed under the **MIT License**. See [`LICENSE`](LICENSE).

---

<div align="center">

**Built with C++17 and Qt 6.**  
Made for clear, private, offline-first personal finance tracking.

</div>
