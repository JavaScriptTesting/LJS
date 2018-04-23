#include <iostream>
#include <QString>
#include <QHash>
namespace artemis {

class TransitionItem {
public:
	TransitionItem(QString from, QString event, QString to);
	TransitionItem(const TransitionItem &);
	TransitionItem &operator=(const TransitionItem &);
	QString toString() const;
	QString getFrom() const;
	QString getEvent() const;
	QString getTo() const;

	double getProbability() const;

	void setProbability(double probability) ;

	inline bool operator==(const TransitionItem &t1) const{
		return t1.getFrom() == from
			   && t1.getEvent() == event
			   && t1.getTo() == to; 
	}

	
private:
	QString from;
	QString event;
	QString to;
	double probability;

};



inline uint qHash(const TransitionItem &key) {
	uint seed = 31;
	uint result = 1;
	result = seed * result + ::qHash(key.getTo());
	result = seed * result + ::qHash(key.getEvent());
	result = seed * result + ::qHash(key.getFrom());
	return result;

}

}