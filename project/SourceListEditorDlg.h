#pragma once

#include <qdialog>

QT_BEGIN_NAMESPACE
class QTreeView;
class QAction;
class QStandardItemModel;
class QCloseEvent;
QT_END_NAMESPACE


class SourceListEditorDlg : public QDialog
{
public:
	SourceListEditorDlg(QWidget *parent);
	~SourceListEditorDlg() = default;
	
protected slots:
	void OnAccepted();
	void OnAddDirectory();
	void OnAddFile();
	void OnDeleteItem();

protected:
	void done(int r) override;

private:
	enum struct ItemType { itFile, itDirectory, itDirectoryRecurse, itUrl, itOther, itCnt };
	QString GetItemTypeStr(SourceListEditorDlg::ItemType it);
	void InitWindow();
	void CreateModel();
	void LoadData();
	void ParseAndAddLine(int row, const QString &curItem);
	void AddItem(int row, ItemType itType, const QString &itemText, bool enabled = true);

	QTreeView *mList = nullptr;
	QStandardItemModel *mModel = nullptr;
	int mModelRows = 0;
};

void AppendToDatafile(const QString &txt);
