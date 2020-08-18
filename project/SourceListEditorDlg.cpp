#include "SourceListEditorDlg.h"
#include <QSettings>
#include <QCoreApplication>
#include <QFile>
#include <QStandardPaths>
#include <QTreeView>
#include <QPushButton>
#include <QBoxLayout>
#include <QStandardItemModel>
#include <QCloseEvent>
#include <QFileDialog>
#include <QTextStream>


SourceListEditorDlg::SourceListEditorDlg(QWidget *parent) : 
	QDialog(parent, Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint)
{
	InitWindow();
	CreateModel();
	LoadData();
}

void
SourceListEditorDlg::InitWindow()
{
	setSizeGripEnabled(true);

	connect(this, &QDialog::accepted, this, &SourceListEditorDlg::OnAccepted);

	QPushButton *addDirButton = new QPushButton("Add &Directory", this);
	addDirButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	connect(addDirButton, &QAbstractButton::clicked, this, &SourceListEditorDlg::OnAddDirectory);
	QPushButton *addFileButton = new QPushButton("Add &File", this);
	addFileButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	connect(addFileButton, &QAbstractButton::clicked, this, &SourceListEditorDlg::OnAddFile);
	QPushButton *deleteButton = new QPushButton("&Remove", this);
	deleteButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	connect(deleteButton, &QAbstractButton::clicked, this, &SourceListEditorDlg::OnDeleteItem);

	QHBoxLayout *buttonsRow1 = new QHBoxLayout;
	buttonsRow1->addWidget(addDirButton);
	buttonsRow1->setAlignment(addDirButton, Qt::AlignLeft);
	buttonsRow1->addWidget(addFileButton);
	buttonsRow1->setAlignment(addFileButton, Qt::AlignLeft);
	buttonsRow1->addWidget(deleteButton);
	buttonsRow1->setAlignment(deleteButton, Qt::AlignLeft);
	buttonsRow1->addSpacerItem(new QSpacerItem(20, 0, QSizePolicy::Expanding));

	mList = new QTreeView;
	mList->setRootIsDecorated(false);
	mList->setAlternatingRowColors(false);
	mList->setSortingEnabled(false);
	mList->setEditTriggers(QAbstractItemView::NoEditTriggers);
	mList->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
	mList->setSelectionMode(QAbstractItemView::SelectionMode::SingleSelection);

	QPushButton *okButton = new QPushButton("OK", this);
	okButton->setDefault(true);
	okButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	connect(okButton, &QAbstractButton::clicked, this, &QDialog::accept);
	QPushButton *cancelButton = new QPushButton("Cancel", this);
	cancelButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	connect(cancelButton, &QAbstractButton::clicked, this, &QDialog::reject);

	QHBoxLayout *buttonsRow2 = new QHBoxLayout;
	buttonsRow2->addSpacerItem(new QSpacerItem(20, 0, QSizePolicy::Expanding));
	buttonsRow2->addWidget(okButton);
	buttonsRow2->setAlignment(okButton, Qt::AlignRight);
	buttonsRow2->addWidget(cancelButton);
	buttonsRow2->setAlignment(cancelButton, Qt::AlignRight);

	QVBoxLayout *mainLayout = new QVBoxLayout;
	mainLayout->addLayout(buttonsRow1);
	mainLayout->addWidget(mList);
	mainLayout->addLayout(buttonsRow2);
	setLayout(mainLayout);

	setWindowTitle(tr("List Editor"));

	QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());
	const QSize sz = settings.value("editWndSize", QSize()).toSize();
	if (sz.isEmpty())
		resize(850, 400);
	else
		resize(sz);
}

void
SourceListEditorDlg::done(int r)
{
	QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());
	settings.setValue("editWndSize", size());
	QDialog::done(r);
}

