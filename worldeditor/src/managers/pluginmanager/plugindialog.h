#ifndef PLUGINMANAGER_H
#define PLUGINMANAGER_H

#include <QDialog>

#include <engine.h>

class PluginModel;

class AMod;

namespace Ui {
    class PluginDialog;
}

class PluginDialog : public QDialog  {
    Q_OBJECT

public:
    PluginDialog                    (QWidget *parent = nullptr);
    ~PluginDialog                   ();

public slots:
    void                            on_loadButton_clicked       ();

private slots:
    void                            on_closeButton_clicked      ();

protected:
    Ui::PluginDialog               *ui;
};

#endif // PLUGINMANAGER_H
