#include "state.h"
#include <assert.h>
#include <time.h>
#include <iostream>

namespace artemis
{

	State::State(QString state) {
		this->state = state;
	}

    QSet<QString> State::getEnabled() {
    	return this->enabled;
    }


    QSet<QString> State::getDone() {
    	return done;
    }
    QString State::getSelEV() {
    	return selEV;
    }

    void State::setSleep(QSet<QString> sleep) {
        this->sleep = sleep;
    }

    QSet<QString> State::getSleep() {
        return this->sleep;
    }


    void State::addSleep(QString event) {
        (this->sleep).insert(event);
    }


    QSet<QString> State::getPrinted() {
        return this->printed;
    }
    void State::addPrinted(QString event) {
        this->printed.insert(event);
    }

    void State::setSelEV(QString event) {
    	this->selEV = event;
    }

    void State::setDone(QSet<QString> done) {
    	this->done = done;
    }

    void State::addToDone(QString event) {
    	this->done.insert(event);
    }

    QString State::getStringState() {
    	return this->state;
    }

}