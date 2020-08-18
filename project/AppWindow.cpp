#include <QtWidgets>
#include <QMenuBar>
#ifdef Q_OS_WIN
#include <windows.h>
#include <shlobj.h>
#include <combaseapi.h>
#endif
#include "AppWindow.h"
#include "Model.h"
#include "../setup/OpenFileVersion.txt"
#include "SourceListEditorDlg.h"
#include "DarkModeStyle.h"


bool gDarkMode = true;

// #todo-minor support shell context menu on items in view

AppWindow::AppWindow()
{
	QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());
	gDarkMode = settings.value("darkmode", true).toBool();
	if (gDarkMode)
	{
		// https://www.medo64.com/2020/08/dark-mode-for-qt-application/
		qApp->setPalette(::GetDarkPalette());
		qApp->setStyleSheet(::GetDarkStyleSheet());
	}

	InitTrayIcon();
	InitModel();
	InitWindowMenu();
	InitWindow();

	mShowHiddenContent = settings.value("showHiddenContent", false).toBool();
	SetSourceModel(LoadModel(this, mShowHiddenContent));
	mProxyView->setColumnWidth(0, settings.value("col1width", 400).toInt());
	mProxyView->setColumnWidth(1, settings.value("col2width", 500).toInt());

	OnToggleHotkey();
}

void
AppWindow::InitModel()
{
	mProxyModel = new QSortFilterProxyModel;
	mProxyModel->setSortCaseSensitivity(Qt::CaseSensitive);
}

void
AppWindow::InitWindow()
{
	mProxyView = new QTreeView;
	mProxyView->setRootIsDecorated(false);
	mProxyView->setAlternatingRowColors(false);
	mProxyView->setModel(mProxyModel);
	mProxyView->setSortingEnabled(true);
	mProxyView->setEditTriggers(QAbstractItemView::NoEditTriggers);
	mProxyView->sortByColumn(1, Qt::AscendingOrder);
	mProxyView->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
	mProxyView->setSelectionMode(QAbstractItemView::SelectionMode::ExtendedSelection); // MultiSelections doesn't work well with key forwarding from edit control

	InitFilterEdit();

	QVBoxLayout * mainLayout = new QVBoxLayout;
	mainLayout->setSpacing(0);
	// necessary to get menubar to be positioned at top like a normal menubar
	mainLayout->setContentsMargins(0, 0, 0, 0);
	mainLayout->addWidget(mMenuBar);
	mainLayout->addWidget(mFilterPatternLineEdit);
	mainLayout->addWidget(mProxyView);
	setLayout(mainLayout);

	setWindowTitle(tr("OpenFile++"));

	QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());
	const QByteArray geometry = settings.value("mainWndGeometry", QByteArray()).toByteArray();
	if (geometry.isEmpty()) 
		resize(1000, 450);
	else
		restoreGeometry(geometry);
}

void
AppWindow::InitFilterEdit()
{
	mFilterPatternLineEdit = new QLineEdit;
	mFilterPatternLineEdit->setPlaceholderText("enter filename search strings (include '" + QString(QDir::separator()) + "' character to search full path)");

	connect(mFilterPatternLineEdit, &QLineEdit::textChanged,
		this, &AppWindow::OnFilterRegExpChanged);

	// https://www.informit.com/articles/article.aspx?p=1405544&seqNum=2
	mFilterPatternLineEdit->installEventFilter(this);
}

bool
AppWindow::eventFilter(QObject *target, QEvent *evt)
{
	if (target == mFilterPatternLineEdit) 
	{
		if (evt->type() == QEvent::KeyPress) 
		{
			QKeyEvent *keyEvent = static_cast<QKeyEvent *>(evt);
			switch (keyEvent->key())
			{
			case Qt::Key_Up:
			case Qt::Key_Down:
			case Qt::Key_PageUp:
			case Qt::Key_PageDown:
				// forwarding/buddy system for up/down/pg-up/pg-down when focus is in the filter edit
				QApplication::sendEvent(mProxyView, evt);
				return true;
			}
		}
	}

	return QWidget::eventFilter(target, evt);
}

