#include <include/cef_urlrequest.h>
#include <vector>

#ifndef CEF_DOWNLOAD_CLIENT_H_
#define CEF_DOWNLOAD_CLIENT_H_

using namespace std;

class CefDownloadClient : public CefURLRequestClient
{
private:	
	vector<unsigned char> dataVector;

protected:
	bool downloadComplete;

public:

	CefDownloadClient()
	{
		downloadComplete = false;
	}

	~CefDownloadClient()
	{

	}
	
	virtual void processResult(bool success, vector<unsigned char> dataVector) = 0;
		

	void OnRequestComplete(CefRefPtr<CefURLRequest> request)
	{	
		processResult(request->GetRequestStatus() == CefURLRequest::Status::UR_SUCCESS, dataVector);		
	}

	void OnUploadProgress(CefRefPtr<CefURLRequest> request, uint64 current, uint64 total)
	{
		;
	}

	void OnDownloadProgress(CefRefPtr<CefURLRequest> request,uint64 current,uint64 total)
	{
		;
	}

	void OnDownloadData(CefRefPtr<CefURLRequest> request,const void* data,size_t data_length)
	{		
		unsigned char * strData = (unsigned char*)data;

		for (int i=0;i<data_length;i++)
		{
			dataVector.push_back(strData[i]);
		}		
	}

	bool isComplete()
	{
		return downloadComplete;
	}
	
    IMPLEMENT_REFCOUNTING(CefDownloadClient);
};


#endif