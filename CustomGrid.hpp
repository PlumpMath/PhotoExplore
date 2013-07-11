#ifndef LEAPIMAGE_CUSTOM_GRID_HPP_
#define LEAPIMAGE_CUSTOM_GRID_HPP_

#include "View.hpp"

struct RowDefinition {
	
	float Height;	
	vector<float> ColumnWidths;

	RowDefinition(float _Height, vector<float> & _ColumnWidths):
		Height(_Height),
		ColumnWidths(_ColumnWidths)
	{
		
	}

	RowDefinition(float _Height):
		Height(_Height)
	{
		
	}
};

class CustomGrid : public ViewGroup {

private:	
	vector<RowDefinition> rowDefinitions;

	cv::Size2i gridDimensions;
	bool transpose;
	cv::Size2f defaultSize;
	cv::Vec4f cellPadding;	

	vector<Leap::Vector> childPositions;
	vector<cv::Size2f> childSizes;
	
	cv::Size2f gridSize;

	void measureChildren();

public:
	CustomGrid(vector<RowDefinition> rowDefinitions, bool transpose = false);

	void measure(cv::Size2f & measuredSize);
	
	void layout(Leap::Vector position, cv::Size2f size);
	
	cv::Rect_<int> getHitRect();

};

#endif