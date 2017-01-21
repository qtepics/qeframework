#ifndef FLIPROTATEMENU_H
#define FLIPROTATEMENU_H

#include <QMenu>
#include "imageContextMenu.h"

class flipRotateMenu : public QMenu
{
    Q_OBJECT
public:

    explicit flipRotateMenu( QWidget *parent = 0 );

    imageContextMenu::imageContextMenuOptions getFlipRotate( const QPoint& pos );
    void setChecked( const int rotation, const bool flipH, const bool flipV );      // Set the initial state of the menu to reflect the current state of the image

signals:

public slots:

private:
    QAction* rotationNoneAction;
    QAction* rotation90RAction;
    QAction* rotation90LAction;
    QAction* rotation180Action;
    QAction* flipHAction;
    QAction* flipVAction;
};

#endif // FLIPROTATEMENU_H
