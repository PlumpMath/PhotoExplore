#ifndef CEFALOPOD_H_
#define CEFALOPOD_H_

#include <include/cef_client.h>
#include "GLImport.h"
#include <Windows.h>


#ifndef GL_UNSIGNED_INT_8_8_8_8_REV
#define GL_UNSIGNED_INT_8_8_8_8_REV 0x8367
#endif

class Cefalopod : public CefClient, public CefLifeSpanHandler, public CefLoadHandler//, public CefRenderHandler
{
public:
	bool done, loadedEnded, quit;

	std::string token;

	//unsigned char * drawBuffer;
	//bool isDirty;
	//int drawWidth,drawHeight;

	//int view_width_,view_height_;
	//GLuint renderTexture;


	//
	//struct LazyType
	//{
	//	LazyType()
	//	{
	//		buffer = NULL;
	//		draw = false;
	//		width = height = 0;
	//	}
	//	bool draw;
	//	RectList dirtyRects;
	//	int * buffer;
	//	int width,height;
	//};
	//LazyType lazy;

    Cefalopod()
	{	
		quit = loadedEnded = done = false;
		token = std::string("");			
		//glGenTextures(1,&renderTexture);
		//glBindTexture(GL_TEXTURE_2D,renderTexture);
		//
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); 
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		//glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

		//glBindTexture(GL_TEXTURE_2D,NULL);

		//view_width_ = view_height_ = 0;
	}
    ~Cefalopod() { }

	//
	//class MyVisitor : public CefCookieVisitor
	//{
	//public:
	//	bool Visit( const CefCookie& cookie, int count, int total, bool& deleteCookie )
	//	{
	//		cout << "Cookie[" << count << "]: Value = " << CefString(cookie.value.str).ToString() << " Domain = " << CefString(cookie.path.str).ToString() << "  Path = " << CefString(cookie.path.str).ToString() <<  "\n";
	//		return true;
	//	}

	//	IMPLEMENT_REFCOUNTING(MyVisitor);
	//};

	//void OnLoadStart(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame)
	//{				
	//	loadedEnded = true;
	//}

	//void OnLoadEnd(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, int httpStatusCode)
	//{		
	//	//if (frame->GetURL().ToString().find_first_of("access_token") > 0)
	//	//	cout << "URL: " << frame->GetURL().ToString() << "\n";
	//	//browser->GetHost()->CloseBrowser(true);
	//}
	//

	CefRefPtr<CefLifeSpanHandler> GetLifeSpanHandler()
	{
		return CefRefPtr<CefLifeSpanHandler>(this);
	}

	CefRefPtr<CefLoadHandler> GetLoadHandler()
	{
		return CefRefPtr<CefLoadHandler>(this);
	}
	
	bool DoClose(CefRefPtr<CefBrowser> browser)
	{
		//quit = true;
		//browser->GetHost()->CloseBrowser(false);
		return false; 
	}
	
	void OnAfterCreated(CefRefPtr<CefBrowser> browser)
	{
		//CefRefPtr<MyVisitor> v = new MyVisitor();
		//CefCookieManager::GetGlobalManager()->VisitAllCookies(v.get());
		SwitchToThisWindow(browser->GetHost()->GetWindowHandle(),false);
		browser->GetHost()->SetFocus(true);
	}

	//CefRefPtr<CefRenderHandler> GetRenderHandler()
	//{
	//	return CefRefPtr<CefRenderHandler>(this);
	//}
	void OnLoadError(CefRefPtr<CefBrowser> browser,CefRefPtr<CefFrame> frame,ErrorCode errorCode,const CefString& errorText,const CefString& failedUrl)
	{		
		int tokenIndex = failedUrl.ToString().find("access_token=");
		int tokenEnd = failedUrl.ToString().find_first_of("&");
		if (tokenIndex > 0)
		{
			//cout << "Token URL: " << failedUrl.ToString() << "\n";
			tokenIndex += 13;
			token = failedUrl.ToString().substr(tokenIndex,tokenEnd-tokenIndex);
			//cout << "Token=" << token << "\n";
			browser->GetHost()->CloseBrowser(false);
			done = true;
		}
		//browser->GetHost()->CloseBrowser(true);
	}

	
	//
	//bool GetScreenPoint(CefRefPtr<CefBrowser> browser,int viewX,int viewY,int& screenX,int& screenY) 
	//{ 
	//	screenX = viewX;
	//	screenY = viewY;
	//	return true;
	//}
	//

	//bool GetRootScreenRect(CefRefPtr<CefBrowser> browser,CefRect& rect)
	//{
	//	rect.Set(0,0,GlobalConfig::ScreenWidth/2.0f,GlobalConfig::ScreenHeight/2.0f);
	//	return true;
	//}

	//bool GetViewRect(CefRefPtr<CefBrowser> browser, CefRect& rect) 
	//{
	//	rect.Set(0,0,GlobalConfig::ScreenWidth/2.0f,GlobalConfig::ScreenHeight/2.0f);
	//	
	//	glTexImage2D(GL_TEXTURE_2D,0,4,rect.width,rect.height,0,GL_RGBA,GL_UNSIGNED_BYTE,0);
	//	return true;
	//}

	//bool GetScreenInfo(CefRefPtr<CefBrowser> browser, CefScreenInfo& screen_info) 
	//{
	//	screen_info.depth = 24;
	//	CefRect r1,r2;
	//	r1.Set(0,0,GlobalConfig::ScreenWidth*.5f, GlobalConfig::ScreenHeight*.5f);
	//	r2.Set(0,0,GlobalConfig::ScreenWidth*.5f, GlobalConfig::ScreenHeight*.5f);
	//	screen_info.Set(1,32,8,false,r1,r2);
	//	return true; 
	//}


