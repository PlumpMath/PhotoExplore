#ifndef LEAPIMAGE_CURSOR_DATACURSOR_HPP_
#define LEAPIMAGE_CURSOR_DATACURSOR_HPP_

#include <boost/function.hpp>
#include "DataNode.hpp"
#include <vector>

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

class VectorCursor : public DataCursor {

private:
	int currentIndex;
	std::vector<DataNode*> data;

public:
	VectorCursor(std::vector<DataNode*> _data):
		data(_data),
		currentIndex(-1)
	{	

	}

	State getState()
	{
		if (currentIndex < data.size())
			return Local;
		else
			return Ended;
	}
	
	DataNode * getNext()
	{
		if (currentIndex < 0)
		{
			currentIndex++;
			return NULL;
		}
		else if (currentIndex < data.size())
		{
			return data.at(currentIndex++);
		}
		else
			return NULL;
	}


};


#endif