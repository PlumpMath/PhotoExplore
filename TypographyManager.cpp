#include "TypographyManager.h"
#include "GlobalConfig.hpp"
#include <iostream>
#include <fstream>

TypographyManager * TypographyManager::instance = NULL;

void TypographyManager::init()
{

	FT_Error error;

	error = FT_Init_FreeType( &fontLibrary );

	if ( error ) {
		cout << "Error initializing TTF library. Error code = " << error << " \n";
	} else {

		std::string fontFile = GlobalConfig::tree()->get<string>("Typography.FontFile");

		error = FT_New_Face( fontLibrary,fontFile.c_str(),0,&fontFace);

		if (error == FT_Err_Unknown_File_Format) {
			cout << "Invalid TTF file format for file: " << fontFile <<  "\n";
		} else if ( error ) {
			cout << "Error opening TTF file: " << fontFile << ", Error code = " << error << "\n";
		} else {
			freetypeInitialized = true;
		}
	}
}



cv::Mat TypographyManager::renderText(std::string text, Color textColor, float fontScale, cv::Size2f targetSize)
{
	if (freetypeInitialized)
	{
		float scaleAdjust = GlobalConfig::tree()->get<float>("Typography.FontScaleModifier");
		if (GlobalConfig::tree()->get<bool>("Typography.AdjustToScreenHeight"))
			scaleAdjust *= GlobalConfig::ScreenHeight/1440.0f;

		return renderTextFreeType(text,64 * (int)(ceilf(fontScale * scaleAdjust)),textColor, targetSize.width);
	}
	else
	{
		fontScale *= .2f;

		int fontFace = cv::FONT_HERSHEY_DUPLEX;
		int thickness =(int)(fontScale*1.5f);

		int baseline=0;
		cv::Size textSize = cv::getTextSize(text, fontFace,fontScale, thickness, &baseline);

		cv::Mat img(textSize.height*1.5f, textSize.width, CV_8UC4, cv::Scalar::all(0));

		cv::putText(img, text, cv::Point2i(0,textSize.height), fontFace, fontScale,cv::Scalar(textColor.r * 255,textColor.g* 255,textColor.b* 255, textColor.a* 255), thickness, CV_AA);

		return img;
	}
}


cv::Mat TypographyManager::renderTextFreeType(std::string text, int fontSize, Color textColor, float targetWrapWidth)
{
	


	int numGlyphs = text.size();

	FT_Error error;

	error = FT_Set_Char_Size(fontFace,0,fontSize,300,300);

	FT_BBox boundingBox;
	FT_Glyph * glyphArray = new FT_Glyph[numGlyphs];
	cv::Point2i * positionArray = new cv::Point2i[numGlyphs];

	if (targetWrapWidth <= 0)
		targetWrapWidth = 10000;



	long lineSpacing = FT_MulFix(fontFace->height, fontFace->size->metrics.y_scale) >> 6;
	long fontHeight = FT_MulFix(fontFace->height, fontFace->size->metrics.height) >> 6;

	computeGlyphs_wrap(text,positionArray,(int)targetWrapWidth,lineSpacing,glyphArray,numGlyphs);
	//else
	//computeGlyphs(text,positionArray,glyphArray,numGlyphs);

	computeBoundingBox(boundingBox,glyphArray,positionArray,numGlyphs);

	float stringWidth  = boundingBox.xMax - boundingBox.xMin;
	float stringHeight = boundingBox.yMax - boundingBox.yMin;

	float targetWidth = stringWidth+3;
	float targetHeight = ceilf((float)stringHeight/(float)lineSpacing)*(float)lineSpacing;

	int start_x = 0;//((targetWidth - stringWidth) / 2.0f);
	int start_y = lineSpacing; //((targetHeight - stringHeight) / 2.0f) - lineSpacing/2.0f;
	
	//ofstream fontDebugOut;
	//fontDebugOut.open("G://FontDebug//FontLog.txt");

	//fontDebugOut << "Target height = " << targetHeight << endl;
	//fontDebugOut << "BB YMin = " << boundingBox.yMin << endl;
	//fontDebugOut << "BB YMax = " << boundingBox.yMax << endl;
	//fontDebugOut <<  text << endl;

	cv::Mat img = cv::Mat::ones((int)targetHeight,(int)targetWidth,CV_8UC4);

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
			//fontDebugOut << "start_y = " << start_y << "," << "pen_y = " << pen.y << "bit->top = " << bit->top << endl;
			drawBitmapToMatrix(img,bit->bitmap,cv::Point2i(start_x + pen.x, (start_y + pen.y + boundingBox.yMin) - bit->top),textColor);
			FT_Done_Glyph(image);
		}
	}
	//fontDebugOut.close();
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
			data[j+0] = textColor.r * 255;
			data[j+1] = textColor.g * 255;
			data[j+2] = textColor.b * 255;
			data[j+3] = max<unsigned char>(0,val);
		}
	}
}