QString
SourceListEditorDlg::GetItemTypeStr(ItemType it)
{
	switch (it)
	{
	case ItemType::itFile:
		return "file";
	case ItemType::itDirectory:
		return "dir";
	case ItemType::itDirectoryRecurse:
		return "rdir";
	case ItemType::itUrl:
		return "url";
	case ItemType::itOther:
	default:
		return "";
	}
}

void
SourceListEditorDlg::OnAccepted()
{
	QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());
	QString fileName(settings.value("dataFile", "").toString());
	if (fileName.isEmpty())
	{
		QString dir;
		dir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
		if (dir.isEmpty())
			return;

		dir = QDir::toNativeSeparators(dir);
		QDir d;
		d.mkpath(dir);
		fileName = dir + "/openfile.txt";
		settings.setValue("dataFile", fileName);
	}

	QFile f(fileName);
	if (!f.open(QIODevice::WriteOnly))
		return;

	QTextStream out(&f);
	for (int row = 0; row < mModel->rowCount(); ++row)
	{
		QVariant col0(mModel->item(row, 0)->data(Qt::CheckStateRole));
		QVariant col1(mModel->item(row, 1)->data(Qt::CheckStateRole));
		QVariant col2(mModel->item(row, 2)->data(Qt::DisplayRole));
		QVariant col3(mModel->item(row, 3)->data(Qt::DisplayRole));

		const QString txt(col2.toString());
		if (txt.isEmpty())
			continue;

		bool en = col0.toBool();
		ItemType it = (ItemType)col3.toUInt();
		if (it > ItemType::itOther)
			it = ItemType::itOther;

		if (it == ItemType::itDirectory || it == ItemType::itDirectoryRecurse)
		{
			if (col1.toBool())
				it = ItemType::itDirectoryRecurse;
			else
				it = ItemType::itDirectory;
		}
	
		QString outTxt;
		if (ItemType::itOther != it)
		{
			outTxt = "[";
			outTxt += GetItemTypeStr(it);
			if (!en)
				outTxt += " inactive";
			outTxt += "]";
		}

		outTxt += txt + "\n";
		out << outTxt;
	}
}

void
SourceListEditorDlg::OnAddDirectory()
{
	const QString dir(QFileDialog::getExistingDirectory(this, "Select Directory"));
	if (dir.isEmpty())
		return;

	AddItem(mModelRows++, ItemType::itDirectory, QDir::toNativeSeparators(dir), true);
}

void
SourceListEditorDlg::OnAddFile()
{
	const QString f(QFileDialog::getOpenFileName(this, "Select File"));
	if (f.isEmpty())
		return;

	AddItem(mModelRows++, ItemType::itFile, QDir::toNativeSeparators(f), true);
}

void
SourceListEditorDlg::OnDeleteItem()
{
	QModelIndexList items(mList->selectionModel()->selectedIndexes());
	for (const auto &it : items)
	{
		if (it.column() != 0)
			continue;

		if (mModel->removeRow(it.row()))
			--mModelRows;
	}
}

void
SourceListEditorDlg::LoadData()
{
	QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());
	QString fileName(settings.value("dataFile", "").toString());
	if (fileName.isEmpty())
	{
		int row = 0;
		QString dir;
		// default to documents, downloads and desktop
		dir = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
		if (!dir.isEmpty())
		{
			dir = QDir::toNativeSeparators(dir);
			AddItem(row++, ItemType::itDirectory, dir);
		}

		dir = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
		if (!dir.isEmpty())
		{
			dir = QDir::toNativeSeparators(dir);
			AddItem(row++, ItemType::itDirectory, dir);
		}

		dir = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
		if (!dir.isEmpty())
		{
			dir = QDir::toNativeSeparators(dir);
			AddItem(row++, ItemType::itDirectory, dir);
		}
		return;
	}

	QFile f(fileName);
	if (!f.exists())
		return;

	if (!f.open(QIODevice::ReadOnly))
		return;

	mModelRows = 0;
	while (!f.atEnd())
	{
		const QString curItem(f.readLine());
		if (!curItem.isEmpty())
			ParseAndAddLine(mModelRows++, curItem);
	}

	mList->setCurrentIndex(mModel->index(0, 0));

	mList->resizeColumnToContents(0);
	mList->resizeColumnToContents(1);
	mList->resizeColumnToContents(2);
	mList->resizeColumnToContents(3);
}

