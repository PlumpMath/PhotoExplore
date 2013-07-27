#include <include/cef_app.h>
#include <include/cef_urlrequest.h>
#include <json_spirit.h>
#include "FBNode.h"
#include "Types.h"
#include "LeapHelper.h"
#include <exception>

#include <boost/function.hpp>

#include <map>

#ifndef FACEBOOK_LOADER_H_
#define FACEBOOK_LOADER_H_

using namespace std;
using namespace Facebook;

class FacebookDownloadTask : public CefTask, public CefURLRequestClient
{
private:	
	vector<unsigned char> dataVector;

protected:
	bool taskComplete;
	string methodName, targetObject;

public:

	FacebookDownloadTask()
	{
		methodName = "GET";
		taskComplete = false;
		targetObject = "";
	}

	FacebookDownloadTask(string method, string _targetObject)
	{
		methodName = method;
		taskComplete = false;
		targetObject = _targetObject;
	}

	~FacebookDownloadTask()
	{

	}

	virtual string getRequestedObject()
	{
		return targetObject;
	}


	virtual string getTaskURL()
	{
		std::string token = std::string(GlobalConfig::TestingToken);

		std::stringstream urlStream;

		urlStream << "https://graph.facebook.com/";

		string requested = getRequestedObject(); 
		urlStream << requested;

		if (requested.find_first_of('?') == -1)
		{
			urlStream << "?";
		}
		else
		{
			urlStream << "&";
		}

		urlStream << "method=" << methodName << "&redirect=true&access_token=";
		urlStream << token;		
		return urlStream.str();
	}

	void Execute()
	{
		if (!taskComplete)
		{
			CefRefPtr<CefRequest> req = CefRequest::Create();
			req->SetURL(getTaskURL());

			CefRefPtr<CefPostData> pd = CefPostData::Create();

			CefRefPtr<CefPostDataElement> pde = CefPostDataElement::Create();
			unsigned char * b = new unsigned char[1];
			b[0] = 'a';
			pde->SetToBytes(1,&b[0]);
			pd->AddElement(pde);
			req->SetPostData(pd);

			req->SetMethod(methodName);			

			Logger::stream("FacebookLoader","INFO") << "Sending request: " << getTaskURL() << "\n";
			CefURLRequest::Create(req,this);	
		}
	}

	virtual void processResult(vector<unsigned char> dataVector)
	{		
		taskComplete = true;
	}

	void OnRequestComplete(CefRefPtr<CefURLRequest> request)
	{	

		CefRequest::HeaderMap hm;
		request->GetRequest()->GetHeaderMap(hm);

		for (auto it = hm.begin();it != hm.end();it++)
		{
			//cout << "Header[" << it->first.ToString() << " = " << it->second.ToString() << "\n";
		}

		if (request->GetRequestStatus() == CefURLRequest::Status::UR_SUCCESS)
		{
			//cout << "Processing result of : " << request->GetRequest()->GetURL().ToString() << "\n";
			processResult(dataVector);
		}
		else
		{
			//cout << "Request failed!\n";			
			//this->Release();
		}
		//request->Release();
	}

	void OnUploadProgress(CefRefPtr<CefURLRequest> request, uint64 current, uint64 total)
	{	}

	void OnDownloadProgress(CefRefPtr<CefURLRequest> request,uint64 current,uint64 total)
	{	}

	void OnDownloadData(CefRefPtr<CefURLRequest> request,const void* data,size_t data_length)
	{		
		if (methodName.compare("GET")==0)
		{
			vector<unsigned char> byteVector;

			unsigned char * strData = (unsigned char*)data;

			for (int i=0;i<data_length;i++)
			{
				dataVector.push_back(strData[i]);
			}
		}
	}

