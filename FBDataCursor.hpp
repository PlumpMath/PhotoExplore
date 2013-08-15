#ifndef LEAPIMAGE_FB_DATA_CURSOR_HPP_
#define LEAPIMAGE_FB_DATA_CURSOR_HPP_

#include "FBNode.h"
#include "GlobalConfig.hpp"
#include "DataCursor.hpp"

using namespace Facebook;

class FBSimpleEdgeCursor : public DataCursor {

protected:
	FBNode * node;	
	FBNode * nextItem;
	string edgeName;
	bool ascending;
	long nextItemNumber;
	int itemsPerRequest;

public:
	FBSimpleEdgeCursor(FBNode * _node, string _edgeName, bool _ascending = false) :
		node(_node),
		nextItem(NULL),
		edgeName(_edgeName),
		ascending(_ascending)
	{
		itemsPerRequest = GlobalConfig::tree()->get<int>("FacebookAPI.ItemsPerRequest");
	}

	virtual void loadItems(int items) = 0;
	void reset();
	DataNode * getNext();
	FBNode * getNextAsc();
	FBNode * getNextDesc();
};


class FBFriendsCursor : public FBSimpleEdgeCursor {

public:
	FBFriendsCursor(FBNode * _node) : FBSimpleEdgeCursor(_node,"friends")
	{}

	void loadItems(int friends);
};


class FBFriendsFQLCursor : public DataCursor {

private:
	FBNode * node;	
	FBNode * nextItem;
	string searchName;
	int currentPosition;
	int itemsPerRequest;


public:
	FBFriendsFQLCursor(string _searchName, FBNode * _node) :
		searchName(_searchName),
		node(_node),
		nextItem(NULL)
	{
		itemsPerRequest = GlobalConfig::tree()->get<int>("FacebookAPI.ItemsPerRequest");
	}

	void loadItems(int friends);

	DataNode * getNext();

	void lookupName(string searchName);


};

class FBUserAlbumsCursor : public FBSimpleEdgeCursor {

public:
	FBUserAlbumsCursor(FBNode * _node) : FBSimpleEdgeCursor(_node,"albums")
	{}

	void loadItems(int items);
};

class FBAlbumPhotosCursor : public FBSimpleEdgeCursor {

public:
	FBAlbumPhotosCursor(FBNode * _node) : FBSimpleEdgeCursor(_node,"photos",true)
	{}

	FBAlbumPhotosCursor(FBNode * _node, bool ascending) : FBSimpleEdgeCursor(_node,"photos",ascending)
	{}

	void loadItems(int items);
};

class FBUserPhotosCursor : public FBAlbumPhotosCursor {

public:
	FBUserPhotosCursor(FBNode * _node) : FBAlbumPhotosCursor(_node,false)
	{}
};

class InterleavingCursor : public DataCursor {
	
private:
	int lastCursorIndex; 

public:
	DataCursor * cursor1, * cursor0;

	InterleavingCursor(DataCursor * _cursor0, DataCursor * _cursor1) :
		cursor0(_cursor0),
		cursor1(_cursor1),
		lastCursorIndex(0)
	{

	}

	DataNode * getNext();

	State getState();

};



#endif