void
SourceListEditorDlg::ParseAndAddLine(int row, const QString &curItem)
{
	if (curItem[0] != '[')
	{
		// comment or ill-formed
		AddItem(row, ItemType::itOther, curItem);
		return;
	}

	int pos = curItem.indexOf(']');
	if (-1 == pos)
	{
		// ill-formed
		AddItem(row, ItemType::itOther, curItem);
		return;
	}

	QString tag(curItem.left(pos + 1));
	bool isEnabled = tag.indexOf("inactive") == -1;
	tag.replace(QRegExp(" *inactive *"), "");

	QString full(curItem.mid(pos + 1));
	full = full.trimmed();

	ItemType it = ItemType::itOther;
	if (tag == "[dir]")
		it = ItemType::itDirectory;
	else if (tag == "[rdir]")
		it = ItemType::itDirectoryRecurse;
	else if (tag == "[file]")
		it = ItemType::itFile;
	else if (tag == "[url]")
		it = ItemType::itUrl;

	AddItem(row, it, ItemType::itOther == it ? curItem : full, isEnabled);
}

void
SourceListEditorDlg::AddItem(int row, 
	ItemType itType, 
	const QString &itemText, 
	bool enabled /*= true*/)
{
	// https://www.walletfox.com/course/qtcheckablelist.php
	// https://stackoverflow.com/questions/31163516/qlistview-item-with-checkbox-selection-behavior
	// https://stackoverflow.com/questions/14158191/qt-qtreeview-and-custom-model-with-checkbox-columns?rq=1

	QStandardItem *pListItem = new QStandardItem;
	pListItem->setCheckable(true);
	pListItem->setCheckState(enabled ? Qt::Checked : Qt::Unchecked);
	pListItem->setData(enabled ? Qt::Checked : Qt::Unchecked, Qt::CheckStateRole);
	mModel->setItem(row, 0, pListItem);

	pListItem = new QStandardItem;
	if (itType == ItemType::itDirectoryRecurse || itType == ItemType::itDirectory)
	{
		pListItem->setCheckable(true);
		pListItem->setCheckState(itType == ItemType::itDirectoryRecurse ? Qt::Checked : Qt::Unchecked);
		pListItem->setData(itType == ItemType::itDirectoryRecurse ? Qt::Checked : Qt::Unchecked, Qt::CheckStateRole);
	}
	else
	{
		pListItem->setCheckable(false);
	}
	mModel->setItem(row, 1, pListItem);

	mModel->setData(mModel->index(row, 2), itemText, Qt::DisplayRole);
	mModel->setData(mModel->index(row, 3), (int)itType, Qt::DisplayRole);
}

void
SourceListEditorDlg::CreateModel()
{
	mModel = new QStandardItemModel(0, 4, this);
	mModel->setHeaderData(0, Qt::Horizontal, QObject::tr("Enabled"));
	mModel->setHeaderData(1, Qt::Horizontal, QObject::tr("Subdirectories"));
	mModel->setHeaderData(2, Qt::Horizontal, QObject::tr("Item"));
	mModel->setHeaderData(3, Qt::Horizontal, QObject::tr("Type"));

	mList->setModel(mModel);
	mList->hideColumn(3);
}

void
AppendToDatafile(const QString &txt)
{
	if (txt.isEmpty())
		return;

	QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());
	QString fileName(settings.value("dataFile", "").toString());
	if (fileName.isEmpty())
		return;

	QFile f(fileName);
	if (!f.open(QIODevice::WriteOnly | QIODevice::Append))
		return;

	QTextStream out(&f);
	out << txt;

	if (txt.at(txt.length() - 1) != '\n')
		out << "\n";
}
