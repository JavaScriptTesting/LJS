
#ifndef MODEL_H
#define MODEL_H

#include <QObject>
#include <QSharedPointer>
#include <QSet>
#include <QHash>
#include <QMap>
#include <QStringList>
#include "util/fileutil.h"


#include "transitionItem.h"

extern QString subjectName;
extern QStringList originalModel;

namespace artemis {

class Model
{

public:
    Model() {
        std::cout << "model" << std::endl;
    }
    Model(QStringList);
    QSet<TransitionItem> getTransitions();
    void printModel();
    QString getStartState();
    QString getEndState();
    void buildModel();

    int getSize();
    TransitionItem* getNextEventByRandom(QString);

    bool hasNextEvent(QString);
    int getNumOfState();

    QString getNextState(QString from, QString event);

    void persistModel(QStringList);
    static void loadModel() {
        std::cout << "loadModel" << std::endl;
        QString filename = "/home/pengfei/guided-artemis/raw-data/" + subjectName + "/info/model.txt";
        std::cout << filename.toStdString() << std::endl;
        QString fileModel = readFile(filename);

        foreach(QString s, fileModel.split("\n")) {
            if(!s.isEmpty())
                originalModel.append(s);
        }

    }

    QMap<QString, QSet<TransitionItem>* > getAdjoinSet();
    QSet<QString> getEnabled(QString state);


private:

    QString startState;

    QStringList oModel;

    
    QSet<TransitionItem> transitions;

    QMap<QString, QSet<TransitionItem>* > adjoinSet;
    
    QString endState;
    int numOfState;

   

};


}

#endif // JAVASCRIPTSTATISTICS_H