void
AppWindow::closeEvent(QCloseEvent *evt)
{
	QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());
	settings.setValue("col1width", mProxyView->columnWidth(0));
	settings.setValue("col2width", mProxyView->columnWidth(1));
	settings.setValue("mainWndGeometry", saveGeometry());

	evt->accept();
}

void
AppWindow::InitTrayIcon()
{
#ifndef QT_NO_SYSTEMTRAYICON
	if (!QSystemTrayIcon::isSystemTrayAvailable())
		return;

	mRestoreAction = new QAction(tr("&Show"), this);
	connect(mRestoreAction, &QAction::triggered, this, &QWidget::show);

	mQuitAction = new QAction(tr("&Quit"), this);
	connect(mQuitAction, &QAction::triggered, qApp, &QCoreApplication::quit);

	mTrayIconMenu = new QMenu(this);
	mTrayIconMenu->addAction(mRestoreAction);
	mTrayIconMenu->addSeparator();
	mTrayIconMenu->addAction(mQuitAction);

	mTrayIcon = new QSystemTrayIcon(this);
	mTrayIcon->setContextMenu(mTrayIconMenu);
//	connect(mTrayIcon, &QSystemTrayIcon::messageClicked, this, &AppWindow::OnMessageClicked);
	connect(mTrayIcon, &QSystemTrayIcon::activated, this, &AppWindow::OnIconActivated);

	QIcon icon(":/images/bad.png");
	mTrayIcon->setIcon(icon);
	mTrayIcon->setToolTip("OpenFile++");
	mTrayIcon->show();
	setWindowIcon(icon);
#endif
}

void
AppWindow::setVisible(bool visible)
{
	if (visible && gDarkMode)
	{
		// #todo-darkmode: deal with flash of white when made visible
	}

	QWidget::setVisible(visible);
}

bool
AppWindow::nativeEvent(const QByteArray &eventType, void *message, long *result)
{
	Q_UNUSED(eventType);
#ifdef Q_OS_WIN
	MSG* msg = static_cast<MSG*>(message);
	if (msg->message == WM_HOTKEY)
	{
		DisplayWindow(true);
		*result = S_OK;
		return true;
	}
#else 
	Q_UNUSED(message);
	Q_UNUSED(result);
#endif
	return false;
}

