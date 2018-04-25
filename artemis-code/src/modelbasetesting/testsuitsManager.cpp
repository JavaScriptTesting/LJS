#include "testsuitsManager.h"
#include "assert.h"
#include <iostream>
#include <QQueue>


extern QMap<QString, QStringList* > depMap;
extern QString subjectName;

namespace artemis {
    TestsuitsManager::TestsuitsManager(Model* model){
        this->model = model;
    }

    void TestsuitsManager::generateTestsuit(int type) {
        QList<QStringList> redundantTestcases;
        std::cout << "generating test suit" << std::endl;
        if(type == 0) {
            while(testsuit.size() < MAX_TEST_SUIT_SIZE) {
                QString currentState = model->getStartState();
                TransitionItem* item; 
                QStringList sequence;
                QString endState = model->getEndState();
                while(sequence.size() < MAX_TEST_SEQUENCE_LENGTH && (currentState != endState || model->hasNextEvent(currentState))) {
                    item = model->getNextEventByRandom(currentState);
                    if(!item)
                        std::cout << "NULL";
                    if(item == NULL) break;
                    if(item->getEvent() == NULL || item->getTo() == NULL)
                        std::cout << "Boom!!!!!!!!!!!!!!!!!!!!!!!!!!";
                    sequence.append(item->getEvent());
                    currentState = item->getTo();
                    
                }
             
                testsuit.append(sequence);
            }
            std::cout << "generating new testsuit and testsuit size: " << testsuit.size() << std::endl;
        } else if(type == 1) {
            std::cout << "Using BFS testsuit generation " << std::endl;
            QMap<QString, QSet<TransitionItem>* > adjoinSet = model->getAdjoinSet();
            QString start_state = model->getStartState();

            QList<QList<TransitionItem> > queue;
            QSet<TransitionItem>* startSet = adjoinSet[start_state];

            std::cout << "startSet.size() " << startSet->size() << std::endl;
            foreach(const TransitionItem& item, *startSet) {
                QList<TransitionItem> qlist;
                TransitionItem i = item;
                std::cout << "gpf: " << i.toString().toStdString() << std::endl;
                qlist.append(item);
                std::cout << "size: " << qlist.size() << std::endl;
                queue.append(qlist);
            }

            int current_testcase_length = 1;

            while(current_testcase_length < MAX_TEST_SEQUENCE_LENGTH) {
                std::cout << "queeu.isEmpty()" << queue.isEmpty() << std::endl;
                QList<QList<TransitionItem> > queue2;
                while(!queue.isEmpty()) {
                    QList<TransitionItem> head = queue.takeFirst();
                    std::cout << "head.length()!!!!! " << head.size() << std::endl;
                    std::cout << head.at(0).getTo().toStdString() << std::endl;
                    std::cout << "head. " << head.first().toString().toStdString() << std::endl;
                    std::cout << "queue.length() after " << queue.size() << std::endl;

                    QString last_state = head.last().getTo();
                    std::cout << "last_state" << last_state.toStdString() << std::endl;



                    foreach(const TransitionItem& item, *(adjoinSet[last_state])) {
                        QList<TransitionItem> temp = head;
                        temp.append(item);
                        std::cout << "for head lengh: " << temp.size() << std::endl;
                        queue2.append(temp);
                    }

                }
                queue = queue2;



                current_testcase_length++;

                QList<QList<TransitionItem> > redundant;
                if(current_testcase_length <= 2) continue;
                else {
                    for(int i = 0; i < queue.size(); i++) {
                        int l = queue.at(i).size();
                        QList<TransitionItem> pi = queue.at(i).mid(0, l - 2);
                        for(int j = i + 1; j < queue.size(); j++) {
                            if(redundant.contains(queue.at(j)))
                                continue;
                            QList<TransitionItem> pj = queue.at(j).mid(0, l - 2);
                            if(pi != pj) {
                                continue;
                            }
                            else if(!(queue.at(i).at(l - 1).getEvent() == queue.at(j).at(l - 2).getEvent() && queue.at(i).at(l - 2).getEvent() == queue.at(j).at(l - 1).getEvent())){
                                continue;
                            } else if(!(queue.at(i).at(l - 1).getTo() == queue.at(j).at(l - 1).getTo())) {
                                continue;
                            } 
                            else if(!isDc(queue.at(i).at(l - 1).getEvent(), queue.at(i).at(l - 2).getEvent())) {
                                redundant.append(queue.at(j));
                            }
                        }
                    }
                }
                std::cout << "redundant numbers: " << redundant.size() << std::endl;
                
            }


            foreach(QList<TransitionItem> list, queue) {
                QStringList sequence;
                foreach(TransitionItem item, list) {
                    sequence.append(item.getEvent());
                }
                testsuit.append(sequence);

            }








        } else if(type == 2) {
            std::cout << "Using dependency relations: " << std::endl;
            int count = 0;
            int redundantTestcase = 0;
            while(testsuit.size() < MAX_TEST_SUIT_SIZE && count < 3 * MAX_TEST_SUIT_SIZE) {
                count++;
                if(testsuit.size() == 0) {
                    QString currentState = model->getStartState();
                    TransitionItem* item; 
                    QStringList sequence;
                    QString endState = model->getEndState();
                    while(sequence.size() < MAX_TEST_SEQUENCE_LENGTH && (currentState != endState || model->hasNextEvent(currentState))) {
                        item = model->getNextEventByRandom(currentState);
                        sequence.append(item->getEvent());
                        currentState = item->getTo();
                       
                    }
                    testsuit.append(sequence);
                } else {
                    QString currentState = model->getStartState();
                    TransitionItem* item; 
                    QStringList sequence;
                    QString endState = model->getEndState();
                    while(sequence.size() < MAX_TEST_SEQUENCE_LENGTH && (currentState != endState || model->hasNextEvent(currentState))) {
                        item = model->getNextEventByRandom(currentState);
                        sequence.append(item->getEvent());
                        currentState = item->getTo();
                    }

                    std::cout << sequence.size() << std::endl;
                    QString newEvent1 = sequence[0];
                    QString newEvent2 = sequence[1];
                    bool flag = false;
                    for(int i = 0; i < testsuit.size(); i++) {
                        assert(testsuit[i][0] != NULL);
                        assert(testsuit[i][1] != NULL);
                        if(isEquivalent(testsuit[i][0], testsuit[i][1], newEvent1, newEvent2)) {
                            flag = true;
                            break;
                        }
                    }
                    if(flag) {
                        redundantTestcase++;
                        redundantTestcases.append(sequence);
                        continue;
                    } else {
                        testsuit.append(sequence);
                    }
                    std::cout << "testsuit length: " << testsuit.size() << std::endl;


                }



            }
            std::cout << "generated a new testsuit which has " << testsuit.size() << " testcases in total, and " << redundantTestcase << " redundant testcases" << std::endl;
               
        } 
        else if(type == 3) {
            if(depMap.isEmpty())
                TestsuitsManager::readDependencyFromFile();

            if(depMap.isEmpty()) {
                std::cout << "Waring: No depMap !!!!!!!!!!!!!!!!!!" << std::endl;
                return;
            }
            
            QList<State* > stateStack;
            QString startStateString = model->getStartState();
            State* startState = new State(startStateString);
            std::cout << "s0: " << startStateString.toStdString() << std::endl;
            stateStack.append(startState);
            explore(&stateStack);

        } 
        else if(type == 4) {
            if(depMap.isEmpty()) {
                TestsuitsManager::readDependencyFromFile();
                if(depMap.isEmpty()) {
                    std::cout << "Waring: No depMap !!!!!!" << std::endl;
                    return; 
                }
            }

            QList<State* > stateStack;
            QString startStateString = model->getStartState();
            State* startState = new State(startStateString);
            std::cout << "s0: " << startStateString.toStdString() << std::endl;
            stateStack.append(startState);
            explore(&stateStack);


        }     
    }