/*
	void OnPaint(CefRefPtr<CefBrowser> browser,PaintElementType type,const RectList& dirtyRects,const void* buffer,int width, int height)
	{				
		return ;
		if (!lazy.draw && type == PET_VIEW) {
			
			lazy.dirtyRects = dirtyRects;

			if (lazy.width != width || lazy.height != height)
			{
				delete lazy.buffer;
				lazy.buffer = NULL;
			}

			if (lazy.buffer == NULL)
			{				
				lazy.buffer = new int[width*height*4];
			}
			memcpy(lazy.buffer,buffer,width*height*4);

			lazy.width = width;
			lazy.height = height;
			lazy.draw = true;
		}
	}*/

	//void draw()
	//{
	//	return;

	//	glBindTexture(GL_TEXTURE_2D, renderTexture);
	//	if (lazy.draw)
	//	{
	//		int old_width = view_width_;
	//		int old_height = view_height_;

	//		view_width_ = lazy.width;
	//		view_height_ = lazy.height;



	//		glPixelStorei(GL_UNPACK_ROW_LENGTH, view_width_);

	//		if (old_width != view_width_ || old_height != view_height_) {
	//			// Update/resize the whole texture.
	//			glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
	//			glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
	//			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, view_width_, view_height_, 0,
	//				GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, lazy.buffer);
	//		} else {
	//			// Update just the dirty rectangles.
	//			CefRenderHandler::RectList::const_iterator i = lazy.dirtyRects.begin();
	//			for (; i != lazy.dirtyRects.end(); ++i) {
	//				const CefRect& rect = *i;
	//				glPixelStorei(GL_UNPACK_SKIP_PIXELS, rect.x);
	//				glPixelStorei(GL_UNPACK_SKIP_ROWS, rect.y);
	//				glTexSubImage2D(GL_TEXTURE_2D, 0, rect.x, rect.y, rect.width,
	//					rect.height, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV,
	//					lazy.buffer);
	//			}
	//		}
	//		//glBindTexture(GL_TEXTURE_2D,NULL);
	//		lazy.draw = false;
	//	}


	//	float tx1 = 0,tx2 = 1,ty1 = 0,ty2 = 1;

	//	float z1 = 10;			

	//	glBindTexture(GL_TEXTURE_2D,renderTexture);

	//	int texWidth,texHeight;
	//	glGetTexLevelParameteriv(GL_TEXTURE_2D,0,GL_TEXTURE_WIDTH,&texWidth);
	//	glGetTexLevelParameteriv(GL_TEXTURE_2D,0,GL_TEXTURE_HEIGHT,&texHeight);


	//	float x1 = 0, y1 = 0;
	//	float x2 = view_width_, y2 = view_height_;	
	//	glColor4f(1,1,1,1);
	//	glBegin( GL_QUADS );

	//		glTexCoord2f(tx1,ty1);
	//		glVertex3f(x1,y1,z1); 

	//		glTexCoord2f(tx2,ty1);
	//		glVertex3f(x2,y1,z1);

	//		glTexCoord2f(tx2,ty2);

	//		glVertex3f(x2,y2,z1);

	//		glTexCoord2f(tx1,ty2);
	//		glVertex3f(x1,y2,z1);

	//	glEnd();	

	//	glBindTexture(GL_TEXTURE_2D,NULL);
	//}


    IMPLEMENT_REFCOUNTING(Cefalopod);
};

#endif