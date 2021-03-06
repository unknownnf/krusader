/*****************************************************************************
 * Copyright (C) 2000 Shie Erlich <erlich@users.sourceforge.net>             *
 * Copyright (C) 2000 Rafi Yanai <yanai@users.sourceforge.net>               *
 *                                                                           *
 * This program is free software; you can redistribute it and/or modify      *
 * it under the terms of the GNU General Public License as published by      *
 * the Free Software Foundation; either version 2 of the License, or         *
 * (at your option) any later version.                                       *
 *                                                                           *
 * This package is distributed in the hope that it will be useful,           *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
 * GNU General Public License for more details.                              *
 *                                                                           *
 * You should have received a copy of the GNU General Public License         *
 * along with this package; if not, write to the Free Software               *
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA *
 *****************************************************************************/

#include "krdialogs.h"

// QtCore
#include <QDir>
// QtGui
#include <QFontMetrics>
#include <QKeyEvent>
// QtWidgets
#include <QApplication>
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QFrame>
#include <QLabel>
#include <QMessageBox>
#include <QVBoxLayout>

#include <KI18n/KLocalizedString>
#include <KIOWidgets/KUrlCompletion>
#include <KCompletion/KLineEdit>
#include <KIOWidgets/KUrlRequester>
#include <KIOCore/KRecentDocument>
#include <KWidgetsAddons/KGuiItem>

#include "../krglobal.h"
#include "../VFS/vfs.h"
#include "../defaults.h"


QUrl KChooseDir::getFile(QString text, const QUrl& url, const QUrl& cwd)
{
    QScopedPointer<KUrlRequesterDialog> dlg(new KUrlRequesterDialog(vfs::ensureTrailingSlash(url), text, krMainWindow));
    dlg->urlRequester()->setStartDir(cwd);
    dlg->urlRequester()->setMode(KFile::File);
    dlg->exec();
    QUrl u = dlg->selectedUrl(); // empty if cancelled
    if (u.scheme() == "zip" || u.scheme() == "krarc" || u.scheme() == "tar" || u.scheme() == "iso") {
        if (QDir(u.path()).exists()) {
            u.setScheme("file");
        }
    }
    return u;
}


QUrl KChooseDir::getDir(QString text, const QUrl& url, const QUrl& cwd)
{
    QScopedPointer<KUrlRequesterDialog> dlg(new KUrlRequesterDialog(vfs::ensureTrailingSlash(url), text, krMainWindow));
    dlg->urlRequester()->setStartDir(cwd);
    dlg->urlRequester()->setMode(KFile::Directory);
    dlg->exec();
    QUrl u = dlg->selectedUrl();
    if (u.scheme() == "zip" || u.scheme() == "krarc" || u.scheme() == "tar" || u.scheme() == "iso") {
        if (QDir(u.path()).exists()) {
            u.setScheme("file");
        }
    }
    return u;
}

QUrl KChooseDir::getDir(QString text, const QUrl& url, const QUrl& cwd, bool &queue)
{
    QScopedPointer<KUrlRequesterDlgForCopy> dlg(new KUrlRequesterDlgForCopy(vfs::ensureTrailingSlash(url),
                                                                            text, false, krMainWindow));
    dlg->hidePreserveAttrs();
    dlg->urlRequester()->setStartDir(cwd);
    dlg->urlRequester()->setMode(KFile::Directory);
    dlg->exec();
    QUrl u = dlg->selectedURL();
    if (u.scheme() == "zip" || u.scheme() == "krarc" || u.scheme() == "tar" || u.scheme() == "iso") {
        if (QDir(u.path()).exists()) {
            u.setScheme("file");
        }
    }
    queue = dlg->enqueue();
    return u;
}

