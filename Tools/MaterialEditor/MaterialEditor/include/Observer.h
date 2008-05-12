#ifndef _OBSERVER_H_
#define _OBSERVER_H_

class Subject;

class Observer 
{ 
public: 
     virtual ~Observer(); 
     virtual void update(Subject* subject) = 0; 
protected: 
     Observer(); 
};

#endif // _OBSERVER_H_
