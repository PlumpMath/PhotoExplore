#include <include/cef_urlrequest.h>

#ifndef CEFURLREQ_H_
#define CEFURLREQ_H_

class CefalopodUrlRequestClient : public CefURLRequestClient 
{

public:
    CefalopodUrlRequestClient() 
	{ 

	}

    ~CefalopodUrlRequestClient() 
	{
	
	}


	void OnRequestComplete(CefRefPtr<CefURLRequest> request)
	{		
		CefRefPtr<CefResponse> resp = request->GetResponse();
		cout << "Request complete! Resp = " << resp->GetStatusText().ToString() << "\n";
	}

	void OnUploadProgress(CefRefPtr<CefURLRequest> request, uint64 current, uint64 total)
	{

	}

	void OnDownloadProgress(CefRefPtr<CefURLRequest> request,
                                  uint64 current,
                                  uint64 total)
	{
		
		//cout << "Request progressed!" << "\n";
	}

	void OnDownloadData(CefRefPtr<CefURLRequest> request,
                              const void* data,
                              size_t data_length)
	{

		char * strData = (char*)data;

		std::stringstream ss;
		for (int i=0;i<data_length;i++)
		{
			ss << strData[i];
		}

		cout << "Data complete!" << ss.str() << "\n";
	}

    IMPLEMENT_REFCOUNTING(CefalopodUrlRequestClient);
};

#endif