QUrl KChooseDir::getDir(QString text, const QUrl& url, const QUrl& cwd, bool &queue, bool &preserveAttrs)
{
    QScopedPointer<KUrlRequesterDlgForCopy> dlg(new KUrlRequesterDlgForCopy(vfs::ensureTrailingSlash(url),
                                                                            text, preserveAttrs, krMainWindow));
    dlg->urlRequester()->setStartDir(cwd);
    dlg->urlRequester()->setMode(KFile::Directory);
    dlg->exec();
    QUrl u = dlg->selectedURL();
    if (u.scheme() == "zip" || u.scheme() == "krarc" || u.scheme() == "tar" || u.scheme() == "iso") {
        if (QDir(u.path()).exists()) {
            u.setScheme("file");
        }
    }
    preserveAttrs = dlg->preserveAttrs();
    queue = dlg->enqueue();
    return u;
}

QUrl KChooseDir::getDir(QString text, const QUrl& url, const QUrl& cwd, bool &queue, bool &preserveAttrs, QUrl &baseURL)
{
    QScopedPointer<KUrlRequesterDlgForCopy> dlg(new KUrlRequesterDlgForCopy(vfs::ensureTrailingSlash(url),
                                                                            text, preserveAttrs, krMainWindow,
                                                                            true, baseURL));
    dlg->urlRequester()->setStartDir(cwd);
    dlg->urlRequester()->setMode(KFile::Directory);
    dlg->exec();
    QUrl u = dlg->selectedURL();
    if (u.scheme() == "zip" || u.scheme() == "krarc" || u.scheme() == "tar" || u.scheme() == "iso") {
        if (QDir(u.path()).exists()) {
            u.setScheme("file");
        }
    }
    if (dlg->copyDirStructure()) {
        baseURL = dlg->baseURL();
    } else {
        baseURL = QUrl();
    }
    preserveAttrs = dlg->preserveAttrs();
    queue = dlg->enqueue();
    return u;
}

KUrlRequesterDlgForCopy::KUrlRequesterDlgForCopy(const QUrl &urlName, const QString& _text, bool /*presAttrs*/, QWidget *parent,
        bool modal, QUrl baseURL)
        :   QDialog(parent), baseUrlCombo(0), copyDirStructureCB(0), queue(false)
{
    setWindowModality(modal ? Qt::WindowModal : Qt::NonModal);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);

    mainLayout->addWidget(new QLabel(_text));

    urlRequester_ = new KUrlRequester(urlName, this);
    urlRequester_->setMinimumWidth(urlRequester_->sizeHint().width() * 3);
    mainLayout->addWidget(urlRequester_);
//     preserveAttrsCB = new QCheckBox(i18n("Preserve attributes (only for local targets)"), widget);
//     preserveAttrsCB->setChecked(presAttrs);
//     topLayout->addWidget(preserveAttrsCB);
    if (!baseURL.isEmpty()) {
        QFrame *line = new QFrame(this);
        line->setFrameStyle(QFrame::HLine | QFrame::Sunken);
        mainLayout->addWidget(line);
        copyDirStructureCB = new QCheckBox(i18n("Keep virtual folder structure"), this);
        connect(copyDirStructureCB, SIGNAL(toggled(bool)), this, SLOT(slotDirStructCBChanged()));
        copyDirStructureCB->setChecked(false);
        mainLayout->addWidget(copyDirStructureCB);
        QWidget *hboxWidget = new QWidget(this);
        QHBoxLayout * hbox = new QHBoxLayout(hboxWidget);
        QLabel * lbl = new QLabel(i18n("Base URL:"),  hboxWidget);
        hbox->addWidget(lbl);

        baseUrlCombo = new QComboBox(hboxWidget);
        baseUrlCombo->setMinimumWidth(baseUrlCombo->sizeHint().width() * 3);
        baseUrlCombo->setEnabled(copyDirStructureCB->isChecked());
        hbox->addWidget(baseUrlCombo);

        QUrl temp = baseURL, tempOld;
        do {
            QString baseURLText = temp.toDisplayString(QUrl::PreferLocalFile);
            baseUrlCombo->addItem(baseURLText);
            tempOld = temp;
            temp = KIO::upUrl(temp);
        } while (!tempOld.matches(temp, QUrl::StripTrailingSlash));
        baseUrlCombo->setCurrentIndex(0);

        mainLayout->addWidget(hboxWidget);
    }

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
    mainLayout->addWidget(buttonBox);

    okButton = buttonBox->button(QDialogButtonBox::Ok);
    okButton->setDefault(true);
    okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    QPushButton *queueButton = new QPushButton(i18n("F2 Queue"), this);
    buttonBox->addButton(queueButton, QDialogButtonBox::ActionRole);
    connect(buttonBox, SIGNAL(accepted()), SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), SLOT(reject()));
    connect(queueButton, SIGNAL(clicked()), SLOT(slotQueue()));
    connect(urlRequester_, SIGNAL(textChanged(QString)), SLOT(slotTextChanged(QString)));

    urlRequester_->setFocus();
    bool state = !urlName.isEmpty();
    okButton->setEnabled(state);
}

