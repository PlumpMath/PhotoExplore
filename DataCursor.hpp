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

class BidirectionalCursor : public DataCursor {

public:
	BidirectionalCursor() : DataCursor()
	{

	}

	virtual DataNode * getPrevious() = 0;
	
	bool fastForward(DataNode * node)
	{
		DataNode * next = getNext();
		do
		{
			next = getNext();
		}
		while (next != node && next != NULL);

		if (next != NULL)
		{
			getPrevious();
			return true;
		}
		return false;
	}

};


class WrappingBDCursor : public BidirectionalCursor {

private:
	DataCursor * cursor;
	std::vector<DataNode*> nodeHistory;
	int nodeIndex;

public:
	WrappingBDCursor(DataCursor * _cursor) :
		cursor(_cursor),
		nodeIndex(-1)
	{

	}

	DataNode * getNext()
	{
		DataNode * result = NULL;
		if (nodeIndex+1 < nodeHistory.size())
		{
			result = nodeHistory.at(++nodeIndex);
		}
		else
		{
			result = cursor->getNext();
			
			if (result != NULL)
			{
				nodeIndex++;
				nodeHistory.push_back(result);
			}
		}

		return result;
	}


	DataNode * getPrevious()
	{
		DataNode * result = NULL;

		if (nodeIndex > 0)
		{		
			result = nodeHistory.at(--nodeIndex);
		}
		else if (nodeIndex == 0) //Don't want to go lower than -1
		{
			nodeIndex--;
		}

		return result;
	}


};





#endif