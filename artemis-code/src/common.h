#include <stdio.h>
#include <sys/time.h>
#include <string>
#include <vector>
#include <QStringList>
#include <QSet>
#include <QString>
#include <QDateTime>
#include <QMap>

//0 for random generation
//1 for McMc
//2 for random generation and use dependency relations 
//3 for sleep set + ASE17
//4 for sleep set
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

//nums of events that explores during model generation 
int modelIteration;

struct timeval start;

QStringList originalModel;
QSet<QString> handlersSet;

bool modelGeneration = false;

QDateTime startTime;

//true for random, false for weight
bool randomOrWeight = true;
//model generation, if the next event depends on current event, then parameterD, else 1 - parameterD
double parameterD = 1;

QString subjectName;

QString lastEvent = "";

//dependency relations
QMap<QString, QStringList* > depMap;

bool isAbstract = false;
bool fflag[10] = {0,0,0,0,0,0,0,0,0,0};
int rres[10];

int numOfModel = 1;
int totalExe;
//bool onlyModel = true;

// QMap<QString, double> fse_coverage;
// fse_coverage.insert();
// fse_coverage.insert();
// fse_coverage.insert();
// fse_coverage.insert();
// fse_coverage.insert();
// fse_coverage.insert();
// fse_coverage.insert();
// fse_coverage.insert();
// fse_coverage.insert();
// fse_coverage.insert();
// fse_coverage.insert();
// fse_coverage.insert();
// fse_coverage.insert();
// fse_coverage.insert();
// fse_coverage.insert();
// fse_coverage.insert();