KUrlRequesterDlgForCopy::KUrlRequesterDlgForCopy()
{
}

void KUrlRequesterDlgForCopy::keyPressEvent(QKeyEvent *e)
{
    switch (e->key()) {
    case Qt::Key_F2:
        slotQueue();
        return;
    default:
        QDialog::keyPressEvent(e);
    }
}

bool KUrlRequesterDlgForCopy::preserveAttrs()
{
//     return preserveAttrsCB->isChecked();
    return true;
}

bool KUrlRequesterDlgForCopy::copyDirStructure()
{
    if (copyDirStructureCB == 0)
        return false;
    return copyDirStructureCB->isChecked();
}

void KUrlRequesterDlgForCopy::slotTextChanged(const QString & text)
{
    bool state = !text.trimmed().isEmpty();
    okButton->setEnabled(state);
}

void KUrlRequesterDlgForCopy::slotQueue()
{
    queue = true;
    accept();
}

void KUrlRequesterDlgForCopy::slotDirStructCBChanged()
{
    baseUrlCombo->setEnabled(copyDirStructureCB->isChecked());
}

QUrl KUrlRequesterDlgForCopy::selectedURL() const
{
    if (result() == QDialog::Accepted) {
        QUrl url = urlRequester_->url();
        if (url.isValid())
            KRecentDocument::add(url);
        return url;
    } else
        return QUrl();
}

KUrlRequester * KUrlRequesterDlgForCopy::urlRequester()
{
    return urlRequester_;
}

QUrl KUrlRequesterDlgForCopy::baseURL() const
{
    if (baseUrlCombo == 0)
        return QUrl();
    return QUrl::fromUserInput(baseUrlCombo->currentText(), QString(), QUrl::AssumeLocalFile);
}

KRGetDate::KRGetDate(QDate date, QWidget *parent) : QDialog(parent, Qt::MSWindowsFixedSizeDialogHint)
{
    setWindowModality(Qt::WindowModal);
    dateWidget = new KDatePicker(this);
    dateWidget->setDate(date);
    dateWidget->resize(dateWidget->sizeHint());
    setMinimumSize(dateWidget->sizeHint());
    setMaximumSize(dateWidget->sizeHint());
    resize(minimumSize());
    connect(dateWidget, SIGNAL(dateSelected(QDate)), this, SLOT(setDate(QDate)));
    connect(dateWidget, SIGNAL(dateEntered(QDate)), this, SLOT(setDate(QDate)));

    // keep the original date - incase ESC is pressed
    originalDate  = date;
}

QDate KRGetDate::getDate()
{
    if (exec() == QDialog::Rejected) chosenDate = QDate();
    hide();
    return chosenDate;
}

void KRGetDate::setDate(QDate date)
{
    chosenDate = date;
    accept();
}

