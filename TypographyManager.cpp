#include "TypographyManager.h"
#include "GlobalConfig.hpp"
#include <iostream>
#include <fstream>
#include "Logger.hpp"

TypographyManager * TypographyManager::instance = NULL;

void TypographyManager::init()
{

	FT_Error error;

	error = FT_Init_FreeType( &fontLibrary );

	if ( error ) {
		Logger::stream("TypographyManager","ERROR") << "Error initializing TTF library. Error code = " << error << " \n";
	} else 
	{

		auto fontList = GlobalConfig::tree()->get_child("Typography.Fonts");

		for (auto fontIt = fontList.begin(); fontIt != fontList.end(); fontIt++)
		{
			string name = fontIt->second.get<string>("Name");
			string fontFile = fontIt->second.get<string>("FontFile");

			FT_Face fontFace;
			error = FT_New_Face( fontLibrary,fontFile.c_str(),0,&fontFace);
		
			if (error == FT_Err_Unknown_File_Format) {
				cout << "Invalid TTF file format for file: " << fontFile <<  "\n";
			} else if ( error ) {
				cout << "Error opening TTF file: " << fontFile << ", Error code = " << error << "\n";
			} else {
				fontFaceMap.insert(make_pair(name,fontFace));
			}
		}

		if (fontFaceMap.size() > 0)
			freetypeInitialized = true;
	}
}


cv::Mat TypographyManager::renderText(std::string text, string fontName, Color textColor, float fontScale, TextLayoutConfig & config)
{
	if (freetypeInitialized)
	{
		auto font = fontFaceMap.find(fontName);
		if (font == fontFaceMap.end())
			font = fontFaceMap.find("Default");

		FT_Face fontFace = font->second;


		cv::Mat result = renderTextFreeType(text,fontFace,(int)(ceilf(fontScale * 64.0f)),textColor, config);
		
		if (GlobalConfig::tree()->get<bool>("Typography.DebugRendering"))
		{
			TextDefinition td(text,textColor,fontScale,cv::Size2f(config.maxLineWidth,0));
			cv::imwrite(GlobalConfig::tree()->get<string>("Typography.DebugImagePath") + td.getKey() + ".png",result);
		}

		return result;
	}
	else
	{
		fontScale *= .2f;

		int fontFace = cv::FONT_HERSHEY_DUPLEX;
		int thickness =(int)(fontScale*1.5f);

		int baseline=0;
		cv::Size textSize = cv::getTextSize(text, fontFace,fontScale, thickness, &baseline);

		cv::Mat img(textSize.height*1.5f, textSize.width, CV_8UC4, cv::Scalar::all(0));

		cv::putText(img, text, cv::Point2i(0,textSize.height), fontFace, fontScale,cv::Scalar(textColor.colorArray[0] * 255,textColor.colorArray[1] * 255,textColor.colorArray[2] * 255, textColor.colorArray[3] * 255), thickness, CV_AA);

		return img;
	}
}


cv::Mat TypographyManager::renderTextFreeType(std::string text, FT_Face fontFace, int fontSize, Color textColor, TextLayoutConfig & config)
{
	int numGlyphs = text.size();

	FT_Error error;
	error = FT_Set_Char_Size(fontFace,0,fontSize,300,300);

	FT_BBox boundingBox;
	FT_Glyph * glyphArray = new FT_Glyph[numGlyphs];
	cv::Point2i * positionArray = new cv::Point2i[numGlyphs];

	if (config.maxLineWidth <= 0)
	{ 
		config.maxLineWidth = 10000;
	}

	long lineSpacing = FT_MulFix(fontFace->height, fontFace->size->metrics.y_scale) >> 6;
	long fontHeight = FT_MulFix(fontFace->height, fontFace->size->metrics.height) >> 6;
	
	computeGlyphs(text,fontFace,positionArray,lineSpacing,glyphArray,numGlyphs,config);
	computeBoundingBox(boundingBox,glyphArray,positionArray,numGlyphs);

	float stringWidth  = boundingBox.xMax - boundingBox.xMin;
	float stringHeight = boundingBox.yMax - boundingBox.yMin;

	float targetWidth = config.maxLineWidth;
	float targetHeight = stringHeight;

	int start_x = ((config.maxLineWidth-stringWidth)/2)-boundingBox.xMin;
	if (config.maxLineWidth == 10000)
		start_x = -boundingBox.xMin;

	int start_y = - boundingBox.yMin; 

	if (boundingBox.yMax > 0 && targetHeight < lineSpacing)
	{
		start_y += boundingBox.yMax;
		targetHeight += boundingBox.yMax;
	}

	cv::Mat img = cv::Mat::zeros((int)targetHeight,(int)targetWidth,CV_8UC4);

	for (int n = 0; n < numGlyphs; n++ )
	{
		FT_Glyph	image;
		FT_Vector	pen;

		image = glyphArray[n];

		pen.x = positionArray[n].x;
		pen.y = positionArray[n].y;

		error = FT_Glyph_To_Bitmap( &image, FT_RENDER_MODE_NORMAL,NULL, 0 );

		if (!error)
		{
			FT_BitmapGlyph bit = (FT_BitmapGlyph)image;
			drawBitmapToMatrix(img,bit->bitmap,cv::Point2i(start_x + pen.x + bit->left, (start_y + pen.y ) - bit->top),textColor);
			FT_Done_Glyph(image);
		}
	}
	return img;
}

