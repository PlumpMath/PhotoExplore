#ifndef LEAPIMG_RESOURCE_MANAGER_TYPES_H_
#define LEAPIMG_RESOURCE_MANAGER_TYPES_H_

#include <boost/function.hpp>
#include <opencv2/opencv.hpp>

typedef float _PriorityType;
typedef int ResourceType;
typedef int LevelOfDetail;


#define LowestPriority 100


struct IResourceWatcher {

	virtual void resourceUpdated(std::string resourceId, bool loaded) = 0;

};

class TextDefinition {
	
private:
	std::string key;

public:	
	const std::string text;
	const Color textColor;
	const float fontSize;
	const cv::Size2f textWrapSize;

	TextDefinition() : 
		fontSize(0)
	{		
	}

	TextDefinition(TextDefinition & copy) : 
		text(copy.text),
		textColor(copy.textColor),
		fontSize(copy.fontSize),
		textWrapSize(copy.textWrapSize),
		key(copy.key)
	{		
	}

	TextDefinition(std::string _text, Color _textColor, float _textSize, cv::Size2f _textWrapSize) :
		text(_text),
		textColor(_textColor),
		fontSize(_textSize),
		textWrapSize(_textWrapSize)
	{
		std::stringstream ss;
		ss << _text << _textColor.idValue() << _textSize << _textWrapSize.width << _textWrapSize.height;
		key = ss.str();
	}

public:
	std::string getKey()
	{
		return key;
	}
};


#endif