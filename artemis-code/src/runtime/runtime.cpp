/*
 * Copyright 2012 Aarhus University
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <assert.h>
#include <iostream>
#include <fstream>
#include <string>

#include <QSharedPointer>

#include "worklist/deterministicworklist.h"
#include "model/coverage/coveragetooutputstream.h"
#include "util/loggingutil.h"

#include "statistics/statsstorage.h"
#include "statistics/writers/pretty.h"
#include "strategies/inputgenerator/randominputgenerator.h"
#include "strategies/inputgenerator/event/staticeventparametergenerator.h"
#include "strategies/inputgenerator/form/staticforminputgenerator.h"
#include "strategies/inputgenerator/form/constantstringforminputgenerator.h"
#include "strategies/termination/numberofiterationstermination.h"

#include "strategies/prioritizer/constantprioritizer.h"
#include "strategies/prioritizer/randomprioritizer.h"
#include "strategies/prioritizer/coverageprioritizer.h"
#include "strategies/prioritizer/readwriteprioritizer.h"
#include "strategies/prioritizer/collectedprioritizer.h"

#include "runtime.h"

#include <sys/time.h>
//#include "common.h"
extern int testType;
extern int iterationCnt;
extern struct timeval start;
extern int linesCovered;
extern int totalTry;
extern int blockedCnt;
extern double totaltime1;
extern double totaltime2;
extern QStringList originalModel;
extern QSet<QString> handlersSet;
extern int modelIteration;
extern bool modelGeneration;
extern QDateTime startTime;
extern bool onlyModel;
extern int MAX_TEST_SUIT_SIZE;
extern int MAX_STATE_LENGTH;
extern bool fflag[10];
extern int rres[10];
extern int numOfModel;
extern int totalExe;
extern bool isAbstract;


using namespace std;

namespace artemis
{

/**
 * This is the main-loop used by artemis.
 *
 * startAnalysis -> preConcreteExecution -> postConcreteExecution -> finishAnalysis
 *                              ^------------------|
 */
Runtime::Runtime(QObject* parent, const Options& options, QUrl url) : QObject(parent)
{

    mOptions = options;

    /** Proxy support **/

    if (!options.useProxy.isNull()) {
        QStringList parts = options.useProxy.split(QString(":"));
        QNetworkProxy proxy(QNetworkProxy::HttpProxy, parts.at(0), parts.at(1).toShort());
        QNetworkProxy::setApplicationProxy(proxy);
    }

    /** Ajax support and cookie injection **/

    AjaxRequestListener* ajaxRequestListner = new AjaxRequestListener(NULL);

    ImmutableCookieJar* immutableCookieJar = new ImmutableCookieJar(
        options.presetCookies, url.host());
    ajaxRequestListner->setCookieJar(immutableCookieJar);

    /** JQuery support **/

    JQueryListener* jqueryListener = new JQueryListener(this);

    /** Runtime Objects **/

    mAppmodel = AppModelPtr(new AppModel(options.coverageIgnoreUrls));

    mWebkitExecutor = new WebKitExecutor(this, mAppmodel, options.presetFormfields, jqueryListener, ajaxRequestListner);

    QSharedPointer<FormInputGenerator> formInputGenerator;
    switch (options.formInputGenerationStrategy) {
    case Random:
        formInputGenerator = QSharedPointer<StaticFormInputGenerator>(new StaticFormInputGenerator());
        break;

    case ConstantString:
        formInputGenerator = QSharedPointer<ConstantStringFormInputGenerator>(new ConstantStringFormInputGenerator());
        break;

    default:
        assert(false);
    }

    mInputgenerator = new RandomInputGenerator(this,
                                               formInputGenerator,
                                               QSharedPointer<StaticEventParameterGenerator>(new StaticEventParameterGenerator()),
                                               new TargetGenerator(this, jqueryListener),
                                               options.numberSameLength);
    mTerminationStrategy = new NumberOfIterationsTermination(this, options.iterationLimit);

    switch (options.prioritizerStrategy) {
    case CONSTANT:
        mPrioritizerStrategy = PrioritizerStrategyPtr(new ConstantPrioritizer());
        break;
    case RANDOM:
        mPrioritizerStrategy = PrioritizerStrategyPtr(new RandomPrioritizer());
        break;
    case COVERAGE:
        mPrioritizerStrategy = PrioritizerStrategyPtr(new CoveragePrioritizer());
        break;
    case READWRITE:
        mPrioritizerStrategy = PrioritizerStrategyPtr(new ReadWritePrioritizer());
        break;
    case  ALL_STRATEGIES:{
        CollectedPrioritizer* strategy = new CollectedPrioritizer();
        strategy->addPrioritizer(new ConstantPrioritizer());
        strategy->addPrioritizer(new CoveragePrioritizer());
        strategy->addPrioritizer(new ReadWritePrioritizer());
        mPrioritizerStrategy = PrioritizerStrategyPtr(strategy);}
        break;
    default:
        assert(false);
    }

    mWorklist = WorkListPtr(new DeterministicWorkList(mPrioritizerStrategy));

    QObject::connect(mWebkitExecutor, SIGNAL(sigExecutedSequence(ExecutableConfigurationConstPtr, QSharedPointer<ExecutionResult>)),
                     this, SLOT(postConcreteExecution(ExecutableConfigurationConstPtr, QSharedPointer<ExecutionResult>)));
    QObject::connect(mWebkitExecutor, SIGNAL(sigAbortedExecution(QString)),
                     this, SLOT(slAbortedExecution(QString)));


    /** Visited states **/
    mVisitedStates = new set<long>();
}

