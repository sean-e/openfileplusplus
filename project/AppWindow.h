#ifndef WINDOW_H
#define WINDOW_H

#include <QWidget>
#include <QSystemTrayIcon>

QT_BEGIN_NAMESPACE
class QAbstractItemModel;
class QLineEdit;
class QSortFilterProxyModel;
class QTreeView;
class QMenuBar;
class QAction;
class QMenu;
QT_END_NAMESPACE

class AppWindow : public QWidget
{
	Q_OBJECT

public:
	AppWindow();

	void SetSourceModel(QAbstractItemModel *model);

private slots:
	void OnLaunchSelection();
	void OnOpenInFileManager();
	void OnAbout();
	void OnRefresh();
    void OnRestart();
	void OnRemove();
	void OnCopy();
	void OnToggleDarkmode();
	void OnToggleClearFilter();
	void OnToggleHotkey();
	void OnToggleHiddenContent();
	void OnFilterRegExpChanged();
	void OnEditSourceList();
	void OnAddFile();
	void OnAddDirectory();
	void OnEditExclusions();
	void OnIconActivated(QSystemTrayIcon::ActivationReason reason);

private:
	void InitWindowMenu();
	void InitModel();
	void InitWindow();
	void InitFilterEdit();
	void InitTrayIcon();
	void DisplayWindow(bool useMousePosIfNotVisible);

	// overrides
	bool nativeEvent(const QByteArray &eventType, void *message, long *result) override;
	bool eventFilter(QObject *target, QEvent *event) override;
	void closeEvent(QCloseEvent *event) override;

private:
	// elements of main window
	QMenuBar    *mMenuBar = nullptr;
	QLineEdit   *mFilterPatternLineEdit = nullptr;
	QTreeView   *mProxyView = nullptr;
	QSortFilterProxyModel *mProxyModel = nullptr;
	QString		mFileDlgDir;
	QAction		*mToggleHotkeyAction = nullptr;
	QAction		*mToggleClearFilterAction = nullptr;
	QAction		*mToggleDarkmodeAction = nullptr;
	QAction		*mToggleHiddenAction = nullptr;
	bool        mClearFilterOnExec = true;
	bool		mShowHiddenContent = false;
	bool		mDarkMode = false;

	// state for system tray
	QAction		*mRestoreAction = nullptr;
	QAction		*mQuitAction = nullptr;
	QSystemTrayIcon *mTrayIcon = nullptr;
	QMenu		*mTrayIconMenu = nullptr;

	bool		mHotkeyEnabled = false;
};

#endif // WINDOW_H
