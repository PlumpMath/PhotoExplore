//#include <include/cef_browser.h>
//#include <include/cef_client.h>
//#include <include/cef_app.h>
//#include <include/cef_urlrequest.h>
//
//#ifndef IMAGE_LOADING_URL_CLIENT_H_
//#define IMAGE_LOADING_URL_CLIENT_H_
//
//class ImageLoadingURLClient : public CefURLRequestClient 
//{
//private:
//
//
//
//public:
//    ImageLoadingURLClient() 
//	{ 
//
//	}
//
//    ~ImageLoadingURLClient() 
//	{
//	
//	}
//
//
//	void OnRequestComplete(CefRefPtr<CefURLRequest> request)
//	{		
//		//CefRefPtr<CefResponse> resp = request->GetResponse();
//		//cout << "Request complete! Resp = " << resp->GetStatusText().ToString() << "\n";
//	}
//
//	void OnUploadProgress(CefRefPtr<CefURLRequest> request, uint64 current, uint64 total)
//	{
//		;
//	}
//
//	void OnDownloadProgress(CefRefPtr<CefURLRequest> request,
//                                  uint64 current,
//                                  uint64 total)
//	{
//		;
//	}
//
//	void OnDownloadData(CefRefPtr<CefURLRequest> request,
//                              const void* data,
//                              size_t data_length)
//	{
//
//		char * strData = (char*)data;
//
//		std::stringstream ss;
//		for (int i=0;i<data_length;i++)
//		{
//			ss << strData[i];
//		}
//
//		cout << "Data complete!" << ss.str() << "\n";
//	}
//
//    IMPLEMENT_REFCOUNTING(CefalopodUrlRequestClient);
//};
//
//#endif
//
