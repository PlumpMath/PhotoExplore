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

class TypographyManager {

private:
	FT_Library fontLibrary;
	FT_Face fontFace;
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
	void computeGlyphs(string text, cv::Point2i * pos, FT_Glyph * glyphs, int & numGlyphs);
	void computeGlyphs_wrap(string text, cv::Point2i * pos, int wrapWidth, int fontSize, FT_Glyph * glyphs, int & numGlyphs);
	void drawBitmapToMatrix(cv::Mat & matrix, FT_Bitmap & glyphBitmap, cv::Point2i topLeft, Color textColor);

	cv::Mat renderTextFreeType(std::string text, int fontSize, Color textColor, float targetWrapWidth);

public:

	cv::Mat renderText(std::string text, Color textColor, float fontScale, cv::Size2f targetSize);
};


#endif