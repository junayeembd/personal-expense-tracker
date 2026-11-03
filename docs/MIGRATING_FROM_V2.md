# Migrating from Version 2.x to Version 3.0

Version 3.0 preserves the existing expense and income JSON formats and adds separate monthly budget and recurring-rule files.

1. Close the old application.
2. Make a separate copy of the complete old `data` folder before updating.
3. Extract the new source files over the project folder, keeping the existing `.git` folder and your private `data/*.json` files.
4. Do not replace or delete `data/expenses.json` or `data/incomes.json`.
5. Version 3.0 creates `data/budgets.json` and `data/recurring.json` when needed. Existing installations without these files are treated as having no budgets or recurring rules.
6. Build and run the updated app. Existing expense and income transactions should load as before.
7. Use **Backup data** in the dashboard to create a complete V3 backup before experimenting with restore. New backups include expenses, income, budgets, and recurring rules.

The app still accepts legacy Version 1 expense records without IDs and generates IDs when loading them. If you keep your data in a different directory, copy only the JSON files after closing the app.

Never commit transaction JSON, backup JSON, or CSV exports to a public source repository.

## Updating an existing working folder from the clean V3 source archive

The downloadable source archive intentionally omits `.git/`, `build/`, and personal JSON records. This prevents stale binaries and private financial information from being distributed.

1. Close the application.
2. Copy the complete `data/` directory to a safe location.
3. Extract the V3 archive to a temporary folder first.
4. Copy the extracted project files over the existing project folder and allow source/documentation files to be replaced. Keep the existing `.git/` directory and every existing `data/*.json` file.
5. The new `data/.gitkeep` may be copied into the existing data folder; it does not replace JSON records.
6. Remove the old generated `build/` directory so CMake performs a clean V3 build.
7. Review the old `docs/screenshots/Screenshot (41).png` through `Screenshot (44).png` images. The new archive replaces these with fictional SVG previews; remove the old PNGs if they are no longer wanted in the repository.
8. Run `scripts/build.ps1`, then `scripts/run.ps1`.
9. Review `git status` before committing. The existing Git history and remote are preserved because the archive does not contain `.git/`.

The V3 app reads the existing `expenses.json` and `incomes.json` files. Missing `budgets.json` and `recurring.json` files are treated as empty stores and will be created when the corresponding data is first saved.