/**
 * @brief Start the analysis for url
 * @param url
 */
void Runtime::startAnalysis(QUrl url)
{
    totalExe = 0;
    std::cout << "modelGeneration: " << modelGeneration << std::endl;
    std::cout << "search depth: " << modelIteration << std::endl;
    std::cout << "isAbstraction: " << isAbstract << std::endl;
    std::cout << "numOfModel: " << numOfModel << std::endl;
    std::cout << "testType: " << testType << std::endl;

    startTime = QDateTime::currentDateTime();
    Log::info("Artemis - Automated tester for JavaScript");
//    Log::info("Started: " + QDateTime::currentDateTime().toString().toStdString());
    Log::info("Started: " + startTime.toString().toStdString());
    Log::info("Compilation date: " + ((string) EXE_BUILD_DATE));
    Log::info( "-----\n");

    QSharedPointer<ExecutableConfiguration> initialConfiguration =
        QSharedPointer<ExecutableConfiguration>(new ExecutableConfiguration(QSharedPointer<InputSequence>(new InputSequence()), url));

    mWorklist->add(initialConfiguration, mAppmodel);

    preConcreteExecution();
}

/**
 * @brief Pre-concrete-execution
 */
void Runtime::preConcreteExecution()
{
    cout << "=======preConcreteExecution======" <<endl;
    struct timeval currentT;
    double ms;
    gettimeofday(&currentT, NULL);
    ms = (currentT.tv_sec - start.tv_sec) * 1000.0f + (currentT.tv_usec - start.tv_usec) / 1000.0f;
    // //cout << "current time cost: " << ms << endl;
    double ts = currentT.tv_sec - start.tv_sec;
    for(int i = 0; i < 10; i++) {
        if(ts > 60 * (i + 1) && fflag[i] == 0) {
            rres[i] = mAppmodel->getCoverageListener()->getNumCoveredLines();
            fflag[i] = 1;
        }
    }

    //termination condition
    int lines = mAppmodel->getCoverageListener()->getNumCoveredLines();

    
    if (mWorklist->empty() ||
        mTerminationStrategy->shouldTerminate()) {

        std::cout << "totalExe: " << totalExe << std::endl;
        mWebkitExecutor->detach();
        finishAnalysis();
        return;
    } 

    if (mWorklist->empty()) {
        cout << "cost: " << ms << " lines: " << lines << "\n";
        std::cout << "totalExe: " << totalExe << std::endl;
        mWebkitExecutor->detach();
        finishAnalysis();
        for(int i = 0; i < 10; i++)
            cout << "fcoverage: " << rres[i] << endl;
        return;
    } 
    

    if(modelGeneration == true && originalModel.size() >= modelIteration * numOfModel) { 

       struct timeval modelTime;
        double mt;
        gettimeofday(&modelTime, NULL);
        mt = (modelTime.tv_sec - start.tv_sec) + (modelTime.tv_usec - start.tv_usec) / 1000000.0f;
        cout << "model construction cost: " << mt << endl;

        

       
        modelGeneration = false;


        this->model->persistModel(originalModel);

        this->model = new Model(originalModel);
        this->model->buildModel();
        this->model->printModel();

        cout << "model print finished" << endl;

        this->testsuitsManager = new TestsuitsManager(this->model);

        
        testsuitsManager->generateTestsuit(testType);
        testsuit = testsuitsManager->getTestsuit();

        cout << "model start state: " << model->getStartState().toStdString() << endl;
        cout << "model transitions num: " << model->getTransitions().size() << endl;
        cout << "model states num: " << model->getNumOfState() << endl;
        cout << "handlers size: " << handlersSet.size() << endl;
        
        cout << "testsuit size: " << this->testsuit->size() << endl;

        cout << "model construction covered: " << mAppmodel->getCoverageListener()->getNumCoveredLines() << endl;

    } else if(modelGeneration == false && originalModel.size() < modelIteration){

        std::cout << "YYYYYYY" << std::endl;
        
        Model::loadModel();

        std::cout << originalModel.size() << std::endl;
        


        this->model = new Model(originalModel);
        this->model->buildModel();
        this->testsuitsManager = new TestsuitsManager(this->model);

        testsuitsManager->generateTestsuit(testType);
        testsuit = testsuitsManager->getTestsuit();
        cout << "testsuit size: " << this->testsuit->size() << endl;
        std::cout << testsuit->size() << std::endl;
        mTerminationStrategy->setmIterationLimit(mTerminationStrategy->getIteration() + this->testsuit->length());
    }

       
    

    Log::debug("\n============= New-Iteration =============");
    Log::debug("--------------- WORKLIST ----------------\n");
    Log::debug(mWorklist->toString().toStdString());
    Log::debug("--------------- COVERAGE ----------------\n");
    Log::debug(mAppmodel->getCoverageListener()->toString().toStdString());

    ExecutableConfigurationConstPtr nextConfiguration = mWorklist->remove();

    QStringList testcase;
    if(modelGeneration == false && this->testsuit->size() > 0) {
        testcase = this->testsuit->first();
        std::cout << testcase.size() << std::endl;
        std::cout << this->testsuit->size() << std::endl;
        this->testsuit->pop_front();
    }
    cout << "=======Execution======" <<endl;
    cout << nextConfiguration->toString().toStdString();
    
    mWebkitExecutor->executeSequence(nextConfiguration, mInputgenerator, testcase); 
    
}

