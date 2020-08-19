#include "DarkModeStyle.h"

// these values are used in QPalettes, but note that they are also 
// duplicated as literals in style sheets
constexpr int gDarkModeTextColor = 0xffffff;
constexpr int gDarkModeBackgroundColor = 0x1a1a1a;

QPalette
GetDarkPalette()
{
	QPalette pal;
	pal.setColor(QPalette::Text, gDarkModeTextColor);
	pal.setColor(QPalette::ButtonText, gDarkModeTextColor);
	pal.setColor(QPalette::WindowText, gDarkModeTextColor);
	pal.setColor(QPalette::Light, gDarkModeTextColor);

	pal.setColor(QPalette::Base, gDarkModeBackgroundColor);
	pal.setColor(QPalette::AlternateBase, gDarkModeBackgroundColor);
	pal.setColor(QPalette::Background, gDarkModeBackgroundColor);
	pal.setColor(QPalette::Button, gDarkModeBackgroundColor);
	pal.setColor(QPalette::Window, gDarkModeBackgroundColor);
	pal.setColor(QPalette::Dark, gDarkModeBackgroundColor);
	pal.setColor(QPalette::Foreground, gDarkModeBackgroundColor);
	pal.setColor(QPalette::Midlight, gDarkModeBackgroundColor);
	pal.setColor(QPalette::Shadow, gDarkModeBackgroundColor);
	pal.setColor(QPalette::Mid, gDarkModeBackgroundColor);

	// pal.setColor(QPalette::Disabled, QPalette::Text, QColor(127, 127, 127));
	return pal;
}

QString
GetDarkStyleSheet()
{
	// #todo-darkmode: fix scrollbar pixel values
	// #todo-darkmode: QTreeView checkboxes
	// https://doc.qt.io/qt-5/stylesheet-reference.html
	// https://doc.qt.io/qt-5/stylesheet-examples.html#customizing-qlineedit
	// https://doc.qt.io/qt-5/stylesheet-customizing.html

	const QString darkStyle(" \
		QTreeView { \
			border: 1px solid black; \
			background: #1a1a1a; \
		} \
		QTreeView::item:selected:!active { \
			color: white; \
			background: #203030; \
		} \
\
		QHeaderView::section { \
			background-color: #1a1a1a; \
			color: white; \
		} \
\
		QLineEdit { \
			border: 1px solid black; \
			background: #1a1a1a; \
		} \
\
		QLabel { \
			color: white; \
			background: #1a1a1a; \
		} \
\
		QMenuBar { \
			border: 1px black; \
			background-color: #1a1a1a; \
			color: white; \
		} \
		QMenuBar::item { \
			spacing: 3px; /* spacing between menu bar items */ \
			padding: 1px 10px; \
			background-color: #1a1a1a; \
		} \
		QMenuBar::item:selected { /* when selected using mouse or keyboard */ \
			background: #505050; \
		} \
		QMenuBar::item:pressed { \
			background: #505050; \
		} \
\
		QMenu { \
			background-color: #1a1a1a; \
			color: white; \
		} \
		QMenu::item { \
			background: #1a1a1a; \
			color: white; \
		} \
		QMenu::item:selected { /* when selected using mouse or keyboard */ \
			background: #505050; \
			color: white; \
		} \
		QMenu::item:pressed { \
			background: #505050; \
		} \
\
		QPushButton { \
			border: 1px solid #505050; \
			background-color: #1a1a1a; \
			min-width: 100px; \
			min-height: 35px; \
		} \
\
		QMessageBox { \
			color: white; \
		} \
		QMessageBox QLabel { \
			color: white; \
		} \
\
		QScrollBar:horizontal { \
			border: 1px #1a1a1a; \
			background: #202020; \
			height: 10px; \
			margin: 0px 10px 0 10px; \
		} \
		QScrollBar::handle:horizontal { \
			background: #505050; \
			min-width: 10px; \
		} \
		QScrollBar::add-line:horizontal { \
			border: 1px #1a1a1a; \
			background: #3a3a3a; \
			width: 10px; \
			subcontrol-position: right; \
			subcontrol-origin: margin; \
		} \
		QScrollBar::sub-line:horizontal { \
			border: 1px #1a1a1a; \
			background: #3a3a3a; \
			width: 10px; \
			subcontrol-position: left; \
			subcontrol-origin: margin; \
		} \
		QScrollBar:left-arrow:horizontal, QScrollBar::right-arrow:horizontal { \
			border: 1px #1a1a1a; \
			width: 3px; \
			height: 3px; \
			background: #808080; \
		} \
		QScrollBar::add-page:horizontal, QScrollBar::sub-page:horizontal { \
			background: none; \
		} \
\
		QScrollBar:vertical { \
			border: 1px #1a1a1a; \
			background: #202020; \
			width: 10px; \
			margin: 10px 0 10px 0; \
		} \
		QScrollBar::handle:vertical { \
			background: #505050; \
			min-height: 10px; \
		} \
		QScrollBar::add-line:vertical { \
			border: 1px #1a1a1a; \
			background: #3a3a3a; \
			height: 10px; \
			subcontrol-position: bottom; \
			subcontrol-origin: margin; \
		} \
		QScrollBar::sub-line:vertical { \
			border: 1px #1a1a1a; \
			background: #3a3a3a; \
			height: 10px; \
			subcontrol-position: top; \
			subcontrol-origin: margin; \
		} \
		QScrollBar::up-arrow:vertical, QScrollBar::down-arrow:vertical { \
			border: 1px #1a1a1a; \
			width: 6px; \
			height: 3px; \
			background: #808080; \
		} \
		QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical { \
			background: none; \
		}"
	);

	return darkStyle;
}
