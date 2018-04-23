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
#include <typeinfo>
#include <QDebug>
#include <iostream>

#include "runtime/input/events/eventypes.h"
#include "runtime/input/forms/forminput.h"
#include "runtime/input/baseinput.h"
#include "runtime/input/dominput.h"
#include "runtime/input/timerinput.h"
#include "runtime/input/ajaxinput.h"

#include "randominputgenerator.h"
extern QSet<QString> handlersSet;
extern bool randomOrWeight;
extern QString lastEvent;
extern double parameterD;

namespace artemis
{

RandomInputGenerator::RandomInputGenerator(
        QObject* parent,
        QSharedPointer<const FormInputGenerator> formInputGenerator,
        QSharedPointer<const EventParameterGenerator> eventParameterInputGenerator,
        TargetGenerator* targetGenerator,
        int numberSameLength) :

    InputGeneratorStrategy(parent),
    mFormInputGenerator(formInputGenerator),
    mEventParameterGenerator(eventParameterInputGenerator)
{
    mTargetGenerator = targetGenerator;
    mTargetGenerator->setParent(this);

    mNumberSameLength = numberSameLength;

}

QList<QSharedPointer<ExecutableConfiguration> > RandomInputGenerator::addNewConfigurations(
        QSharedPointer<const ExecutableConfiguration> configuration,
        QSharedPointer<const ExecutionResult> result)
{

    QList<QSharedPointer<ExecutableConfiguration> > newConfigurations;

        newConfigurations.append(insertSameLength(configuration, result));
        newConfigurations.append(insertExtended(configuration, result));

    return newConfigurations;
}

//gpf
/**
1. generate testsuit(20 * 30)(event * sequence)
2. run for result
**/

// QList<QSharedPointer<ExecutableConfiguration> > RandomInputGenerator::insertModelTestSuit(
//         QSharedPointer<const ExecutableConfiguration> oldConfiguration,
//         QSharedPointer<const ExecutionResult> result)
// {

//     //read testsuit
//     QFile file("/home/gpf/test.txt");
//     if(!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
//         qDebug() << "Can't open the file." << endl;
//     }
//     QTextStream in(&file);
//     QList<QStringList>> testsuit;
//     while(!in.atEnd()) {
//         QString line = in.readLine();
//         testsuit.append(line.split(" "));
//     }



    
//     QSharedPointer<const InputSequence> sequence = oldConfiguration->getInputSequence();

//     int numberNewEvents = result->getEventHandlers().length();
//     int randomNum = (rand() % (numberNewEvents));
//         EventHandlerDescriptor* ee = result->getEventHandlers().at(randomNum);
//     EventParameters* newParams = mEventParameterGenerator->generateEventParameters(NULL, ee);
//     TargetDescriptor* target = mTargetGenerator->generateTarget(NULL, ee);
//     QSharedPointer<FormInput> newForm = mFormInputGenerator->generateFormFields(NULL, result->getFormFields(), result);
//     QSharedPointer<const DomInput> domInput = QSharedPointer<const DomInput>(new DomInput(ee, newForm, newParams, target));

   
//     QSharedPointer<const InputSequence> newInputSequence;
//     if(sequence->isEmpty()) {
//         newInputSequence = sequence->extend(domInput);
//     } else {
//         newInputSequence = sequence->replaceLast(domInput);
//     }

//     QSharedPointer<ExecutableConfiguration> newConfiguration = QSharedPointer<ExecutableConfiguration>(new ExecutableConfiguration(newInputSequence, oldConfiguration->getUrl()));

//     newConfigurations.append(newConfiguration);

//     return newConfigurations;
// }

//gpf
 QSharedPointer<const BaseInput> RandomInputGenerator::insertOne(
        QSharedPointer<const ExecutableConfiguration> oldConfiguration,
        QSharedPointer<const ExecutionResult> result, QMap<QString, QStringList*>* depMap, QMap<QString, int>* eventNums)
{
    EventHandlerDescriptor* ee;

    // std::cout << randomOrWeight << lastEvent.toStdString() << std::endl;
    if(randomOrWeight) {
        //std::cout << "Using Random model generation..." << std::endl; 
        int numberNewEvents = result->getEventHandlers().length();
        int randomNum = (rand() % (numberNewEvents));
        ee = result->getEventHandlers().at(randomNum);
        lastEvent = ee->name() + QString("@") + ee->domElement()->toString();
//        std::cout << "lastEvent: " + lastEvent.toStdString()<< std::endl; 
    } else {
        // choose next event according to weight
        // std::cout << "Using weighted model generation... and the parameterD is " << parameterD << std::endl; 
        double maxWeight = 0.0;
        int index = 0;
        QList<EventHandlerDescriptor*> descriptorList = result->getEventHandlers();
        // std::cout << descriptorList.size() << std::endl; 
        for(int i = 0; i < descriptorList.size(); i++) {
            EventHandlerDescriptor* current = descriptorList[i];
            QString des = current->name() + QString("@") + current->domElement()->toString();
            // std::cout << "hahahaha " << des.toStdString() << std::endl;
            double weight;

            if((*depMap).contains(des) && (*depMap)[des]->contains(lastEvent)) {
                weight = parameterD / (1 + (*eventNums)[des]);
            }
            else {
                weight = (1 - parameterD) / (1 + (*eventNums)[des]);
            }
            // std::cout << "weight: " << weight << std::endl;

            if(weight > maxWeight) {
                maxWeight = weight;
                index = i;
            }
        }
        ee = result->getEventHandlers().at(index);
        // std::cout << "ee: " << ee->name().toStdString() << " maxWeight: " << maxWeight << std::endl; 
        lastEvent = ee->name() + QString("@") + ee->domElement()->toString();

    }
    EventParameters* newParams = mEventParameterGenerator->generateEventParameters(NULL, ee);
    TargetDescriptor* target = mTargetGenerator->generateTarget(NULL, ee);
    QSharedPointer<FormInput> newForm = mFormInputGenerator->generateFormFields(NULL, result->getFormFields(), result);
    QSharedPointer<const DomInput> domInput = QSharedPointer<const DomInput>(new DomInput(ee, newForm, newParams, target));

    return domInput;
}


QList<QSharedPointer<ExecutableConfiguration> > RandomInputGenerator::insertSameLength(
        QSharedPointer<const ExecutableConfiguration> oldConfiguration,
        QSharedPointer<const ExecutionResult> result)
{

    

    QList<QSharedPointer<ExecutableConfiguration> > newConfigurations;

    QSharedPointer<const InputSequence> sequence = oldConfiguration->getInputSequence();

    if (sequence->isEmpty()) {
        return newConfigurations;
    }

    QSharedPointer<const BaseInput> last = sequence->getLast();

    for (int i = 0; i < mNumberSameLength; i++) {
        QSharedPointer<const BaseInput> newLast = last->getPermutation(mFormInputGenerator, mEventParameterGenerator, mTargetGenerator, result);

        QSharedPointer<const InputSequence> newSeq = sequence->replaceLast(newLast);
        QSharedPointer<ExecutableConfiguration> newConf = \
                QSharedPointer<ExecutableConfiguration>(new ExecutableConfiguration(newSeq, oldConfiguration->getUrl()));
        qDebug() << newConf->toString();
        newConfigurations.append(newConf);

        /**
         * // TODO
         *
         * The above code will generate duplicates of the same sequence over time, as the variants generator can repeat already
         * used parameters. Is this a good way to extend our test sequences? And if so should we put in some "isEqual"-ish check
         * to avoid duplications?
         */
    }

    return newConfigurations;
}



QSharedPointer<const BaseInput> RandomInputGenerator::insertAEvent(QSharedPointer<const ExecutableConfiguration> oldConfiguration,
        QSharedPointer<const ExecutionResult> result, QString name) {
    qDebug() << "QSTRING " + name;
    foreach (EventHandlerDescriptor* ee, result->getEventHandlers()) {
        qDebug() << "eename " << ee->name() + "@" + ee->domElement()->toString();
        // qDebug() << "eeElement " << ee->domElement()->toString();
        if(name == (ee->name() + "@" + ee->domElement()->toString())) {
            EventParameters* newParams = mEventParameterGenerator->generateEventParameters(NULL, ee);
            TargetDescriptor* target = mTargetGenerator->generateTarget(NULL, ee);
            QSharedPointer<FormInput> newForm = mFormInputGenerator->generateFormFields(NULL, result->getFormFields(), result);
            QSharedPointer<const DomInput> domInput = QSharedPointer<const DomInput>(new DomInput(ee, newForm, newParams, target));
            return domInput;
        }         

    }

    EventHandlerDescriptor* ee = result->getEventHandlers().last();
    EventParameters* newParams = mEventParameterGenerator->generateEventParameters(NULL, ee);
    TargetDescriptor* target = mTargetGenerator->generateTarget(NULL, ee);
    QSharedPointer<FormInput> newForm = mFormInputGenerator->generateFormFields(NULL, result->getFormFields(), result);
    QSharedPointer<const DomInput> domInput = QSharedPointer<const DomInput>(new DomInput(ee, newForm, newParams, target));
    return domInput;

}

QList<QSharedPointer<ExecutableConfiguration> > RandomInputGenerator::insertExtended(
        QSharedPointer<const ExecutableConfiguration> oldConfiguration,
        QSharedPointer<const ExecutionResult> result)
{
    QList<QSharedPointer<ExecutableConfiguration> > newConfigurations;

    foreach (EventHandlerDescriptor* ee, result->getEventHandlers()) {
        EventParameters* newParams = mEventParameterGenerator->generateEventParameters(NULL, ee);
        TargetDescriptor* target = mTargetGenerator->generateTarget(NULL, ee);
        QSharedPointer<FormInput> newForm = mFormInputGenerator->generateFormFields(NULL, result->getFormFields(), result);
        QSharedPointer<const DomInput> domInput = QSharedPointer<const DomInput>(new DomInput(ee, newForm, newParams, target));

        QSharedPointer<const InputSequence> newInputSequence = oldConfiguration->getInputSequence()->extend(domInput);

        QSharedPointer<ExecutableConfiguration> newConfiguration = QSharedPointer<ExecutableConfiguration>(new ExecutableConfiguration(newInputSequence, oldConfiguration->getUrl()));

        newConfigurations.append(newConfiguration);
    }

    foreach (QSharedPointer<const Timer> timer, result->getTimers()) {
        QSharedPointer<const BaseInput> newInput = QSharedPointer<const TimerInput>(new TimerInput(timer));

        QSharedPointer<const InputSequence> newSeq = oldConfiguration->getInputSequence()->extend(newInput);

        QSharedPointer<ExecutableConfiguration> newConf = QSharedPointer<ExecutableConfiguration>(new ExecutableConfiguration(newSeq, oldConfiguration->getUrl()));

        newConfigurations.append(newConf);
    }

    foreach (int callbackId, result->getAjaxCallbackHandlers()) {
        QSharedPointer<const BaseInput> newInput = QSharedPointer<const AjaxInput>(new AjaxInput(callbackId));

        QSharedPointer<const InputSequence> newSequence = oldConfiguration->getInputSequence()->extend(newInput);

        QSharedPointer<ExecutableConfiguration> newConfiguration = QSharedPointer<ExecutableConfiguration>(new ExecutableConfiguration(newSequence,
                oldConfiguration->getUrl()));

        newConfigurations.append(newConfiguration);
    }

    return newConfigurations;
}

}