void
AppWindow::InitWindowMenu()
{
	mMenuBar = new QMenuBar;

	QMenu *fileMenu = mMenuBar->addMenu(tr("&File"));

	QAction *execAct = fileMenu->addAction(tr("&Launch item(s)"), this, &AppWindow::OnLaunchSelection);
	execAct->setShortcut(tr("Return"));

#ifdef Q_OS_WIN
	QAction *expAct = fileMenu->addAction(tr("Open in &Explorer"), this, &AppWindow::OnOpenInFileManager);
	expAct->setShortcut(tr("Ctrl+E"));
#endif

	fileMenu->addSeparator();
	QAction *closeAct = fileMenu->addAction(tr("&Hide window"), this, &QWidget::hide);
	closeAct->setShortcut(tr("Esc"));
	closeAct->setStatusTip(tr("Hide this window"));

	const QIcon exitIcon = QIcon::fromTheme("application-exit");
	QAction *exitAct = fileMenu->addAction(exitIcon, tr("E&xit"), qApp, &QCoreApplication::quit);
	exitAct->setShortcuts(QKeySequence::Quit);
	exitAct->setStatusTip(tr("Exit the application"));

	QMenu *editMenu = mMenuBar->addMenu(tr("&Edit"));

	QAction *addFileAct = editMenu->addAction(tr("&Add file"), this, &AppWindow::OnAddFile);
	addFileAct->setShortcut(tr("Ctrl+N"));
	QAction *addDirAct = editMenu->addAction(tr("&Add directory"), this, &AppWindow::OnAddDirectory);
	addDirAct->setShortcut(tr("Ctrl+D"));
	editMenu->addSeparator();

	const QIcon copyIcon = QIcon::fromTheme("edit-copy", QIcon(":/images/copy.png"));
	QAction *copyAct = editMenu->addAction(copyIcon, tr("&Copy"), this, &AppWindow::OnCopy);
	copyAct->setShortcuts(QKeySequence::Copy);
	copyAct->setStatusTip(tr("Copy the selection to the clipboard"));

	const QIcon removeIcon = QIcon::fromTheme("edit-delete");
	QAction *removeAct = editMenu->addAction(removeIcon, tr("Remove &item from list temporarily"), this, &AppWindow::OnRemove);
	removeAct->setShortcuts(QKeySequence::Delete);
	editMenu->addSeparator();

	const QIcon editIcon = QIcon::fromTheme("file-new", QIcon(":/images/new.png"));
	editMenu->addAction(editIcon, tr("&Edit source list..."), this, &AppWindow::OnEditSourceList);

	QAction *refreshAct = editMenu->addAction(tr("&Refresh"), this, &AppWindow::OnRefresh);
	refreshAct->setShortcut(tr("F5"));
	refreshAct->setStatusTip(tr("Refresh the list"));

	QMenu *optionsMenu = mMenuBar->addMenu(tr("&Settings"));

	mToggleClearFilterAction = optionsMenu->addAction(tr("&Clear filter and hide on launch"), this, &AppWindow::OnToggleClearFilter);
	mToggleClearFilterAction->setCheckable(true);
	mToggleClearFilterAction->setChecked(mClearFilterOnExec);

#ifdef Q_OS_WIN
	mToggleHotkeyAction = optionsMenu->addAction(tr("&Enable hotkey (Ctrl+Shift+O)"), this, &AppWindow::OnToggleHotkey);
	mToggleHotkeyAction->setCheckable(true);
	mToggleHotkeyAction->setChecked(mHotkeyEnabled);
#endif

	// #todo-minor issue#4 add command to edit file exclusions (*.obj;*.dll;) used by the model populater

	mToggleDarkmodeAction = optionsMenu->addAction(tr("&Dark mode"), this, &AppWindow::OnToggleDarkmode);
	mToggleDarkmodeAction->setCheckable(true);
	mToggleDarkmodeAction->setChecked(gDarkMode);

	mToggleHiddenAction = optionsMenu->addAction(tr("&Show hidden files"), this, &AppWindow::OnToggleHiddenContent);
	mToggleHiddenAction->setCheckable(true);
	mToggleHiddenAction->setChecked(mShowHiddenContent);

	QMenu *helpMenu = mMenuBar->addMenu(tr("&Help"));

	helpMenu->addAction(tr("&About"), this, &AppWindow::OnAbout);
	helpMenu->addAction(tr("About &Qt"), this, &QApplication::aboutQt);
}

void
AppWindow::SetSourceModel(QAbstractItemModel *model)
{
	mProxyModel->invalidate();
	mProxyModel->setSourceModel(model);
	mProxyView->setCurrentIndex(mProxyModel->index(0, 0));
}

void
AppWindow::OnLaunchSelection()
{
	QModelIndexList items(mProxyView->selectionModel()->selectedIndexes());
	for (const auto &it : items)
	{
		if (it.column() != 1)
			continue;

		auto cur = mProxyModel->data(it, Qt::DisplayRole).toString();
		if (0 == cur.indexOf("http"))
		{
			QDesktopServices::openUrl(QUrl(cur, QUrl::TolerantMode));
			continue;
		}

		if (QFile::exists(cur))
		{
			QDesktopServices::openUrl(QUrl("file:///" + cur, QUrl::TolerantMode));
			continue;
		}
	}

	if (mClearFilterOnExec)
	{
		hide();
		mFilterPatternLineEdit->setText("");
		mProxyView->setCurrentIndex(mProxyModel->index(0, 0));
	}
}

void
AppWindow::OnOpenInFileManager()
{
	QModelIndexList items(mProxyView->selectionModel()->selectedIndexes());
	for (const auto &it : items)
	{
		if (it.column() != 1)
			continue;

		auto cur = mProxyModel->data(it, Qt::DisplayRole).toString();
		if (0 == cur.indexOf("http"))
			continue;

		if (QFile::exists(cur))
		{
#ifdef Q_OS_WIN
			::CoInitializeEx(nullptr, COINIT_MULTITHREADED);
			ITEMIDLIST *pidl = ::ILCreateFromPathW((PCWSTR)cur.toStdU16String().c_str());
			if (pidl)
			{
				::SHOpenFolderAndSelectItems(pidl, 0, 0, 0);
				::ILFree(pidl);
			}
			::CoUninitialize();
#endif
			continue;
		}
	}
}

