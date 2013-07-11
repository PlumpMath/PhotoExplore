#ifndef LEAPIMAGE_VIEW_ORCHESTRATOR_HPP_
#define LEAPIMAGE_VIEW_ORCHESTRATOR_HPP_

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/sequenced_index.hpp>
#include <boost/multi_index/composite_key.hpp>
#include <boost/multi_index/mem_fun.hpp>

#include "View.hpp"

using namespace std;



class ViewOwner {

public:
	virtual void viewOwnershipChanged(View * view, ViewOwner * newOwner)
	{
		//return false;
	}

};

struct CachedView {
	
	View * view;
	ViewOwner * currentOwner;
	string viewId;

	CachedView(string _viewId, View * _view, ViewOwner * _owner):
		viewId(_viewId),
		view (_view),
		currentOwner(_owner)
	{

	}
};

struct ViewIdIndex{};

typedef boost::multi_index_container
	<
		CachedView,
		boost::multi_index::indexed_by
		<
			boost::multi_index::ordered_unique
			<
				boost::multi_index::tag<ViewIdIndex>,
				boost::multi_index::member<CachedView,string,&CachedView::viewId>
			>
		>
	> ViewRepository;


class ViewOrchestrator {

private:
	static ViewOrchestrator  * instance;
public:
	static ViewOrchestrator  * getInstance();

private:
	ViewRepository viewCache;


public:
		
	View * requestView(string & viewIdentifier, ViewOwner * owner, bool & ownershipChanged);
	View * requestView(string & viewIdentifier, ViewOwner * owner);

	void registerView(string & viewIdentifier, View * view, ViewOwner * owner);
	
	void releaseView(string & viewIdentifier);
	void releaseView(View * view, ViewOwner * owner);

};


#endif