# User Guide — Personal Finance Tracker 3.0

This guide explains the main screens and the effect of each action. The application is local-first: it writes data to JSON files on your device.

## Dashboard

The dashboard summarizes all-time income, all-time expenses, current balance (income minus expenses), and expenses dated in the current calendar month. The six-month chart groups income and expenses by calendar month. The category chart shows current-month spending by category. The budget health bar compares expenses only with budgets defined for the current month.

## Transactions

Use **Add income** or **Add expense** to create a transaction. Enter a positive amount, date, category/source, and optional description. The category control accepts a custom value as well as an existing category.

Select a row and choose **Edit selected** or **Delete selected**. Double-clicking a row opens the edit dialog. Deletion requires confirmation and cannot be undone unless you restore a backup.

The search field matches category/source and description. Filters can be combined: transaction type, category, and an inclusive start/end date range. The sort control orders by newest, oldest, highest amount, or lowest amount. **Clear filters** resets the search and filter controls.

Choose **Export CSV** to save the complete income and expense transaction list. CSV exports can contain private financial information; store them carefully.

## Monthly budgets

A budget applies to one category and one calendar month. Create a budget by choosing the month, category, and positive limit. A category/month pair can have only one budget.

- **On track:** less than 80% of the limit has been used.
- **Near limit:** 80% through 100% has been used.
- **Over budget:** spending is above the limit.

Budgets are warnings and reports; they do not block a transaction from being entered. Editing or deleting a budget does not change transactions.

## Recurring transactions

Create a rule for either income or expense. Choose the category/source, amount, frequency (weekly, monthly, yearly), first due date, and optional description. Rules can be edited, paused/resumed, or deleted. Deleting a rule does not delete transactions it previously generated.

Active rules are checked when the application starts and when **Process due now** is clicked. Due occurrences dated today or earlier are created; future occurrences are left alone. A deterministic ID is generated from the rule ID and scheduled date, preventing duplicate records if processing is retried.

Monthly schedules keep the original day-of-month anchor. For example, a rule on the 30th moves to February 28 in a non-leap year and then returns to March 30. A rule anchored to the last day of a month remains on the last day of each month. If a rule has more than 240 overdue occurrences, processing handles up to 240 for that rule in one run; click **Process due now** again to continue catching up.

**Important:** recurring rules create actual income/expense records. Review the amount, category, and first due date carefully before saving a rule.

## Backup and restore

Use **Backup data** on the dashboard to save a JSON backup containing expenses, income, budgets, and recurring rules. Keep the backup in a private location.

Use **Restore backup** only when you intend to replace the app's current records. The app validates the selected file, asks for confirmation, and writes a pre-restore safety copy under `data/backups/` before replacement. Do not close the app during a restore.

## Local data files

- `data/expenses.json` — expense transactions
- `data/incomes.json` — income transactions
- `data/budgets.json` — monthly category budgets
- `data/recurring.json` — recurring rules
- `data/backups/` — automatic safety copies created before restore

Keep these files private. The project `.gitignore` excludes JSON data, backup files, build output, and CSV exports. Back up the complete `data/` folder before replacing or migrating the project.
