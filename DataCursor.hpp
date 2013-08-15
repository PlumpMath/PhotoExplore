#ifndef LEAPIMAGE_CURSOR_DATACURSOR_HPP_
#define LEAPIMAGE_CURSOR_DATACURSOR_HPP_

#include <boost/function.hpp>
#include "DataNode.hpp"

class DataCursor {

public:		
	enum State
	{
		Local = 0,
		Loading = 1,
		Ended = 2,		
		Finished = 3
	};

	volatile State state;
	boost::function<void()> cursorChangedCallback;

	DataCursor() :	
	state(State::Local)
	{

	}

	virtual State getState() {
		return state;
	}

	virtual DataNode * getNext() = 0;
};


#endif