void TypographyManager::computeGlyphs_wrap(string text, cv::Point2i * pos, int wrapWidth, int lineSpacing, FT_Glyph * glyphs, int & numGlyphs)
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

	//cout << "Glyphs for : " << text << ":  ";
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
			//if (deltaK != 0)
			//	cout << text.at(n-1) << "_" << deltaK << "_" << glyphChar;
		}
		
		error = FT_Load_Glyph( fontFace, glyph_index, FT_LOAD_DEFAULT );
		if (error) 
			continue;  

		error = FT_Get_Glyph( fontFace->glyph, &glyphs[numGlyphs] );
		if (error)
			continue; 
		
		if (glyphChar == ' ' || glyphChar == '\t')
			lastSplit = n;
				
		if (pen_x > wrapWidth)
		{	
			if (lastSplit > 0 && lastSplit < n-1)
			{
				xOffset = pos[lastSplit+1].x;
				pen_y += lineSpacing;
				pen_x -= xOffset;

				for (int i = lastSplit+1; i < n;i++)
				{
					pos[i].y += lineSpacing;
					pos[i].x -= xOffset;
				}					
			}
			else
			{
				pen_y += lineSpacing;
				pen_x = 0;
			}		
			lastSplit = 0;
		}

		pos[numGlyphs].x = pen_x;
		pos[numGlyphs].y = pen_y;
		
		pen_x += slot->advance.x >> 6;
		pen_y += slot->advance.y >> 6;


		previous = glyph_index;
		numGlyphs++;
	}
	 //cout << endl;

}


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
			glyph_bbox.yMin += pos[n].y;
			glyph_bbox.yMax += pos[n].y;

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

	void TypographyManager::computeGlyphs(string text, cv::Point2i * pos, FT_Glyph * glyphs, int & numGlyphs)
	{		
		FT_GlyphSlot  slot = fontFace->glyph;   /* a small shortcut */
		FT_UInt       glyph_index;
		FT_Bool       use_kerning;
		FT_UInt       previous;
		int           pen_x, pen_y;

		FT_Error error;

		pen_x = 0;   /* start at (0,0) */
		pen_y = 0;

		use_kerning = FT_HAS_KERNING( fontFace );
		previous    = 0;
		numGlyphs = 0;

		for (int n = 0; n < text.size(); n++ )
		{
			/* convert character code to glyph index */
			glyph_index = FT_Get_Char_Index( fontFace, text.at(n));

			/* retrieve kerning distance and move pen position */
			if ( use_kerning && previous && glyph_index )
			{
				FT_Vector  delta;
				FT_Get_Kerning( fontFace, previous, glyph_index, FT_KERNING_DEFAULT, &delta );
				pen_x += delta.x >> 6;
			}

			/* store current pen position */
			pos[numGlyphs].x = pen_x;
			pos[numGlyphs].y = pen_y;

			/* load glyph image into the slot without rendering */
			error = FT_Load_Glyph( fontFace, glyph_index, FT_LOAD_DEFAULT );
			if ( error )
				continue;  /* ignore errors, jump to next glyph */

			/* extract glyph image and store it in our table */
			error = FT_Get_Glyph( fontFace->glyph, &glyphs[numGlyphs] );
			if ( error )
				continue;  /* ignore errors, jump to next glyph */

			/* increment pen position */
			pen_x += slot->advance.x >> 6;

			/* record current glyph index */
			previous = glyph_index;

			/* increment number of glyphs */
			numGlyphs++;
		}
	}
