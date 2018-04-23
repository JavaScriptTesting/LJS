#include "transitionItem.h"
#include <iostream>

namespace artemis
{
	TransitionItem::TransitionItem(QString from, QString event, QString to) {
		this->from = from;
		this->event = event;
		this->to = to;
		this->probability = 0.0;
	}

	TransitionItem::TransitionItem(const TransitionItem & t) {
		this->from = t.from;
		this->event = t.event;
		this->to = t.to;
		this->probability = t.probability;
	}

	TransitionItem & TransitionItem::operator=(const TransitionItem &t) {
		this->from = t.from;
		this->event = t.event;
		this->to = t.to;
		this->probability = t.probability;

		return *this;
	}


	QString TransitionItem::getFrom() const{
		return this->from;
	}

	QString TransitionItem::getEvent() const{
		return this->event;
	}

	QString TransitionItem::getTo() const{
		return this->to;
	}

	double TransitionItem::getProbability() const{
		return this->probability;
	}

	void TransitionItem::setProbability(double probability) {
		this->probability = probability;
	}

	QString TransitionItem::toString() const{
		// return this->from + " " + this->event + " " + this->to + " " + this->probability + "\n";
		return this->from + " " + this->event + " " + this->to;
	}	
}