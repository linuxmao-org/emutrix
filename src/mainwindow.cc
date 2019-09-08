/*
 * Copyright 2010 Camilo Polymeris
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QString>
#include <QErrorMessage>
#include <QDebug>
#include "soundcard.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow), card(new NullSoundCard)
{
    qDebug("Setting up UI...");
    // Qt creator magic
    ui->setupUi(this);
    QComboBox * cardsBox = this->findChild<QComboBox*>("card");
    // Hide "setup" (that is, extended settings, frame)
    this->findChild<QWidget*>("setupWidget")->setVisible(false);

    try
    {
        QList<QPair<QString, int> > cards = SoundCard::getCardList();
        for (QList<QPair<QString, int> >::iterator it = cards.begin();
            it != cards.end();
            ++it)
            cardsBox->addItem(it->first, it->second);

        if (cardsBox->count() == 0)
        {
            throw tr("Sorry! No EMU 1010 based cards found.");
            return;
        }
    }
    catch (QString err)
    {
        showError(err);
    }
    cardsBox->setCurrentIndex(0); // Calls code to initialize first card.
    startTimer(0);
}

MainWindow::~MainWindow()
{
    qDebug("Cleaning up...");
    delete card;
    delete ui;
}

////////// ERROR HANDLING
void MainWindow::showError(const QString & msg)
{
      qDebug() << "Error: " << msg;
      QErrorMessage * err = new QErrorMessage(this);
      err->showMessage(msg);
}


void MainWindow::matrixSetVisible(const int rows[], const int cols[], bool visible)
{
    QGridLayout * matrix = (QGridLayout *)ui->matrixContents->layout();
    int i, j;
    for (i = 0; rows[i] != -1; i++)
        for (j = 0; j < matrix->columnCount(); j++)
        {
            QLayoutItem * li = matrix->itemAtPosition(rows[i], j);
            // Some positions are empty, because there are spanning labels
            if (!li)
                continue;
            li->widget()->setVisible(visible);
        }
    for (j = 0; cols[j] != -1; j++)
    for (i = 0; i < matrix->rowCount(); i++)
        {
            QLayoutItem * li = matrix->itemAtPosition(i, cols[j]);
            // Some positions are empty, because there are spanning labels
            if (!li)
                continue;
            li->widget()->setVisible(visible);
        }
}

//// HELPER FUNCTIONS

void MainWindow::timerEvent(QTimerEvent *)
{
    card->updateCallbacks();
}

void MainWindow::checkLinked(QButtonGroup * bg, QButtonGroup * linked, QButtonGroup * linkedr)
{
    // L-R link enabled?
    if (!ui->link->isChecked())
        return;
    int ix = bg->checkedId();

    if (ix == -2);
    else if (linked && linked->objectName().endsWith('l'))
        ix++;
    else if(linked && linked->objectName().endsWith('r'))
        ix--;
    /// make this work for input channels. -- not quite sure if this is the right behavior
    else if (-ix >= 5 && -ix <= 14 && -ix % 2 == 0) // checked an even id => right channel
    {
        ix++; // change left
        linked = linkedr;
    }
    else if (-ix >= 5 && -ix <= 14 && -ix % 2 == 1) // checked an odd id => left channel
        ix--; // change right
    else
        return;

    if (!linked)
        return;
    if (linked->checkedId() != ix)
        // Using click() because it fires the appropiate signals
        linked->button(ix)->click();
}
