#include "model.h"
#include "util/fileutil.h"
#include <assert.h>
#include <time.h>
#include <iostream>

extern QString subjectName;
namespace artemis
{

	Model::Model(QStringList originalModel) {
		this->oModel = originalModel;
	}


	QSet<TransitionItem> Model::getTransitions() {
		return this->transitions;
	}

    void Model::printModel() {
    	foreach(TransitionItem t, this->transitions) {
    		std::cout << "T: " << t.toString().toStdString() << "\n";
    	}

    }

	QString Model::getStartState() {

		return this->startState;
	}

	QString Model::getEndState() {
		return this->endState;
	}

	void Model::persistModel(QStringList originalModel) {
		QString filename = "/home/pengfei/guided-artemis/raw-data/" + subjectName + "/info/model.txt";
		std::cout << originalModel.size() << std::endl;

		writeListToFile(filename, originalModel);
	}

	

    QString Model::getNextState(QString from, QString event) {
    	QSet<TransitionItem>* set = adjoinSet[from];
    	foreach(const TransitionItem& item, *set) {
    		if(item.getEvent() == event) {
    			return item.getTo();
    		}
		}

		return NULL;

    }

    QSet<QString> Model::getEnabled(QString state) {
		QSet<TransitionItem>* set = adjoinSet[state];

		QSet<QString> enabled;
		if(!set) {
			return enabled;
		}

    	foreach(const TransitionItem& item, *set) {
    		enabled.insert(item.getEvent());
		}

		return enabled;
    }
	
	int Model::getNumOfState() {
		return numOfState;
	}

	int Model::getSize() {
		return oModel.size();
	}


    QMap<QString, QSet<TransitionItem>* > Model::getAdjoinSet() {
    	return adjoinSet;
    }


    bool Model::hasNextEvent(QString currentState) {
		QSet<TransitionItem>* transitionSet = adjoinSet[currentState];
		if(NULL != transitionSet && transitionSet->size() > 0)
			return true;
		return false;
    }


	void Model::buildModel() {

		this->startState = oModel[0].trimmed();

		QString from;
		QString event;
		QString to;
		QSet<QString> states;
		int modelSize = oModel.size();

		for(int i = 0; i < modelSize; i++) {
			QStringList temp = oModel[i].split(" ");
			if(temp.size() == 1) {
				from = temp[0];
				states.insert(from);
				continue;
			} else if(temp.size() == 2) {

				assert(from.toStdString() != "");
				event = temp[0];
				assert(event.toStdString() != "");
				to = temp[1];
				assert(to.toStdString() != "");
				TransitionItem transitionItem(from, event, to);
				transitions.insert(transitionItem);
				states.insert(to);
				from = to;
			}
			// std::cout << "1111" << std::endl;
			// for(int j = 0; j < temp.size(); j++)
			// 	std::cout << temp[j].toStdString() << std::endl;
			// std::cout << "2222" << std::endl;
			// event = temp[0];
			// assert(event.toStdString() != "");
			// to = temp[1];
			// assert(to.toStdString() != "");
			// TransitionItem transitionItem(from, event, to);
		 //    std::cout << "T: " + transitionItem.toString().toStdString() << std::endl;
			// transitions.insert(transitionItem);
			// states.insert(to);
		}
		// for(int i = 1; i < modelSize; i++) {
		// 	if(i == 1)
		// 		from = startState;
		// 	else
		// 		from = to;
		// 	QStringList temp = oModel[i].split(" ");
		// 	states.insert(from);
		// 	// std::cout << "1111" << std::endl;
		// 	// for(int j = 0; j < temp.size(); j++)
		// 	// 	std::cout << temp[j].toStdString() << std::endl;
		// 	// std::cout << "2222" << std::endl;
		// 	assert(from.toStdString() != "");
		// 	event = temp[0];
		// 	assert(event.toStdString() != "");
		// 	to = temp[1];
		// 	assert(to.toStdString() != "");
		// 	TransitionItem transitionItem(from, event, to);
		//     std::cout << "T: " + transitionItem.toString().toStdString() << std::endl;
		// 	transitions.insert(transitionItem);
		// 	states.insert(to);
		// }

		this->numOfState = states.size();

		// foreach(const TransitionItem& transitionItem, transitions) {
		//   //  std::cout << "T: " + transitionItem.toString().toStdString();
		// }

		this->endState = to;

		foreach(const TransitionItem& item, transitions) {
			QString state = item.getFrom();
			if(adjoinSet.contains(state)) {
				adjoinSet[state]->insert(item);
				//std::cout << "1111 " << adjoinSet.size() << std::endl;
			} else {
				adjoinSet.insert(state, new QSet<TransitionItem>());
				adjoinSet[state]->insert(item);
			}
		}

		

		QMutableMapIterator<QString, QSet<TransitionItem>* > i(adjoinSet);

		QList<QString> keySet = adjoinSet.keys();

		for(int i = 0; i < keySet.size(); i++) {
			QSet<TransitionItem>* temp = adjoinSet[keySet[i]];
			double averageProbability = 1.0 / temp->size();
			foreach(const TransitionItem& item, *temp) {
				TransitionItem* modifier = const_cast<TransitionItem*>(&item);
				modifier->setProbability(averageProbability);
			}

		}

		srand(time(0));

	}	

    TransitionItem* Model::getNextEventByRandom(QString state) {
		QSet<TransitionItem>* transitionSet = adjoinSet[state];
		if(!transitionSet) {
			std::cout << "empty" << std::endl;
			return NULL;
		}
		QList<TransitionItem* > transitionItems;
		QList<double> weightList;
		foreach(const TransitionItem& item, *transitionSet) {
			TransitionItem* modifier = const_cast<TransitionItem*>(&item);
			transitionItems.append(modifier);
			if(weightList.size() == 0)
				weightList.append(item.getProbability());
			else
				weightList.append(item.getProbability() + weightList.last());
		}


		double random = ((double) rand() / (RAND_MAX));
		for(int i = 0; i < weightList.size(); i++) {
			if(i == 0 && (random < weightList[0])) {
				// std::cout << "Choose: " + transitionItems[0]->toString().toStdString() << std::endl;
				// std::cout << "choose: " << 0 << std::endl;
				return transitionItems[0];
			} else if(i != 0 && random >= weightList[i - 1] && random < weightList[i]) {
				// std::cout << "Choose: " + transitionItems[i]->toString().toStdString() << std::endl;
				// std::cout << "choose: " << i << std::endl;
				return transitionItems[i];
			}
		}
		return transitionItems[transitionSet->size() - 1];

	}
}