#include "Model.h"
#include <QFileInfo>
#include <QDirIterator>
#include <QSettings>
#include <QCoreApplication>
#include <QStandardPaths>
#include <QSet>
#include <QRegularExpression>
#ifdef Q_OS_WIN
#include <windows.h>
#endif
#include "SourceListEditorDlg.h"


class PopulateModel
{
public:
	PopulateModel(QObject *parent)
	{
		mModel = new QStandardItemModel(0, 2, parent);
		mModel->setHeaderData(0, Qt::Horizontal, QObject::tr("Name"));
		mModel->setHeaderData(1, Qt::Horizontal, QObject::tr("Path"));

		QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());
		QString exclusions = settings.value("exclusions", "").toString();
		if (!exclusions.isEmpty())
		{
			mExclusions = QRegularExpression(exclusions, QRegularExpression::PatternOption::CaseInsensitiveOption);
			mExclusions.optimize();
			mFilter = true;
		}
	}

	~PopulateModel() = default;

	QStandardItemModel* GetModel() const { return mModel; }

	void AddModelItem(const QString &name, const QString &path)
	{
		if (mAdded.contains(path))
			return;
		
		mAdded.insert(path);
		mModel->insertRow(0);
		mModel->setData(mModel->index(0, 0), name, Qt::DisplayRole);
		mModel->setData(mModel->index(0, 1), path, Qt::DisplayRole);
	}

	void AddModelDirItems(const QString &dir, 
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
							AddModelDirItems(newDir, allowHidden, recurse);
						}
					}

					continue;
				}

				if (allowHidden || !(wfd.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN))
				{
					const QString full(dir + "\\" + curFile);
					if (mFilter)
					{
						QRegularExpressionMatch it = mExclusions.match(full);
						if (it.hasMatch())
							continue;
					}

					if (mHasDirSearchSpec)
					{
						QRegularExpressionMatch it = mDirSearchSpec.match(curFile);
						if (!it.hasMatch())
							continue;
					}

					AddModelItem(curFile, full);
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

			if (allowHidden || !fi.isHidden())
			{
				const QString full(QDir::toNativeSeparators(fi.canonicalFilePath()));
				if (mFilter)
				{
					QRegularExpressionMatch it = mExclusions.match(full);
					if (it.hasMatch())
						continue;
				}

				if (mHasDirSearchSpec)
				{
					QRegularExpressionMatch it = mDirSearchSpec.match(fi.fileName());
					if (!it.hasMatch())
						continue;
				}

				AddModelItem(fi.fileName(), full);
			}
		}
#endif
	}

	void SetDirectorySearchSpec(QString searchSpec)
	{
		if (searchSpec.isEmpty())
		{
			mHasDirSearchSpec = false;
			return;
		}

		mHasDirSearchSpec = true;
		searchSpec.replace(".", "[.]");
		searchSpec.replace("*", ".*");
		mDirSearchSpec = QRegularExpression{ searchSpec, QRegularExpression::PatternOption::CaseInsensitiveOption };
	}

private:
	QStandardItemModel *mModel = nullptr;
	QSet<QString> mAdded;
	QRegularExpression mExclusions;
	QRegularExpression mDirSearchSpec;
	bool mFilter = false;
	bool mHasDirSearchSpec = false;
};

void
SplitSearchSpec(QString full, 
	QString &dir, 
	QString &searchSpec)
{
	dir = full;
	searchSpec.clear();

	int p1 = full.indexOf("[[");
	if (-1 == p1)
		return;

	int p2 = full.indexOf("]]");
	if (-1 == p2)
		return;

	dir = full.left(p1);
	searchSpec = full.mid(p1 + 2, p2 - p1 - 2);
}

QAbstractItemModel *
LoadModel(QObject *parent, bool allowHidden)
{
	PopulateModel pm(parent);

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
			pm.AddModelDirItems(dir, allowHidden, false);
			dir = "[dir]" + dir;
			AppendToDatafile(dir);
		}

		dir = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
		if (!dir.isEmpty())
		{
			dir = QDir::toNativeSeparators(dir);
			pm.AddModelDirItems(dir, allowHidden, false);
			dir = "[dir]" + dir;
			AppendToDatafile(dir);
		}

		dir = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
		if (!dir.isEmpty())
		{
			dir = QDir::toNativeSeparators(dir);
			pm.AddModelDirItems(dir, allowHidden, false);
			dir = "[dir]" + dir;
			AppendToDatafile(dir);
		}

		return pm.GetModel();
	}

	QFile f(fileName);
	if (!f.exists())
		return pm.GetModel();

	if (!f.open(QIODevice::ReadOnly))
		return pm.GetModel();

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
			QString dir, searchSpec;
			SplitSearchSpec(full, dir, searchSpec);
			pm.SetDirectorySearchSpec(searchSpec);
			pm.AddModelDirItems(dir, allowHidden, false);
		}
		else if (tag == "[rdir]")
		{
			QString dir, searchSpec;
			SplitSearchSpec(full, dir, searchSpec);
			pm.SetDirectorySearchSpec(searchSpec);
			pm.AddModelDirItems(dir, allowHidden, true);
		}
		else if (tag == "[file]")
		{
			QFileInfo f(full);
			if (f.isFile())
				pm.AddModelItem(f.fileName(), full);
		}
		else if (tag == "[url]")
		{
			pm.AddModelItem(full, full);
		}
		else if (-1 == tag.indexOf(" inactive"))
		{
			pm.AddModelItem("unknown tag", curItem);
		}
	}

	return pm.GetModel();
}
