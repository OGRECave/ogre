#ifndef _SUBJECT_H_
#define _SUBJECT_H_

#include <list>

class Observer;

typedef std::list<Observer*> ObserverList;

class Subject { 
public: 
     virtual ~Subject(); 
     virtual void attach(Observer*); 
     virtual void detach(Observer*); 
     virtual void notify(); 
     
protected: 
     Subject(); 
     
private: 
     ObserverList mObservers;
};

Subject::Subject() {}

Subject::~Subject()
{   
    // TODO: Clear list!
}

void Subject::attach(Observer* o) { 
     mObservers->Insert(_observers->end(), o); 
} 

void Subject::detach(Observer* o) { 
     mObservers->remove(o); 
} 

void Subject::notify () { 
     ObserverList::iterator it;
     for(it = mObservers.begin(); it != mObservers.end(); ++it)
     {
        *it->update(this);
     }
}

#endif // _SUBJECT_H_

