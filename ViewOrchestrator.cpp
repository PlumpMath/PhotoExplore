#include "ViewOrchestrator.hpp"

ViewOrchestrator * ViewOrchestrator::instance = NULL;

ViewOrchestrator * ViewOrchestrator::getInstance()
{
	if (ViewOrchestrator::instance == NULL)
		ViewOrchestrator::instance = new ViewOrchestrator();

	return ViewOrchestrator::instance;
}

View * ViewOrchestrator::requestView(string & viewIdentifier, ViewOwner * owner, bool & ownershipChanged)
{
	ownershipChanged = false;
	return requestView(viewIdentifier, owner);
}

View * ViewOrchestrator::requestView(string & viewIdentifier, ViewOwner * owner)
{
	auto v = viewCache.find(viewIdentifier);
	if (v == viewCache.end())
		return NULL;
	else
	{
		if (v->currentOwner != owner)
		{			
			CachedView v2 = (*v);

			if (v2.currentOwner != NULL)
				v2.currentOwner->viewOwnershipChanged(v2.view, owner);

			v2.currentOwner = owner;
			viewCache.get<ViewIdIndex>().replace(v,v2);
		}
		return v->view;
	}
}

void ViewOrchestrator::registerView(string & viewIdentifier, View * view, ViewOwner * owner)
{
	auto v = viewCache.get<ViewIdIndex>().find(viewIdentifier);
	if (v == viewCache.end())
		viewCache.insert(CachedView(viewIdentifier,view,owner));
	else
	{	
		if (v->view != view || v->currentOwner != owner)
		{			
			CachedView v2 = (*v);

			if (v2.currentOwner != owner)
			{
				if (v2.currentOwner != NULL)
					v2.currentOwner->viewOwnershipChanged(v2.view, owner);

				v2.currentOwner = owner;
			}
			v2.view = view;
			viewCache.get<ViewIdIndex>().replace(v,v2);
		}
	}
}

void ViewOrchestrator::releaseView(string & viewIdentifier)
{
	 viewCache.get<ViewIdIndex>().erase(viewIdentifier);
}