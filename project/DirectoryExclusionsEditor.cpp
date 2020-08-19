#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QBoxLayout>
#include <QSettings>
#include <QCoreApplication>
#include <QRegularExpression>
#include "DirectoryExclusionsEditor.h"


DirectoryExclusionsEditor::DirectoryExclusionsEditor(QWidget *parent) :
	QDialog(parent, Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint)
{
	InitWindow();
}

void
DirectoryExclusionsEditor::InitWindow()
{
	QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());
	setSizeGripEnabled(true);

	connect(this, &QDialog::accepted, this, &DirectoryExclusionsEditor::OnAccepted);

	mStatus = new QLabel;
	const QString statusStyle = 
	{
		"QLabel { \
			color: red; \
		}"
	};
	mStatus->setStyleSheet(statusStyle);

	mEdit = new QLineEdit;
	mEdit->setPlaceholderText("enter regular expression to match filepaths to be excluded from directory listings");
	connect(mEdit, &QLineEdit::textChanged, this, &DirectoryExclusionsEditor::OnEditTextChanged);
	mEdit->setText(settings.value("exclusions", "").toString());

	QPushButton *okButton = new QPushButton("OK", this);
	okButton->setDefault(true);
	okButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	connect(okButton, &QAbstractButton::clicked, this, &QDialog::accept);
	QPushButton *cancelButton = new QPushButton("Cancel", this);
	cancelButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	connect(cancelButton, &QAbstractButton::clicked, this, &QDialog::reject);

	QHBoxLayout *buttonsRow = new QHBoxLayout;
	buttonsRow->addSpacerItem(new QSpacerItem(20, 0, QSizePolicy::Expanding));
	buttonsRow->addWidget(okButton);
	buttonsRow->setAlignment(okButton, Qt::AlignRight);
	buttonsRow->addWidget(cancelButton);
	buttonsRow->setAlignment(cancelButton, Qt::AlignRight);

	QVBoxLayout *mainLayout = new QVBoxLayout;
	mainLayout->addWidget(mEdit);
	mainLayout->addWidget(mStatus);
	mainLayout->addLayout(buttonsRow);
	setLayout(mainLayout);

	setWindowTitle(tr("Directory Exclusions"));
	mEdit->setFocus(Qt::FocusReason::ActiveWindowFocusReason);

	const QSize sz = settings.value("WndSizeDirExclEditor", QSize()).toSize();
	if (sz.isEmpty())
		resize(700, 140);
	else
		resize(sz);
}

void
DirectoryExclusionsEditor::done(int r)
{
	QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());
	settings.setValue("WndSizeDirExclEditor", size());
	QDialog::done(r);
}

void
DirectoryExclusionsEditor::OnAccepted()
{
	const QString txt(mEdit->text());
	QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());
	settings.setValue("exclusions", txt);
}

void
DirectoryExclusionsEditor::OnEditTextChanged()
{
	const QString txt(mEdit->text());
	QRegularExpression exp(txt);
	if (exp.isValid())
		mStatus->setText("");
	else
		mStatus->setText("Error: " + exp.errorString());
	
}
