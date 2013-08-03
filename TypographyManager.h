#include <iostream>
#include <ft2build.h>
#include <opencv2/opencv.hpp>
#include FT_FREETYPE_H
#include FT_GLYPH_H 
#include <freetype/ftbitmap.h>
#include "Types.h"

#ifndef TYPOGRAPHY_MANAGER_H_
#define TYPOGRAPHY_MANAGER_H_

#define MAX_GLYPHS 400

using namespace std;

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

struct TextLayoutConfig {
		
	const static int LeftAligned = 0;
	const static int CenterAligned = 1;
	const static int RightAligned = 2;

	TextLayoutConfig(int _alignment, float _maxLineWidth) :
		alignment(_alignment),
		maxLineWidth(_maxLineWidth),
		fitToText(false)
	{

	}

	int maxLineWidth;
	int alignment;
	bool fitToText;

};


class TypographyManager {

private:
	FT_Library fontLibrary;
	map<string,FT_Face> fontFaceMap;
	static TypographyManager * instance;
	bool freetypeInitialized;
		
	TypographyManager()
	{
		freetypeInitialized = false;
		init();
	}

public:
	static TypographyManager * getInstance()
	{
		if (instance == NULL)
			instance = new TypographyManager();

		return instance;
	}

private:
	void init();

	void computeBoundingBox(FT_BBox & bbox, FT_Glyph * glyphArray, cv::Point2i * pos, int numGlyphs);
	void computeGlyphs(string text, FT_Face fontFace, cv::Point2i * pos, int fontSize, FT_Glyph * glyphs, int & numGlyphs, TextLayoutConfig & config);
	void drawBitmapToMatrix(cv::Mat & matrix, FT_Bitmap & glyphBitmap, cv::Point2i topLeft, Color textColor);

	cv::Mat renderTextFreeType(std::string text, FT_Face fontFace, int fontSize, Color textColor, TextLayoutConfig & config, cv::Rect_<float> & textRect);

public:

	cv::Mat renderText(std::string text, string fontName, Color textColor, float fontScale, TextLayoutConfig & config, cv::Rect_<float> & textRect);
};


#endif