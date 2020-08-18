#include "Model.h"
#include <QFileInfo>
#include <QDirIterator>
#include <QSettings>
#include <QCoreApplication>
#include <QStandardPaths>
#ifdef Q_OS_WIN
#include <windows.h>
#endif
#include "SourceListEditorDlg.h"


void
AddModelItem(QAbstractItemModel *model, 
	const QString &name, 
	const QString &path)
{
	model->insertRow(0);
	model->setData(model->index(0, 0), name, Qt::DisplayRole);
	model->setData(model->index(0, 1), path, Qt::DisplayRole);
}

void AddModelDirItems(QStandardItemModel *model, 
	const QString &dir, 
	bool allowHidden, 
	bool recurse)
{
#ifdef Q_OS_WIN
	// QDirIterator is too slow on Windows mapped network drives, so use native API
	HANDLE hFindFile;
	WIN32_FIND_DATAW wfd;

	std::wstring fstr((wchar_t*)dir.toStdU16String().c_str());
	fstr += L"\\*.*";
	hFindFile = ::FindFirstFileW(fstr.c_str(), &wfd);
	if (INVALID_HANDLE_VALUE != hFindFile)
	{
		do
		{
			const QString curFile(QString::fromStdWString(std::wstring(wfd.cFileName)));
			if (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				if ((wfd.cFileName[0] == L'.' && wfd.cFileName[1] == L'\0')
					|| (wfd.cFileName[0] == L'.' && wfd.cFileName[1] == L'.' && wfd.cFileName[2] == L'\0'))
				{
					// "." and ".."
				}
				else if (recurse)
				{
					if (allowHidden || !(wfd.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN))
					{
						const QString newDir(dir + "\\" + curFile);
						AddModelDirItems(model, newDir, allowHidden, true);
					}
				}

				continue;
			}

			if (allowHidden || !(wfd.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN))
			{
				const QString full(dir + "\\" + curFile);
				AddModelItem(model, curFile, full);
			}
		} 
		while (::FindNextFileW(hFindFile, &wfd));
	}

	if (hFindFile != INVALID_HANDLE_VALUE)
		::FindClose(hFindFile);
#else
	// this works on windows, but is slow on mapped network drives
	QDirIterator di(dir, recurse ? QDirIterator::IteratorFlag::Subdirectories : QDirIterator::IteratorFlag::NoIteratorFlags);
	while (di.hasNext())
	{
		QFileInfo fi(di.next());
		if (!fi.isFile())
			continue;

		// #todo-minor support file exclusions (*.obj;*.dll;*.ilk;*.pch;/.vs/;) (applied when directories are scanned before user filtering)

		if (allowHidden || !fi.isHidden())
			AddModelItem(model, fi.fileName(), QDir::toNativeSeparators(fi.canonicalFilePath()));
	}
#endif
}

QAbstractItemModel *
LoadModel(QObject *parent, bool allowHidden)
{
	QStandardItemModel *model = new QStandardItemModel(0, 2, parent);

	model->setHeaderData(0, Qt::Horizontal, QObject::tr("Name"));
	model->setHeaderData(1, Qt::Horizontal, QObject::tr("Path"));

	QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());
	QString fileName(settings.value("dataFile", "").toString());
	if (fileName.isEmpty())
	{
		QString dir;
		dir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
		if (!dir.isEmpty())
		{
			dir = QDir::toNativeSeparators(dir);
			QDir d;
			d.mkpath(dir);
			fileName = dir + "\\openfile.txt";
			settings.setValue("dataFile", fileName);
		}

		// default to documents, downloads and desktop
		dir = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
		if (!dir.isEmpty())
		{
			dir = QDir::toNativeSeparators(dir);
			AddModelDirItems(model, dir, allowHidden, false);
			dir = "[dir]" + dir;
			AppendToDatafile(dir);
		}

		dir = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
		if (!dir.isEmpty())
		{
			dir = QDir::toNativeSeparators(dir);
			AddModelDirItems(model, dir, allowHidden, false);
			dir = "[dir]" + dir;
			AppendToDatafile(dir);
		}

		dir = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
		if (!dir.isEmpty())
		{
			dir = QDir::toNativeSeparators(dir);
			AddModelDirItems(model, dir, allowHidden, false);
			dir = "[dir]" + dir;
			AppendToDatafile(dir);
		}

		return model;
	}

	QFile f(fileName);
	if (!f.exists())
		return model;

	if (!f.open(QIODevice::ReadOnly))
		return model;

	while (!f.atEnd()) 
	{
		const QString curItem(f.readLine());
		if (curItem.length() < 2)
			continue;

		if (curItem[0] != '[')
			continue; // comment or otherwise ill-formed

		int pos = curItem.indexOf(']');
		if (-1 == pos)
			continue; // ill-formed

		const QString tag(curItem.left(pos + 1));
		QString full(curItem.mid(pos + 1));
		full = full.trimmed();

		if (tag == "[dir]")
		{
			AddModelDirItems(model, full, allowHidden, false);
		}
		else if (tag == "[rdir]")
		{
			AddModelDirItems(model, full, allowHidden, true);
		}
		else if (tag == "[file]")
		{
			QFileInfo f(full);
			if (f.isFile())
				AddModelItem(model, f.fileName(), full);
		}
		else if (tag == "[url]")
		{
			AddModelItem(model, full, full);
		}
		else if (-1 == tag.indexOf(" inactive"))
		{
			AddModelItem(model, "unknown tag", curItem);
		}
	}

	return model;
}