void
AppWindow::OnAbout()
{
	QString txt =
		"<html><body>OpenFile++  (" OpenFileAppVersion ")<br><br>"
		"<a href=\"https://github.com/sean-e/openfileplusplus/\">https://github.com/sean-e/openfileplusplus/</a><br><br>"
		"&copy; copyright 2020 Sean Echevarria<br><br>";
	txt += "Uses the open source <a href=\"https://www.qt.io/download-open-source\">Qt</a> framework<br><br>"
		"Licensed under GPL v3"
		"</body></html>";

	QMessageBox::about(this, tr("About OpenFile++"), txt);
}

void
AppWindow::OnRefresh()
{
	QGuiApplication::setOverrideCursor((QCursor(Qt::WaitCursor)));
	SetSourceModel(LoadModel(this, mShowHiddenContent));
	QGuiApplication::restoreOverrideCursor();
}

void
AppWindow::OnRemove()
{
	int lastRowHandled = -1;
	QModelIndexList items(mProxyView->selectionModel()->selectedIndexes());
	// handle in reverse order so that row numbers aren't offset by removals
	for (auto it = items.rbegin(); it != items.rend(); ++it)
	{
		const int curRow = (*it).row();
		if (curRow == lastRowHandled)
			continue; // index exists for each column of each row

		lastRowHandled = curRow;
		mProxyModel->removeRow(curRow);
	}
}

void
AppWindow::OnCopy()
{
	QClipboard *clipboard = QGuiApplication::clipboard();
	if (!clipboard)
		return;
		
	QString txt;
	QModelIndexList items(mProxyView->selectionModel()->selectedIndexes());
	constexpr int kColumns = 2;
	const bool addLinebreaks = (items.size() / kColumns) > 1; // don't append linebreak if only single item is copied
	for (const auto &it : items)
	{
		if (it.column() != 1)
			continue;

		auto cur = mProxyModel->data(it, Qt::DisplayRole).toString();
		txt += cur;

		if (addLinebreaks)
			txt += "\n";
	}

	clipboard->setText(txt);
}

void
AppWindow::OnToggleDarkmode()
{
	QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());
	gDarkMode = !gDarkMode;
	mToggleDarkmodeAction->setChecked(gDarkMode);
	settings.setValue("darkmode", gDarkMode);

	if (gDarkMode)
	{
		qApp->setPalette(::GetDarkPalette());
		qApp->setStyleSheet(::GetDarkStyleSheet());
	}
	else
	{
		qApp->setPalette(QStyleFactory::create("Fusion")->standardPalette());
		qApp->setStyleSheet("");
	}
}

void
AppWindow::OnToggleClearFilter()
{
	mClearFilterOnExec = !mClearFilterOnExec;
	mToggleClearFilterAction->setChecked(mClearFilterOnExec);
}

void
AppWindow::OnToggleHotkey()
{
	if (mHotkeyEnabled)
	{
#ifdef Q_OS_WIN
		::UnregisterHotKey(HWND(winId()), 0);
		mHotkeyEnabled = false;
#endif
	}
	else
	{
#ifdef Q_OS_WIN
		// #todo-minor add support for user-configurable hotkey
		if (::RegisterHotKey(HWND(winId()), 0, MOD_SHIFT | MOD_CONTROL, 'O'))
			mHotkeyEnabled = true;
		else
			QMessageBox::warning(this, "OpenFile++ Error", "Failed to register hotkey.");
#endif
	}

	mToggleHotkeyAction->setChecked(mHotkeyEnabled);
}

void
AppWindow::OnToggleHiddenContent()
{
	mShowHiddenContent = !mShowHiddenContent;
	QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());
	settings.setValue("showHiddenContent", mShowHiddenContent);
	mToggleHiddenAction->setChecked(mShowHiddenContent);
	OnRefresh();
}