/**
 * @brief Post-concrete-execution
 * @param configuration
 * @param result
 */
void Runtime::postConcreteExecution(ExecutableConfigurationConstPtr configuration, QSharedPointer<ExecutionResult> result)
{
    

    cout << "=======postConcreteExecution======" <<endl;
    originalModel << (configuration->toString() + QString::number(result->getPageStateHash()));
     string temp = configuration->toString().toStdString() + QString::number(result->getPageStateHash()).toStdString();
    
    long hash;

   
    hash = result->getPageStateHash();
    qDebug() << "getPageStateHash" << hash;
    QList<EventHandlerDescriptor*> handlers = result->getEventHandlers();

    if (mOptions.disableStateCheck ||
            mVisitedStates->find(hash = result->getPageStateHash()) == mVisitedStates->end()) {
        qDebug() << "Visiting new state";

        mVisitedStates->insert(hash);
        qDebug() << "djfkdjkfj" << mVisitedStates->size();
        
        mWorklist->add(configuration, mAppmodel);

    } else {
        qDebug() << "Page state has already been seen";
    }

    if(modelGeneration == false && (this->testsuit)->size() == 0) {
        QDateTime currentTime = QDateTime::currentDateTime();
        int t = startTime.secsTo(currentTime);
        std::cout << "Stop time: " << t << "\n";
       

        startTime = currentTime;


        MAX_STATE_LENGTH = MAX_STATE_LENGTH + 1;
        testsuitsManager->generateTestsuit(testType);
        this->testsuit = testsuitsManager->getTestsuit();
    }

    preConcreteExecution();
}

void Runtime::finishAnalysis()
{
    Log::info("Artemis: Testing done...");

    switch (mOptions.outputCoverage) {
    case HTML:
        writeCoverageHtml(mAppmodel->getCoverageListener());
        break;
    case STDOUT:
         writeCoverageStdout(mAppmodel->getCoverageListener());
         break;
    default:
        break;
    }

    statistics()->accumulate("WebKit::coverage::covered-unique", mAppmodel->getCoverageListener()->getNumCoveredLines());

    Log::info("\n=== Statistics ===\n");
    StatsPrettyWriter::write(statistics());
    Log::info("\n=== Statistics END ===\n\n");
    QDateTime endTime = QDateTime::currentDateTime();
    Log::info("Artemis terminated on: "+ endTime.toString().toStdString());
    int a = startTime.secsTo(endTime);
    cout << "time:" << a << endl;

    emit sigTestingDone();
}

void Runtime::slAbortedExecution(QString reason)
{
    cerr << reason.toStdString() << std::endl;
    emit sigTestingDone();
}

} /* namespace artemis */
