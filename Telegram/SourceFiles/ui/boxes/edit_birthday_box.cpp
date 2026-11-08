/*
This file is part of Telegram Desktop,
the official desktop application for the Telegram messaging service.

For license and copyright information please follow this link:
https://github.com/telegramdesktop/tdesktop/blob/master/LEGAL
*/
#include "ui/boxes/edit_birthday_box.h"

#include "base/event_filter.h"
#include "data/data_birthday.h"
#include "lang/lang_keys.h"
#include "ui/layers/generic_box.h"
#include "ui/widgets/vertical_drum_picker.h"
#include "ui/ui_utility.h"
#include "styles/style_layers.h"
#include "styles/style_settings.h"

#include <QtCore/QCalendar>
#include <QtCore/QDate>

namespace Ui {

class GenericBox;

void EditBirthdayBox(
		not_null<Ui::GenericBox*> box,
		Data::Birthday current,
		Fn<void(Data::Birthday)> save,
		EditBirthdayType type) {
	if (type != EditBirthdayType::Suggest) {
		box->setTitle(tr::lng_settings_birthday_title());
	}
	box->setWidth(st::boxWideWidth);
	const auto content = box->addRow(object_ptr<Ui::FixedHeightWidget>(
		box,
		st::settingsWorkingHoursPicker));

	const auto font = st::boxTextFont;
	const auto itemHeight = st::settingsWorkingHoursPickerItemHeight;
	const auto picker = [=](
			int count,
			int startIndex,
			Fn<void(QPainter &p, QRectF rect, int index)> paint) {
		return Ui::CreateChild<Ui::VerticalDrumPicker>(
			content,
			Ui::VerticalDrumPicker::DefaultPaintCallback(
				font,
				itemHeight,
				paint),
			count,
			itemHeight,
			startIndex);
	};

	const auto calendar = QCalendar(QCalendar::System::Jalali);
	const auto nowDate = QDate::currentDate();
	const auto unknownYearReference = 1399;
	const auto referenceDate = [&](int month, int day) {
		auto year = nowDate.year();
		auto result = QDate(year, month, day);
		while (!result.isValid()) {
			result = QDate(--year, month, day);
		}
		return result;
	};
	const auto currentDate = current.year()
		? QDate(current.year(), current.month(), current.day())
		: current
		? referenceDate(current.month(), current.day())
		: nowDate;
	const auto maxDate = current.year()
		? std::max(nowDate, currentDate)
		: nowDate;
	const auto maxParts = calendar.partsFromDate(maxDate);
	const auto minParts = calendar.partsFromDate(QDate(
		Data::Birthday::kYearMin,
		1,
		1));
	const auto currentParts = calendar.partsFromDate(currentDate);
	const auto maxYear = maxParts.year;
	const auto minYear = minParts.year;
	const auto yearsCount = (maxYear - minYear + 2); // Last - not set.
	const auto yearsStartIndex = current.year()
		? (currentParts.year - minYear)
		: (yearsCount - 1);
	const auto persianDigits = [](QString value) {
		for (auto digit = u'0'; digit <= u'9'; ++digit) {
			value.replace(QChar(digit), QChar(0x06F0 + digit - u'0'));
		}
		return value;
	};
	const auto monthName = [](int month) {
		switch (month) {
		case 1: return u"فروردین"_q;
		case 2: return u"اردیبهشت"_q;
		case 3: return u"خرداد"_q;
		case 4: return u"تیر"_q;
		case 5: return u"مرداد"_q;
		case 6: return u"شهریور"_q;
		case 7: return u"مهر"_q;
		case 8: return u"آبان"_q;
		case 9: return u"آذر"_q;
		case 10: return u"دی"_q;
		case 11: return u"بهمن"_q;
		case 12: return u"اسفند"_q;
		}
		return u"ماه نامعتبر"_q;
	};
	const auto yearsPaint = [=](QPainter &p, QRectF rect, int index) {
		p.drawText(
			rect,
			(index < yearsCount - 1
				? persianDigits(QString::number(minYear + index))
				: QString::fromUtf8("\xe2\x80\x94")),
			style::al_center);
	};
	const auto years = picker(yearsCount, yearsStartIndex, yearsPaint);

	struct State {
		rpl::variable<Ui::VerticalDrumPicker*> months;
		rpl::variable<Ui::VerticalDrumPicker*> days;
		int firstMonth = 1;
	};
	const auto state = content->lifetime().make_state<State>();

	// years->value() is valid only after size is set.
	rpl::combine(
		content->sizeValue(),
		state->months.value(),
		state->days.value()
	) | rpl::on_next([=](
			QSize s,
			Ui::VerticalDrumPicker *months,
			Ui::VerticalDrumPicker *days) {
		const auto half = s.width() / 2;
		years->setGeometry(half * 3 / 2, 0, half / 2, s.height());
		if (months) {
			months->setGeometry(half / 2, 0, half, s.height());
		}
		if (days) {
			days->setGeometry(0, 0, half / 2, s.height());
		}
	}, content->lifetime());

	Ui::SendPendingMoveResizeEvents(years);

	years->value() | rpl::on_next([=](int yearsIndex) {
		const auto year = (yearsIndex == yearsCount - 1)
			? 0
			: minYear + yearsIndex;
		const auto firstMonth = (year == minYear)
			? minParts.month
			: 1;
		const auto lastMonth = (year == maxYear)
			? maxParts.month
			: 12;
		const auto monthsCount = lastMonth - firstMonth + 1;
		const auto oldMonth = state->months.current()
			? state->firstMonth + state->months.current()->index()
			: currentParts.month;
		const auto monthsStartIndex = std::clamp(
			oldMonth - firstMonth,
			0,
			monthsCount - 1);
		state->firstMonth = firstMonth;
		const auto monthsPaint = [=](QPainter &p, QRectF rect, int index) {
			p.drawText(
				rect,
				monthName(firstMonth + index),
				style::al_center);
		};
		const auto updated = picker(
			monthsCount,
			monthsStartIndex,
			monthsPaint);
		delete state->months.current();
		state->months = updated;
		state->months.current()->show();
	}, years->lifetime());

	Ui::SendPendingMoveResizeEvents(state->months.current());

	state->months.value() | rpl::map([=](Ui::VerticalDrumPicker *picker) {
		return picker ? picker->value() : rpl::single(currentParts.month
			- state->firstMonth);
	}) | rpl::flatten_latest() | rpl::on_next([=](int monthIndex) {
		const auto month = state->firstMonth + monthIndex;
		const auto yearsIndex = years->index();
		const auto year = (yearsIndex == yearsCount - 1)
			? 0
			: minYear + yearsIndex;
		const auto calendarYear = year ? year : unknownYearReference;
		auto daysCount = calendar.daysInMonth(month, calendarYear);
		if (year == maxYear && month == maxParts.month) {
			daysCount = std::min(daysCount, maxParts.day);
		}
		const auto firstDay = (year == minYear && month == minParts.month)
			? minParts.day
			: 1;
		const auto oldDay = state->days.current()
			? state->days.current()->index() + 1
			: currentParts.day;
		const auto daysStartIndex = std::clamp(
			oldDay - 1,
			firstDay - 1,
			daysCount - 1);
		const auto daysPaint = [=](QPainter &p, QRectF rect, int index) {
			p.drawText(
				rect,
				persianDigits(QString::number(index + 1)),
				style::al_center);
		};
		const auto updated = picker(
			daysCount,
			daysStartIndex,
			daysPaint);
		delete state->days.current();
		state->days = updated;
		state->days.current()->show();
	}, years->lifetime());

	Ui::SendPendingMoveResizeEvents(state->months.current());

	content->paintRequest(
	) | rpl::on_next([=](const QRect &r) {
		auto p = QPainter(content);

		p.fillRect(r, Qt::transparent);

		const auto lineRect = QRect(
			0,
			content->height() / 2,
			content->width(),
			st::defaultInputField.borderActive);
		p.fillRect(lineRect.translated(0, itemHeight / 2), st::activeLineFg);
		p.fillRect(lineRect.translated(0, -itemHeight / 2), st::activeLineFg);
	}, content->lifetime());

	base::install_event_filter(box, [=](not_null<QEvent*> e) {
		if (e->type() == QEvent::KeyPress) {
			years->handleKeyEvent(static_cast<QKeyEvent*>(e.get()));
		}
		return base::EventFilterResult::Continue;
	});

	auto confirmText = (type == EditBirthdayType::Suggest)
		? tr::lng_suggest_birthday_box_confirm()
		: tr::lng_settings_save();
	box->addButton(std::move(confirmText), [=] {
		const auto year = (years->index() == yearsCount - 1)
			? 0
			: minYear + years->index();
		const auto month = state->firstMonth
			+ state->months.current()->index();
		const auto date = calendar.dateFromParts(
			year ? year : unknownYearReference,
			month,
			state->days.current()->index() + 1);
		const auto result = Data::Birthday(
			date.day(),
			date.month(),
			year ? date.year() : 0);
		box->closeBox();
		save(result);
	});
	box->addButton(tr::lng_cancel(), [=] {
		box->closeBox();
	});
	if (current && type == EditBirthdayType::Edit) {
		box->addLeftButton(tr::lng_settings_birthday_reset(), [=] {
			box->closeBox();
			save(Data::Birthday());
		});
	}
}

} // namespace Ui
