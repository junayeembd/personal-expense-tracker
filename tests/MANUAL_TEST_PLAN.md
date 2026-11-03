# Manual GUI Test Plan — Version 3.0

Use clearly fictional test records. Do not use private transactions when capturing public screenshots.

## Startup and persistence

1. Launch with no JSON files; confirm the dashboard opens with zero totals and no crash.
2. Add one expense and one income; close and reopen; confirm both records remain.
3. Confirm older Version 1/2 expense records still load (records without IDs receive IDs).
4. Inspect each JSON file and confirm the expected `expenses`, `incomes`, `budgets`, or `rules` top-level array is present.

## Transactions

5. Add, edit, and delete an expense.
6. Add, edit, and delete an income.
7. Test invalid amount, empty category, and cancel actions.
8. Search by category and description.
9. Filter by type, category, and date range.
10. Try a start date later than the end date; confirm the invalid range message is displayed.
11. Sort newest, oldest, highest amount, and lowest amount.
12. Double-click a row and confirm the edit dialog opens.
13. Export CSV, open it in a spreadsheet, and inspect non-ASCII text and formula-like descriptions.

## Budgets

14. Create a Food budget for the current month.
15. Add Food expenses; verify spent, remaining, and usage update.
16. Verify 80% or greater shows Near limit and more than 100% shows Over budget.
17. Try to create a duplicate category/month budget; it should be rejected.
18. Edit and delete a budget; existing transactions must remain unchanged.
19. Change the month filter; only that month's budgets should appear.

## Recurring transactions

20. Create a weekly expense rule with a due date in the past; confirm one transaction is created for each due date through today.
21. Create a monthly income rule with a future due date; confirm it is not generated early.
22. Press **Process due now** twice and restart the app; confirm the same scheduled occurrence is not duplicated.
23. Pause a rule and confirm it is skipped; resume and process due records.
24. Verify a monthly rule anchored to the 30th returns to the 30th after February; verify a month-end rule remains on each month's last day.
25. Edit a recurring rule and confirm changes persist after restart.
26. Delete a rule and confirm already-generated transactions remain in the transaction history.

## Backup and restore

27. Create a backup and verify it contains `expenses`, `incomes`, `budgets`, and `recurringRules` arrays.
28. Make a test change, restore the backup, and verify all four data groups return.
29. Confirm a pre-restore safety copy is created under `data/backups/`.
30. Try to restore invalid JSON; the app must reject it without changing data.
31. Restore a schema version 1 backup without recurring rules; the app should accept it and restore an empty recurring-rule list.

## Privacy and release

32. Confirm `data/*.json`, `data/backups/`, `build/`, and CSV exports are not included in source packages.
33. Capture only fictional sample data for public screenshots.
