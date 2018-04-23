#include <stdio.h>
#include <sys/time.h>
#include <string>
#include <vector>
#include <QStringList>
#include <QSet>
#include <QString>
#include <QDateTime>
#include <QMap>


int testType = 0;

int iterationCnt;
int linesCovered;
int totalTry;
int blockedCnt;
int blockedCnt2;
double totaltime1;
double totaltime2;

int MAX_TEST_SUIT_SIZE = 1;
int MAX_TEST_SEQUENCE_LENGTH;
int MAX_STATE_LENGTH = 3;

int modelIteration;

struct timeval start;

QStringList originalModel;
QSet<QString> handlersSet;

bool modelGeneration = false;

QDateTime startTime;

bool randomOrWeight = true;
double parameterD = 1;

QString subjectName;

QString lastEvent = "";

QMap<QString, QStringList* > depMap;

bool isAbstract = false;
bool fflag[10] = {0,0,0,0,0,0,0,0,0,0};
int rres[10];

int numOfModel = 1;
int totalExe;


