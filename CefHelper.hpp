#ifndef LEAPIMAGE_CEF_HELPER_HPP_
#define LEAPIMAGE_CEF_HELPER_HPP_

#include <include/cef_browser.h>
#include <include/cef_client.h>
#include <include/cef_app.h>
#include <include/cef_urlrequest.h>

#include "GraphicContext.hpp"


class MyVisitor : public CefCookieVisitor
{
public:
	bool Visit( const CefCookie& cookie, int count, int total, bool& deleteCookie )
	{
		deleteCookie = true;
		return true;
	}

	IMPLEMENT_REFCOUNTING(MyVisitor);
};

class DeleteComplete : public CefCompletionHandler
{
public:
	void OnComplete()
	{
		CefShutdown();		
		GraphicsContext::getInstance().invokeApplicationExitCallback();
	}

	IMPLEMENT_REFCOUNTING(DeleteComplete);
};

class DeleteCookieTask : public CefTask {

public:
	void Execute()
	{		
		CefCookieManager::GetGlobalManager()->DeleteCookies("","");		
		CefRefPtr<DeleteComplete> cc = new DeleteComplete();
		CefCookieManager::GetGlobalManager()->FlushStore(cc.get());
	}

	IMPLEMENT_REFCOUNTING(DeleteCookieTask);

};

class ShutdownTask : public CefTask {

public:
	void Execute()
	{		
		CefShutdown();
		GraphicsContext::getInstance().invokeApplicationExitCallback();
	}

	IMPLEMENT_REFCOUNTING(ShutdownTask);

};


#endif