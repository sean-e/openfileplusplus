#pragma once

#include <qdialog>

QT_BEGIN_NAMESPACE
class QLineEdit;
class QLabel;
QT_END_NAMESPACE

class DirectoryExclusionsEditor : public QDialog
{
public:
	DirectoryExclusionsEditor(QWidget *parent);
	~DirectoryExclusionsEditor() = default;

protected slots:
	void OnEditTextChanged();
	void OnAccepted();

protected:
	void done(int r) override;

private:
	void InitWindow();

	QLineEdit *mEdit;
	QLabel *mStatus;
};