    void TestsuitsManager::explore(QList<State* >* S) {
        State* s = S->last();

        if(S->size() <= MAX_STATE_LENGTH) {
            QSet<QString> done;
            s->setDone(done);
            QSet<QString> sleep;
            s->setSleep(sleep);
            

            QSet<QString> temp = model->getEnabled(s->getStringState()) - s->getDone() - s->getSleep();

            QSetIterator<QString> i(temp);
            while(i.hasNext()) {
                QString event = i.next();
                s->setSelEV(event);
                s->addToDone(event); 

                QString nextState = model->getNextState(s->getStringState() ,event);
                State* state = new State(nextState);
                state->setSleep(calSleep(*s, event));

               

                S->append(state);
                explore(S);

                temp = model->getEnabled(s->getStringState()) - s->getDone() - s->getSleep();
                QSetIterator<QString> i(temp);
                s->addSleep(event);
            }
        }
        if(s->getSelEV() == NULL) {
            QString sequence;
            QStringList sequenceList;
            for(int j = 0; j < S->size(); j++) {
                sequenceList.append(S->at(j)->getSelEV());
                sequence += S->at(j)->getSelEV() + ",";
            }
            

            testsuit.append(sequenceList);

        }
        S->removeLast();
      


    }

    QSet<QString> TestsuitsManager::calSleep(State& s, QString event) {

        QSet<QString> sleep;
        foreach(const QString& e, s.getSleep()) {
            if(!isDc(e, event)) { 
                sleep.insert(e);
            }
        }

        return sleep;

    }

    QList<QStringList>* TestsuitsManager::getTestsuit() {
        return &testsuit;
    }

    bool TestsuitsManager::redundantSequence(QList<State* >* S, State* s) {
        bool causalIndep = true;
        foreach(State* state, *S) {
            if(isDc(state->getSelEV(), s->getSelEV())) {
                causalIndep = false;
                break;
            }
        }
        if(!s->getPrinted().contains(s->getSelEV()) && causalIndep){
            return true;
        } else{
            foreach(State* state, *S) {
                s->addPrinted(state->getSelEV());
            }
            return false;
        }
    }

    bool TestsuitsManager::isEquivalent(QString existEvent1, QString existEvent2, QString newEvent1, QString newEvent2) {
        
        if(existEvent1 == newEvent1 && existEvent2 == newEvent2) {
            return true;
        } 
        if(existEvent1 == newEvent2 && existEvent2 == newEvent1 && depMap.keys().contains(existEvent2) && !(depMap[existEvent2]->contains(existEvent1))) {
            return true;
        }     
        return false;
    }

    bool TestsuitsManager::isDc(QString event1, QString event2) {
        if((depMap.keys().contains(event1) && depMap[event1]->contains(event2)) || (depMap.keys().contains(event2) && depMap[event2]->contains(event1)) )
            return true;
        else
            return false;
    }

    
}