void
AppWindow::OnFilterRegExpChanged()
{
	QString filterTextInput(mFilterPatternLineEdit->text());
	bool looksLikeRealRegExp = false;
	if (-1 == filterTextInput.indexOf("\\\\") &&
		-1 == filterTextInput.indexOf('+') &&
		-1 == filterTextInput.indexOf('*') &&
		-1 == filterTextInput.indexOf('?') &&
		-1 == filterTextInput.indexOf('=') &&
		-1 == filterTextInput.indexOf('^') &&
		-1 == filterTextInput.indexOf('!') &&
		-1 == filterTextInput.indexOf('[') &&
		-1 == filterTextInput.indexOf('{') &&
		-1 == filterTextInput.indexOf('('))
	{
		// doesn't look like reg exp
		if (QDir::separator() == '\\')
		{
			// escape use of '\' on windows
			filterTextInput.replace('\\', "\\\\");
		}

		filterTextInput.replace('.', "[.]");
	}
	else
		looksLikeRealRegExp = true;

	const bool searchFullPath = -1 != filterTextInput.indexOf(QDir::separator());
	QString filterText;
	int pos = looksLikeRealRegExp ? -1 : filterTextInput.indexOf(' ');
	if (-1 != pos)
	{
		// combine multiple reset lookahead matches
		// (?=.*substr)
		QString tmp;
		for (;;)
		{
			tmp = filterTextInput.mid(0, pos);
			filterText += "(?=.*" + tmp + ")";
			filterTextInput = filterTextInput.mid(pos + 1);
			pos = filterTextInput.indexOf(' ');
			if (-1 == pos)
			{
				filterText += "(?=.*" + filterTextInput + ")";
				break;
			}
		}

		// #todo-minor support negative filter using '-' character (but doesn't seem like ! or ^ work with QtRegularExpression)
	}
	else
	{
		// input is either simple single term, or looks like reg exp.
		// assume they know what they are doing.
		filterText = filterTextInput;
	}

	QRegularExpression regExp(filterText, QRegularExpression::PatternOption::CaseInsensitiveOption);
	mProxyModel->setFilterKeyColumn(searchFullPath ? 1 : 0);
	mProxyModel->setFilterRegularExpression(regExp);

	QModelIndexList items(mProxyView->selectionModel()->selectedIndexes());
	if (items.isEmpty())
	{
		// there should always be a default visible selection, so that user 
		// can just press enter after filtering
		QModelIndex firstItem(mProxyModel->index(0, 0));
		mProxyView->setCurrentIndex(firstItem);
		mProxyView->scrollTo(firstItem);
	}
	else
	{
		mProxyView->scrollTo(items.first());
	}
}

void
AppWindow::OnEditSourceList()
{
	SourceListEditorDlg ed(this);
	if (ed.exec() == QDialog::Accepted)
		OnRefresh();
}

void
AppWindow::OnAddFile()
{
	QString f(QFileDialog::getOpenFileName(this, tr("Select File")));
	if (f.isEmpty())
		return;

	f = QDir::toNativeSeparators(f);
	QString txt("[file]" + f);
	AppendToDatafile(txt);
	OnRefresh();
}

void
AppWindow::OnAddDirectory()
{
	QString f(QFileDialog::getExistingDirectory(this, tr("Select Directory")));
	if (f.isEmpty())
		return;

	f = QDir::toNativeSeparators(f);
	QString txt("[dir]" + f);
	AppendToDatafile(txt);
	OnRefresh();
}

void
AppWindow::OnIconActivated(QSystemTrayIcon::ActivationReason reason)
{
	switch (reason) 
	{
	case QSystemTrayIcon::Trigger:
	case QSystemTrayIcon::DoubleClick:
	case QSystemTrayIcon::MiddleClick:
		DisplayWindow(false);
		break;
	case QSystemTrayIcon::Context:
	default:
		break;
	}
}

void
AppWindow::DisplayWindow(bool useMousePosIfNotVisible)
{
	if (isHidden())
	{
		if (useMousePosIfNotVisible)
			move(QCursor::pos());

		show();
	}

	raise();
	activateWindow();
}
