#ifndef TESTSUITSMANAGER_H
#define TESTSUITSMANAGER_H

#include <QObject>
#include <QSharedPointer>
#include <QSet>
#include <QHash>
#include <QMap>
#include <QStringList>

#include "model.h"
#include "state.h"

extern int MAX_TEST_SUIT_SIZE;
extern int MAX_TEST_SEQUENCE_LENGTH;
extern int MAX_STATE_LENGTH;
extern QMap<QString, QStringList* > depMap;
extern QString subjectName;


namespace artemis {
    class TestsuitsManager {
    private:
        Model* model;
        QList<QStringList> testsuit;
        bool isEquivalent(QString existEvent1, QString existEvent2, QString newEvent1, QString newEvent2);
        bool isDc(QString event1, QString event2); 
    public:
        TestsuitsManager(Model*);
        void generateTestsuit(int type);
        QList<QStringList>* getTestsuit();
        void explore(QList<State* >* S);

        bool redundantSequence(QList<State* >* S, State* s); 
        QSet<QString> calSleep(State& s, QString event);
        static void readDependencyFromFile() {
            std::cout << "Reading dependency from file" << std::endl;
            QString depFilePath = QString("/home/pengfei/guided-artemis/raw-data/") + subjectName + QString("/info/dep.txt");
            QString readRestul = readFile(depFilePath);
            QStringList templist = readRestul.split("\n");
            for(int i = 0; i < templist.size(); i += 4) {
                if(templist[i].toStdString() == "")
                    break;
                QString event1 = templist[i + 1] + "@" + templist[i];
                QString event2 = templist[i + 3] + "@" + templist[i + 2];

                if(depMap.contains(event2)) {
                    *(depMap[event2]) << event1;
                } else {
                    depMap[event2] = new QStringList();
                    *(depMap[event2]) << event1;
                }
            }
        }
    };


}

#endif