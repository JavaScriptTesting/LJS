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
#include <iostream>
#include <unistd.h>

#include <QtWebKit>
#include <QApplication>
#include <QStack>
#include <QDebug>
#include <qwebexecutionlistener.h>
#include <instrumentation/executionlistener.h>

#include "runtime/input/forms/formfield.h"
#include "runtime/input/events/domelementdescriptor.h"
#include "strategies/inputgenerator/targets/jquerylistener.h"
#include "runtime/input/baseinput.h"
#include "strategies/inputgenerator/randominputgenerator.h"
#include "util/fileutil.h"
#include <sys/time.h>

#include "webkitexecutor.h"
extern bool modelGeneration;
extern int modelIteration;
extern QStringList originalModel;
extern QSet<QString> handlersSet;
extern QString subjectName;
extern QMap<QString, QStringList* > depMap;
extern bool isAbstract;
extern struct timeval start;
extern bool randomOrWeight;
extern int totalExe;



using namespace std;

namespace artemis
{


WebKitExecutor::WebKitExecutor(QObject* parent,
                               AppModelPtr appmodel,
                               QMap<QString, QString> presetFields,
                               JQueryListener* jqueryListener,
                               AjaxRequestListener* ajaxListener) :
    QObject(parent)
{


    mPresetFields = presetFields;
    mJquery = jqueryListener;

    mAjaxListener = ajaxListener;
    mAjaxListener->setParent(this);

    mPage = ArtemisWebPagePtr(new ArtemisWebPage());
    mPage->setNetworkAccessManager(mAjaxListener);

    QObject::connect(mPage.data(), SIGNAL(loadFinished(bool)),
                     this, SLOT(slLoadFinished(bool)));

    mResultBuilder = ExecutionResultBuilderPtr(new ExecutionResultBuilder(mPage));

    mCoverageListener = appmodel->getCoverageListener();
    mJavascriptStatistics = appmodel->getJavascriptStatistics();

    QWebExecutionListener::attachListeners();
    webkitListener = QWebExecutionListener::getListener();

    // TODO cleanup in ajax stuff, we are handling ajax through AjaxRequestListener, the ajaxRequest signal and addAjaxCallHandler

    QObject::connect(webkitListener, SIGNAL(jqueryEventAdded(QString, QString, QString)),
                     mJquery, SLOT(slEventAdded(QString, QString, QString)));

    QObject::connect(webkitListener, SIGNAL(loadedJavaScript(QString, QUrl, uint)),
                     mCoverageListener.data(), SLOT(slJavascriptScriptParsed(QString, QUrl, uint)));
    QObject::connect(webkitListener, SIGNAL(statementExecuted(uint, QUrl, uint)),
                     mCoverageListener.data(), SLOT(slJavascriptStatementExecuted(uint, QUrl, uint)));
    QObject::connect(webkitListener, SIGNAL(sigJavascriptBytecodeExecuted(uint,  uint, QUrl, uint)),
                     mCoverageListener.data(), SLOT(slJavascriptBytecodeExecuted(uint, uint, QUrl, uint)));
    QObject::connect(webkitListener, SIGNAL(sigJavascriptFunctionCalled(QString, size_t, uint, QUrl, uint)),
                     mCoverageListener.data(), SLOT(slJavascriptFunctionCalled(QString, size_t, uint, QUrl, uint)));

    QObject::connect(webkitListener, SIGNAL(sigJavascriptPropertyRead(QString,intptr_t,intptr_t,QUrl,int)),
                     mJavascriptStatistics.data(), SLOT(slJavascriptPropertyRead(QString,intptr_t,intptr_t,QUrl,int)));
    QObject::connect(webkitListener, SIGNAL(sigJavascriptPropertyWritten(QString,intptr_t,intptr_t,QUrl,int)),
                     mJavascriptStatistics.data(), SLOT(slJavascriptPropertyWritten(QString,intptr_t,intptr_t,QUrl,int)));

    QObject::connect(webkitListener, SIGNAL(addedEventListener(QWebElement*, QString)),
                     mResultBuilder.data(), SLOT(slEventListenerAdded(QWebElement*, QString)));
    QObject::connect(webkitListener, SIGNAL(removedEventListener(QWebElement*, QString)),
                     mResultBuilder.data(), SLOT(slEventListenerRemoved(QWebElement*, QString)));

    QObject::connect(webkitListener, SIGNAL(addedTimer(int, int, bool)),
                     mResultBuilder.data(), SLOT(slTimerAdded(int, int, bool)));
    QObject::connect(webkitListener, SIGNAL(removedTimer(int)),
                     mResultBuilder.data(), SLOT(slTimerRemoved(int)));

    QObject::connect(webkitListener, SIGNAL(script_crash(QString, intptr_t, int)),
                     mResultBuilder.data(), SLOT(slScriptCrashed(QString, intptr_t, int)));
    QObject::connect(webkitListener, SIGNAL(eval_call(QString)),
                     mResultBuilder.data(), SLOT(slStringEvaled(QString)));

    QObject::connect(webkitListener, SIGNAL(addedAjaxCallbackHandler(int)),
                     mResultBuilder.data(), SLOT(slAjaxCallbackHandlerAdded(int)));
    QObject::connect(webkitListener, SIGNAL(ajax_request(QUrl, QString)),
                     mResultBuilder.data(), SLOT(slAjaxRequestInitiated(QUrl, QString)));

    QObject::connect(webkitListener, SIGNAL(sigJavascriptConstantEncountered(QString)),
                     mResultBuilder.data(), SLOT(slJavascriptConstantEncountered(QString)));

}

WebKitExecutor::~WebKitExecutor()
{
}

void WebKitExecutor::detach() {
    webkitListener->disconnect(mResultBuilder.data());

}

void WebKitExecutor::executeSequence(ExecutableConfigurationConstPtr conf, InputGeneratorStrategy* mInputgenerator, QStringList testcase)
{
    currentConf = conf;
    currentGenerator = mInputgenerator;
    currentTestcase = testcase;

    mJquery->reset(); // TODO merge into result?
    mResultBuilder->reset();

    qDebug() << "--------------- FETCH PAGE --------------" << endl;

    mCoverageListener->notifyStartingLoad();
    mResultBuilder->notifyStartingLoad();
    mJavascriptStatistics->notifyStartingLoad();

    mPage->mainFrame()->load(conf->getUrl());
}

void WebKitExecutor::slLoadFinished(bool ok)
{
    mResultBuilder->notifyPageLoaded();

    if (!ok) {
        emit sigAbortedExecution(QString("Error: The requested URL ") + currentConf->getUrl().toString() + QString(" could not be loaded"));
        return;
    }

    foreach(QString f , mPresetFields.keys()) {
        QWebElement elm = mPage->mainFrame()->findFirstElement(f);

        if (elm.isNull()) {
            continue;
        }

        elm.setAttribute("value", mPresetFields[f]);
    }


    qDebug() << "\n------------ EXECUTE SEQUENCE -----------" << endl;

        
    if(modelGeneration == true) {

        QMapIterator<QString, QStringList*> iter(depMap);
        if(!iter.hasNext()) {
            std::cout << "!!!!!!!!!!!!!!!!!!!!No dependent relations" << std::endl; 

            std::cout << "Reading dependency from file" << std::endl;
            QString depFilePath = QString("../../raw-data/") + subjectName + QString("/info/dep.txt");
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

            QMapIterator<QString, QStringList*> iter(depMap);
            if(!iter.hasNext())
                std::cout << "still has no dependent relations" << std::endl; 


        } else{
            std::cout << "Already Read successfully" << std::endl; 

        }

        

        std::cout << "generating model..." << std::endl;

       

     
        if(isAbstract) {
            QSet<QSharedPointer<const FormField> > formfields = mResultBuilder->getResult()->getFormFields();

            foreach(QSharedPointer<const FormField> formField, formfields) {
                const DOMElementDescriptor* elmDesc = formField->getDomElement();
                QWebElement element = elmDesc->getElement(this->mPage);
                if (!element.isNull()) {
                    element.setAttribute("value", "gpf");
                }
            }
        }
        originalModel << QString::number(mResultBuilder->getResult()->getPageStateHash());
        int i = 1;
        QSharedPointer<const BaseInput> input;

        if(randomOrWeight) {
            std::cout << "Using random" << std::endl;
        } else {
            std::cout << "Using weighted" << std::endl;
        }

        std::cout << "modelIteration: " << modelIteration << std::endl;

        while(i < modelIteration) {
            
            input = ((RandomInputGenerator* )currentGenerator)->insertOne(currentConf, mResultBuilder->getResult(), &depMap, &numsOfEventExecution);
            
            QString inputName = input->toString();
            if(numsOfEventExecution.contains(inputName)) {
                numsOfEventExecution[inputName] += 1;
            } else {
                numsOfEventExecution[inputName] = 1;
            }
           

            mResultBuilder->notifyStartingEvent();
            mCoverageListener->notifyStartingEvent(input);
            mJavascriptStatistics->notifyStartingEvent(input);
            input->apply(this->mPage, this->webkitListener);


         


            if(isAbstract) {
                QSet<QSharedPointer<const FormField> > formfields = mResultBuilder->getResult()->getFormFields();

                foreach(QSharedPointer<const FormField> formField, formfields) {
                    const DOMElementDescriptor* elmDesc = formField->getDomElement();
                    QWebElement element = elmDesc->getElement(this->mPage);
                    if (!element.isNull()) {
                        // element.setAttribute("value", input.second->stringRepresentation());
                        // std::cout << "value: " << element.attribute("value").toStdString() << std::endl;
                        element.setAttribute("value", "gpf");
                    }
                }
            }


            
            

            originalModel << (input->toString() + QString(" ") + QString::number(mResultBuilder->getResult()->getPageStateHash()));
            i++;
            foreach (EventHandlerDescriptor* ee, mResultBuilder->getResult()->getEventHandlers()) {
                handlersSet.insert(ee->name() + "@" + ee->domElement()->toString());
            }
        }

       
        QMapIterator<QString, int> iter1(numsOfEventExecution);
        while(iter1.hasNext()) {
            iter1.next();
            std::cout << iter1.key().toStdString() <<": " << iter1.value() << std::endl;
        }


    } 
    else {
        totalExe += 1;
        int length = currentTestcase.size();
        QString temp;
        temp = currentTestcase.join(" ");
        int i = 0;

        struct timeval startT;
        double s;
        gettimeofday(&startT, NULL);
        
        QSharedPointer<const BaseInput> input;
        while(length > i) {
            input = ((RandomInputGenerator* )currentGenerator)->insertAEvent(currentConf, mResultBuilder->getResult(), currentTestcase[i]);
            mResultBuilder->notifyStartingEvent();
            mCoverageListener->notifyStartingEvent(input);
            mJavascriptStatistics->notifyStartingEvent(input);
            input->apply(this->mPage, this->webkitListener);
            i++;
        }


        struct timeval endT;
        gettimeofday(&endT, NULL);

        s = endT.tv_sec - startT.tv_sec + (endT.tv_usec - startT.tv_usec) / 1000000.0f;
        cout << "testcase running time: " << s << endl;
    }


    emit sigExecutedSequence(currentConf, mResultBuilder->getResult());
}

}