void TypographyManager::drawBitmapToMatrix(cv::Mat & matrix, FT_Bitmap & glyphBitmap, cv::Point2i topLeft, Color textColor)
{
	//topLeft.y = max<int>(0,topLeft.y);
	//if (topLeft.y < 0)
	//{
	//	cout << "Negative TL.y = " << topLeft.y << endl;
	//	topLeft. y = 0;
	//}
	for (int i=topLeft.y, i2 = 0; i2 < glyphBitmap.rows && i < matrix.rows;i++, i2++)
	{
		unsigned char * data = matrix.ptr<unsigned char>(i);
		int j2 = i2*glyphBitmap.pitch;
		for (int j=topLeft.x*4;j<matrix.cols*4 && j2 < ((i2+1)*glyphBitmap.pitch);j += 4, j2++)
		{
			unsigned char val =  glyphBitmap.buffer[j2];
			data[j+0] = (unsigned char)(textColor.colorArray[0] * 255.0f);
			data[j+1] = (unsigned char)(textColor.colorArray[1] * 255.0f);
			data[j+2] = (unsigned char)(textColor.colorArray[2] * 255.0f);
			data[j+3] = max<unsigned char>(0,val);
		}
	}
}

void TypographyManager::computeGlyphs(string text, FT_Face fontFace, cv::Point2i * pos, int lineSpacing, FT_Glyph * glyphs, int & numGlyphs, TextLayoutConfig & config)
{		
	FT_GlyphSlot  slot = fontFace->glyph;  
	FT_UInt       glyph_index;
	FT_Bool       use_kerning;
	FT_UInt       previous;
	int           pen_x, pen_y;

	FT_Error error;

	pen_x = 0; 
	pen_y = 0;

	use_kerning = FT_HAS_KERNING( fontFace );
	previous    = 0;
	numGlyphs = 0;

	int lastSplit = -1;


	char glyphChar;

	pen_y = 0;// lineSpacing;
	int xOffset = 0;

	int wrapWidth = config.maxLineWidth;
	
	bool isCentered = config.alignment == TextLayoutConfig::CenterAligned;


	int lineStartIndex = 0, lineCount = 0;//, int leftMost = 0;

	for (int n = 0; n < text.size(); n++ )
	{
		glyphChar = text.at(n);
		
		glyph_index = FT_Get_Char_Index( fontFace, glyphChar);

		if ( use_kerning && previous && glyph_index )
		{
			FT_Vector  delta;
			FT_Get_Kerning( fontFace, previous, glyph_index, FT_KERNING_DEFAULT, &delta );
			int deltaK =  delta.x >> 6;
			pen_x += deltaK;
		}
		
		error = FT_Load_Glyph( fontFace, glyph_index, FT_LOAD_DEFAULT );
		if (error) 
			continue;  

		error = FT_Get_Glyph( fontFace->glyph, &glyphs[numGlyphs] );
		if (error)
			continue; 
		
		if (glyphChar == ' ' || glyphChar == '\t')
			lastSplit = n;
				
		if (pen_x + (slot->advance.x>>6) > wrapWidth || glyphChar == '\n')
		{	
			if (isCentered)
			{
				int lineEnd;
				if (lastSplit > 0)
				{						
					lineEnd = lastSplit -1;
				}
				else
				{
					lineEnd = numGlyphs-1;
				}

				FT_BBox  glyph_bbox;
				FT_Glyph_Get_CBox( glyphs[lineStartIndex], ft_glyph_bbox_pixels,&glyph_bbox );

				int xStart = glyph_bbox.xMin;

				FT_Glyph_Get_CBox( glyphs[lineEnd], ft_glyph_bbox_pixels,&glyph_bbox );

				int lineXOffset = (pos[lineEnd].x+glyph_bbox.xMax) - xStart;
				int lastLineOffset = (wrapWidth - lineXOffset)/2;		
				for (int i = lineStartIndex; i <= lineEnd;i++)
				{
					pos[i].x += lastLineOffset;
				}
			}

			if (lastSplit > 0 && lastSplit < numGlyphs-1)
			{
				xOffset = pos[lastSplit+1].x; //Everything past here gets moved down
				pen_y += lineSpacing;
				pen_x -= xOffset;
				for (int i = lastSplit+1; i < numGlyphs;i++)
				{
					pos[i].y += lineSpacing;
					pos[i].x -= xOffset;
				}		
				
				lineStartIndex = lastSplit+1;
				
				pos[numGlyphs].x = pen_x;
				pos[numGlyphs].y = pen_y;
				
				pen_x += slot->advance.x >> 6;
			}
			else if (lastSplit == n)
			{				
				pen_y += lineSpacing;
				pen_x = 0;
				lineStartIndex = numGlyphs;
			}		
			else
			{				
				pen_y += lineSpacing;
				pen_x = 0;

				pos[numGlyphs].x = pen_x;
				pos[numGlyphs].y = pen_y;
				
				lineStartIndex = numGlyphs;
				pen_x += slot->advance.x >> 6;
			}		
			lastSplit = 0;
			lineCount++;
						
		}
		else
		{			
			pos[numGlyphs].x = pen_x;
			pos[numGlyphs].y = pen_y;

			pen_x += slot->advance.x >> 6;
		}

		
		pen_y += slot->advance.y >> 6;


		previous = glyph_index;
		numGlyphs++;
	}

	if (isCentered && lineCount > 0)
	{		
		FT_BBox  glyph_bbox;
		FT_Glyph_Get_CBox( glyphs[lineStartIndex], ft_glyph_bbox_pixels,&glyph_bbox );

		int xStart = glyph_bbox.xMin;
		
		FT_Glyph_Get_CBox( glyphs[numGlyphs-1], ft_glyph_bbox_pixels,&glyph_bbox );
		
		int xOffset = (pos[numGlyphs-1].x+glyph_bbox.xMax) - xStart;
		int lastLineOffset = (wrapWidth - xOffset)/2;		
		for (int i = lineStartIndex; i < numGlyphs;i++)
		{
			pos[i].x += lastLineOffset;
		}
	}

}


