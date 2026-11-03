# GitHub release checklist

1. Build and run the app locally in Windows 11 using `scripts/build.ps1` and `scripts/run.ps1`.
2. Complete the manual test plan in `tests/MANUAL_TEST_PLAN.md` with disposable records.
3. Check that `data/*.json`, `data/backups/`, CSV exports, and `build/` are not staged by Git.
4. Create an empty public repository on GitHub.
5. From the project root, run:

   ```powershell
   git init
   git add .
   git status
   git commit -m "Release Personal Finance Tracker 3.0"
   git branch -M main
   git remote add origin https://github.com/YOUR_USERNAME/cpp-personal-expense-tracker.git
   git push -u origin main
   ```

6. Open the repository's Actions tab and confirm the Windows UCRT64 workflow passes.
7. Add application screenshots to the repository README after capturing them on Windows.
8. Create a GitHub Release tagged `v3.0.0` after the build and manual tests pass.

Do not commit real income/expense records, passwords, tokens, or other private data.
