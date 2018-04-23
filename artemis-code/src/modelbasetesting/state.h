
#ifndef STATE_H
#define STATE_H

#include <QObject>
#include <QSharedPointer>
#include <QSet>
#include <QHash>
#include <QMap>
#include <QStringList>


namespace artemis {

class State
{

public:
    State(QString);

    QSet<QString> getDone();
    QString getSelEV();
    void setSelEV(QString event);
    void setDone(QSet<QString>);
    QSet<QString> getEnabled();
    void addToDone(QString);
    QString getStringState();

    void setSleep(QSet<QString>);
    void addSleep(QString event);
    QSet<QString> getSleep();

    QSet<QString> getPrinted();
    void addPrinted(QString event);

private:

    QString state;
    QSet<QString> done;
    QString selEV;

    QSet<QString> enabled;

    QSet<QString> sleep;

    QSet<QString> printed;

};


}

#endif // JAVASCRIPTSTATISTICS_H