//void TypographyManager::computeGlyphs(string text, FT_Face fontFace, cv::Point2i * pos, int lineSpacing, FT_Glyph * glyphs, int & numGlyphs, TextLayoutConfig & config)
//{		
//	FT_GlyphSlot  slot = fontFace->glyph;  
//	FT_UInt       glyph_index;
//	FT_Bool       use_kerning;
//	FT_UInt       previous;
//	int           pen_x, pen_y;
//
//	FT_Error error;
//
//	pen_x = 0; 
//	pen_y = 0;
//
//	use_kerning = FT_HAS_KERNING( fontFace );
//	previous    = 0;
//	numGlyphs = 0;
//
//	int lastSplit = -1;
//
//
//	char glyphChar;
//
//	pen_y = 0;// lineSpacing;
//	int xOffset = 0;
//
//	int wrapWidth = config.maxLineWidth;
//	
//	bool isCentered = config.alignment == TextLayoutConfig::CenterAligned;
//
//
//	int lineStartIndex = 0, lineCount = 0;//, int leftMost = 0;
//
//	//cout << "Glyphs for : " << text << ":  ";
//	for (int n = 0; n < text.size(); n++ )
//	{
//		glyphChar = text.at(n);
//		
//		glyph_index = FT_Get_Char_Index( fontFace, glyphChar);
//
//		if ( use_kerning && previous && glyph_index )
//		{
//			FT_Vector  delta;
//			FT_Get_Kerning( fontFace, previous, glyph_index, FT_KERNING_DEFAULT, &delta );
//			int deltaK =  delta.x >> 6;
//			pen_x += deltaK;
//			//if (deltaK != 0)
//			//	cout << text.at(n-1) << "_" << deltaK << "_" << glyphChar;
//		}
//		
//		error = FT_Load_Glyph( fontFace, glyph_index, FT_LOAD_DEFAULT );
//		if (error) 
//			continue;  
//
//		error = FT_Get_Glyph( fontFace->glyph, &glyphs[numGlyphs] );
//		if (error)
//			continue; 
//		
//		if (glyphChar == ' ' || glyphChar == '\t')
//			lastSplit = n;
//				
//		if (pen_x > wrapWidth)
//		{	
//			if (lastSplit > 0 && lastSplit < n-1)
//			{
//				xOffset = pos[lastSplit+1].x; //Everything past here gets moved down
//
//				if (isCentered)
//				{
//					int lastLineOffset = (wrapWidth - xOffset)/2;
//					for (int i = lineStartIndex; i < lastSplit;i++)
//					{
//						pos[i].x += lastLineOffset;
//					}	
//				}
//
//				pen_y += lineSpacing;
//				pen_x -= xOffset;
//				for (int i = lastSplit+1; i < n;i++)
//				{
//					pos[i].y += lineSpacing;
//					pos[i].x -= xOffset;
//				}		
//				
//				lineStartIndex = lastSplit+1;
//			}
//			else
//			{
//				pen_y += lineSpacing;
//				pen_x = 0;
//				lineStartIndex = n;
//			}		
//			lastSplit = 0;
//			lineCount++;
//		}
//
//		pos[numGlyphs].x = pen_x;
//		pos[numGlyphs].y = pen_y;
//		
//		pen_x += slot->advance.x >> 6;
//		pen_y += slot->advance.y >> 6;
//
//
//		previous = glyph_index;
//		numGlyphs++;
//	}
//
//	if (isCentered && lineCount > 0)
//	{		
//		FT_BBox  glyph_bbox;
//		FT_Glyph_Get_CBox( glyphs[lineStartIndex], ft_glyph_bbox_pixels,&glyph_bbox );
//
//		int xStart = glyph_bbox.xMin;
//		
//		FT_Glyph_Get_CBox( glyphs[numGlyphs-1], ft_glyph_bbox_pixels,&glyph_bbox );
//		
//		int xOffset = (pos[numGlyphs-1].x+glyph_bbox.xMax) - xStart;
//		int lastLineOffset = (wrapWidth - xOffset)/2;		
//		for (int i = lineStartIndex; i < numGlyphs;i++)
//		{
//			pos[i].x += lastLineOffset;
//		}
//	}
//	 //cout << endl;
//
//}