	bool isComplete()
	{
		return taskComplete;
	}

//  private:                                          
//	  CefRefCount refct_;
//
//public:                                           
//	int AddRef()
//	{
//		int count = refct_.AddRef(); 
//		//cout << "Task : " << getRequestedObject() << " Added ref: " << count << " \n";
//		return count;
//	}        
//	int Release() {                                 
//		int retval = refct_.Release();       
//
//		
//		//cout << "Task : " << getRequestedObject() << " Removed ref: " << retval << " \n";
//		if (retval == 0)                              
//			delete this;                                
//		return retval;                                
//	}                                               
//	int GetRefCt() {
//		return refct_.GetRefCt();
//	}   

	IMPLEMENT_REFCOUNTING(FacebookDownloadTask);
};

class JSONDownloadTask : public FacebookDownloadTask
{
public:
	FBNode * callbackNode;
	string objectQuery;
	string edgeType;
	boost::function<void(FBNode*)> onComplete;

	JSONDownloadTask(string objectQuery, FBNode * callbackNode) 
	{
		this->objectQuery = objectQuery;
		this->edgeType = "";
		this->callbackNode = callbackNode;		
	}

	JSONDownloadTask(string objectId, string edgeName, FBNode * callbackNode) 
	{
		stringstream ss;
		ss << objectId << "/" << edgeName;

		this->objectQuery = ss.str();
		this->edgeType = edgeName;
		this->callbackNode = callbackNode;		
	}
	
	string getRequestedObject()
	{
		return objectQuery;
	}

	void processJSONResult(json_spirit::Value result, string dataString)
	{		
		Logger::stream("FacebookLoader","INFO") << "Processing JSON result : " << dataString << "\n";

		if (result.type() == json_spirit::obj_type)
		{
			vector<json_spirit::Pair> obj = result.get_obj();	
			callbackNode->addJSON(edgeType,obj);
		}
		taskComplete = true;

		if (!onComplete.empty())
			onComplete(callbackNode);

		//this->Release();
	}

	
	void processResult(vector<unsigned char> dataVector)
	{		
		string dataString = string((char*)dataVector.data());

		json_spirit::Value result;
		json_spirit::read(dataString, result);

		processJSONResult(result, dataString);
	}

};

class FacebookLoader : public FBDataSource {

private:
	map<string,CefRefPtr<JSONDownloadTask> > activeTaskMap;

public:
	FacebookLoader()
	{

	}

	void postRequest(string request)
	{		
		CefRefPtr<FacebookDownloadTask> newTask = new FacebookDownloadTask("POST",request);
		CefTaskRunner::GetForThread(TID_IO).get()->PostTask(newTask.get());
	}

	void load(FBNode * parent, string objectId, string edge)
	{
//		std::exception e;// = std::exception("Not implemented!");
//		e.what
		//CefRefPtr<JSONDownloadTask> newTask = new JSONDownloadTask(objectId,edge,parent);
		//newTask->AddRef();
		//CefTaskRunner::GetForThread(TID_IO).get()->PostTask(newTask.get());
	}

	void loadField(FBNode * parent, string nodeQuery, string interpretAs, boost::function<void(FBNode*)> loadCompleteCallback)
	{
		//cout << "Loading node fields " << nodeQuery << "\n";
		Logger::stream("FacebookLoader","INFO") << "Sending request: " << nodeQuery << endl;
		CefRefPtr<JSONDownloadTask> newTask = new JSONDownloadTask(nodeQuery,parent);
		newTask->onComplete = loadCompleteCallback;
		newTask->edgeType = interpretAs;
		
		CefTaskRunner::GetForThread(TID_IO).get()->PostTask(newTask.get());		
	}

	void loadQuery(FBNode * parent, string nodeQuery, string interpretAs, boost::function<void(FBNode*)> loadCompleteCallback)
	{
		Logger::stream("FacebookLoader","INFO") << "Executing query: " << nodeQuery << "\n";
		CefRefPtr<JSONDownloadTask> newTask = new JSONDownloadTask(nodeQuery,parent);
		newTask->edgeType = interpretAs;
		newTask->onComplete = loadCompleteCallback;

		CefTaskRunner::GetForThread(TID_IO).get()->PostTask(newTask.get());
	}

};

#endif
