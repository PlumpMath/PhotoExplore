#ifndef LEAPIMAGE_FB_DATA_CURSOR_HPP_
#define LEAPIMAGE_FB_DATA_CURSOR_HPP_

#include "FBNode.h"

using namespace Facebook;

class FBDataCursor {
	
//protected:

public:	
	volatile bool isLoading, canLoad;
	boost::function<void()> cursorChangedCallback;

	FBDataCursor() :	
		isLoading(false),
		canLoad(true)
	{
	}

	virtual FBNode * getNext() = 0;

};

class FBFriendsCursor : public FBDataCursor {

private:
	FBNode * node;
	
	FBNode * nextItem;

public:
	FBFriendsCursor(FBNode * _node) :
		node(_node),
		nextItem(NULL)
	{
	}

	void loadItems(int friends);
	void reset();
	FBNode * getNext();


};

class FBFriendsFQLCursor : public FBDataCursor {

private:
	FBNode * node;	
	FBNode * nextItem;
	string searchName;
	int currentPosition;


public:
	FBFriendsFQLCursor(string _searchName, FBNode * _node) :
		searchName(_searchName),
		node(_node),
		nextItem(NULL)
	{}

	void loadItems(int friends);

	FBNode * getNext();

	void lookupName(string searchName);


};

#endif