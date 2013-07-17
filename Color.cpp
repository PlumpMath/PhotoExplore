#include "Types.h"


Color::Color()
{
	for (int i=0;i<4;i++)
		colorArray[i] = 0;
}	

Color::Color(boost::property_tree::ptree colorConfig)
{
	float ri = colorConfig.get<float>("R");
	float gi = colorConfig.get<float>("G");
	float bi = colorConfig.get<float>("B");
	float ai = colorConfig.get<float>("A",1.0f);

	if (ri + gi + bi + ai > 4.0f)
	{
		colorArray[0] = ri / 255.f;
		colorArray[1] = gi  / 255.f;
		colorArray[2] = bi / 255.f;
		colorArray[3] = ai / 255.f;
	}
	else
	{		
		colorArray[0] = ri;
		colorArray[1] = gi;
		colorArray[2] = bi;
		colorArray[3] = ai;
	}
}

Color::Color(int r, int g, int b, int a)
{	
	colorArray[0] = r / 255.f;
	colorArray[1] = g / 255.f;
	colorArray[2] = b / 255.f;
	colorArray[3] = a / 255.f;
}

Color::Color(float * _colorArray)
{
	for (int i=0;i<4;i++)
		colorArray[i] = _colorArray[i];
}

float * Color::getFloat()
{
	return &colorArray[0];
}


void Color::setAlpha(float alpha)
{
	colorArray[3] = alpha;
}

void Color::scaleRGB(float scale)
{
	for (int i=0;i<3;i++)
	{
		colorArray[i] *= scale;
	}
}

void Color::addRGB(Color c)
{
	for (int i=0;i<3;i++)
	{
		colorArray[i] += c.colorArray[i];
	}
}

//int Color::asInteger()
//{	
//	int result = 0;
//	for (int i=0;i<3;i++)
//	{
//		result << 2;
//		int comp =  (int)(std::min<float>(255.0f,(255.0f * colorArray[i])));
//		result += comp;
//	}
//	return result;
//}

float Color::idValue()
{
	float uniqueId = 0;

	for (int i=0;i<4;i ++)
	{
		uniqueId += colorArray[i] + (float)i;
	}
	return uniqueId;
}	

Color Color::withAlpha(float _alpha)
{
	float tmp[4] = {colorArray[0],colorArray[1],colorArray[2],_alpha};
	return Color(&tmp[0]);
}

//Color Color::fromFloat(float _a, float _r, float _g, float _b)
//{
//	Color c;
//	c.a = _a;
//	c.r = _r;
//	c.g = _g;
//	c.b = _b;
//	return c;
//}
