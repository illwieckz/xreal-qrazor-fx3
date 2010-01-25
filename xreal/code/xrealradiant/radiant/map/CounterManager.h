#ifndef COUNTERMANAGER_H_
#define COUNTERMANAGER_H_

#include "icounter.h"
#include "iradiant.h"
#include "iuimanager.h"
#include <map>
#include "string/string.h"

#include <boost/shared_ptr.hpp>

namespace map {

class Counter : 
	public ICounter
{
	Observer* _observer;
	std::size_t _count;
public:
	Counter(Observer* observer = NULL) :
		_observer(observer), 
		_count(0)
	{}

	virtual ~Counter() {}
	
	void increment() {
		++_count;
		
		if (_observer != NULL) {
			_observer->countChanged();
		}
	}
	
	void decrement() {
		--_count;
		
		if (_observer != NULL) {
			_observer->countChanged();
		}
	}
	
	std::size_t get() const {
		return _count;
	}
};

class CounterManager :
	public ICounter::Observer
{
	typedef boost::shared_ptr<Counter> CounterPtr;
	typedef std::map<CounterType, CounterPtr> CounterMap;
	CounterMap _counters;
public:
	CounterManager() {
		// Create the counter object
		_counters[counterBrushes] = CounterPtr(new Counter(this));
		_counters[counterPatches] = CounterPtr(new Counter(this));
		_counters[counterEntities] = CounterPtr(new Counter(this));	
	}

	virtual ~CounterManager() {}
	
	ICounter& get(CounterType counter) {
		if (_counters.find(counter) == _counters.end()) {
			throw std::runtime_error("Counter ID not found.");
		}
		return *_counters[counter];
	}

	void init() {
		// Add the statusbar command text item
		GlobalUIManager().getStatusBarManager().addTextElement(
			"MapCounters", 
			"",  // no icon
			IStatusBarManager::POS_BRUSHCOUNT
		);
	}
	
	// ICounter::Observer implementation
	void countChanged() {
		std::string text = "Brushes: " + sizetToStr(_counters[counterBrushes]->get());
		text += " Patches: " + sizetToStr(_counters[counterPatches]->get());
		text += " Entities: " + sizetToStr(_counters[counterEntities]->get());
		
		GlobalUIManager().getStatusBarManager().setText("MapCounters", text);
	}
};

} // namespace map

#endif /*COUNTERMANAGER_H_*/
