#ifndef LEAPIMAGE_FB_DATA_CURSOR_HPP_
#define LEAPIMAGE_FB_DATA_CURSOR_HPP_

#include "FBNode.h"
#include "GlobalConfig.hpp"

using namespace Facebook;

class FBDataCursor {
	

protected:
	int itemsPerRequest;

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

	FBDataCursor() :	
		state(State::Local)
	{
		itemsPerRequest = GlobalConfig::tree()->get<int>("FacebookAPI.ItemsPerRequest");
	}

	virtual State getState() {
		return state;
	}

	virtual FBNode * getNext() = 0;

};

class FBSimpleEdgeCursor : public FBDataCursor {

protected:
	FBNode * node;	
	FBNode * nextItem;
	string edgeName;
	bool ascending;

public:
	FBSimpleEdgeCursor(FBNode * _node, string _edgeName, bool _ascending = false) :
		node(_node),
		nextItem(NULL),
		edgeName(_edgeName),
		ascending(_ascending)
	{
	}

	virtual void loadItems(int items) = 0;
	void reset();
	FBNode * getNext();
	FBNode * getNextAsc();
	FBNode * getNextDesc();
};


class FBFriendsCursor : public FBSimpleEdgeCursor {

public:
	FBFriendsCursor(FBNode * _node) : FBSimpleEdgeCursor(_node,"friends")
	{}

	void loadItems(int friends);
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

class FBUserAlbumsCursor : public FBSimpleEdgeCursor {

public:
	FBUserAlbumsCursor(FBNode * _node) : FBSimpleEdgeCursor(_node,"albums",true)
	{}

	void loadItems(int items);
};


//class FBUserPhotosCursor : public FBSimpleEdgeCursor {
//
//public:
//	FBUserPhotosCursor(FBNode * _node) : FBSimpleEdgeCursor(_node,"photos")
//	{}
//
//	void loadItems(int items);
//};

class FBAlbumPhotosCursor : public FBSimpleEdgeCursor {

public:
	FBAlbumPhotosCursor(FBNode * _node) : FBSimpleEdgeCursor(_node,"photos")
	{}

	void loadItems(int items);
};

class InterleavingCursor : public FBDataCursor {
	
private:
	int lastCursorIndex; 

public:
	FBDataCursor * cursor1, * cursor0;

	InterleavingCursor(FBDataCursor * _cursor0, FBDataCursor * _cursor1) :
		cursor0(_cursor0),
		cursor1(_cursor1),
		lastCursorIndex(0)
	{

	}

	FBNode * getNext();

	State getState();

};



#endif