void TypographyManager::computeBoundingBox(FT_BBox & bbox, FT_Glyph * glyphArray, cv::Point2i * pos, int numGlyphs)
{
	//FT_BBox  bbox;
	FT_BBox  glyph_bbox;

	/* initialize string bbox to "empty" values */
	bbox.xMin = bbox.yMin =  32000;
	bbox.xMax = bbox.yMax = -32000;

	/* for each glyph image, compute its bounding box, */
	/* translate it, and grow the string bbox          */
	for (int n = 0; n < numGlyphs; n++ )
	{
		FT_Glyph_Get_CBox( glyphArray[n], ft_glyph_bbox_pixels,&glyph_bbox );

		glyph_bbox.xMin += pos[n].x;
		glyph_bbox.xMax += pos[n].x;

		auto gMin = glyph_bbox.yMin;
		glyph_bbox.yMin = pos[n].y - glyph_bbox.yMax;
		glyph_bbox.yMax = pos[n].y - gMin;

		if ( glyph_bbox.xMin < bbox.xMin )
			bbox.xMin = glyph_bbox.xMin;

		if ( glyph_bbox.yMin < bbox.yMin )
			bbox.yMin = glyph_bbox.yMin;

		if ( glyph_bbox.xMax > bbox.xMax )
			bbox.xMax = glyph_bbox.xMax;

		if ( glyph_bbox.yMax > bbox.yMax )
			bbox.yMax = glyph_bbox.yMax;
	}

	/* check that we really grew the string bbox */
	if ( bbox.xMin > bbox.xMax )
	{
		bbox.xMin = 0;
		bbox.yMin = 0;
		bbox.xMax = 0;
		bbox.yMax = 0;
	}
}
