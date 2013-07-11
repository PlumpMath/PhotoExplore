#ifndef CEF_FILE_BROWSER_H_
#define CEF_FILE_BROWSER_H_

#include <include/cef_client.h>

class CefFileBrowser : public CefRunFileDialogCallback, public CefClient, public CefLifeSpanHandler
{
private:
	bool done;

public:
	vector<CefString> result;

	CefFileBrowser()
	{
		done = false;
	}

	~CefFileBrowser()
	{

	}

	CefRefPtr<CefLifeSpanHandler> GetLifeSpanHandler()
	{
		return CefRefPtr<CefLifeSpanHandler>(this);
	}


	void OnAfterCreated(CefRefPtr<CefBrowser> browser)
	{
		vector<CefString> acceptedTypes;
		CefRefPtr<CefFileBrowser> cb = CefRefPtr<CefFileBrowser>(this);

		browser->GetHost()->RunFileDialog(FILE_DIALOG_OPEN_MULTIPLE, "C:","C:",acceptedTypes,cb.get());
	}

	void OnFileDialogDismissed(CefRefPtr<CefBrowserHost> browser_host, const std::vector<CefString>& file_paths)
	{
		result = vector<CefString>(file_paths);

		cout << file_paths.size() << " files chosen\n";
		
		browser_host->CloseBrowser(false);

		done = true;
	}

	bool isDone()
	{
		return done;
	}


	IMPLEMENT_REFCOUNTING(CefFileBrowser);
};
#endif