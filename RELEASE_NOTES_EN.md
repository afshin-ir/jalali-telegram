# Telegram Desktop 7.4.6 with Jalali Calendar (Linux)

This unofficial Linux build of Telegram Desktop 7.4.6 displays dates using the Persian (Jalali/Solar Hijri) calendar. Dates are formatted in day–month–year order.

## Jalali implementation

The fork keeps Telegram Desktop's existing custom calendar UI and adapts it for Jalali dates. It uses Qt 6's `QCalendar::System::Jalali` to derive Jalali date components from `QDate` values, with Persian month names and Persian digits. The underlying timestamps and stored dates are unchanged; conversion is applied for display and calendar selection.

Jalali dates are implemented across message timestamps and date labels, chat lists, message tooltips and edited or forwarded dates, scheduled-date selection, birthdays and profile details, last-seen dates, active sessions, and other date displays. The calendar and month/year pickers also display Jalali dates.

## Upstream changes from 7.4.1 to 7.4.6

- **7.4.1–7.4.4:** Bug fixes and minor improvements, including fixes to the date filter in chat exports, send options in chat previews, selecting multiple proxies and contacts, and several crashes and freezes.
- **7.4.5:** Story posting, an update to Qt 6.12 and other dependencies, and bug fixes.
- **7.4.6:** On-chain gift offers and sales from the Money section; memory-safe, process-isolated HTML widgets; Stories drafting and posting; and bot story previews.

## Upstream release notes

- [Telegram Desktop 7.4.5 release notes](https://github.com/telegramdesktop/tdesktop/releases/tag/v7.4.5)
- [Telegram Desktop 7.4.6 release notes](https://github.com/telegramdesktop/tdesktop/releases/tag/v7.4.6)
- [Compare upstream 7.4.4 and 7.4.6](https://github.com/telegramdesktop/tdesktop/compare/v7.4.4...v7.4.6)
