#ifndef LEAPIMAGE_SHAKE_GESTURE_DETECTOR_HPP_
#define LEAPIMAGE_SHAKE_GESTURE_DETECTOR_HPP_

#include <Leap.h>
#include <LeapMath.h>
#include "GLImport.h"
#include <list>
#include <map>
#include "Types.h"
#include "HandModel.h"
#include <queue>
#include <set>
#include <numeric>
#include <vector>
#include <list>
#include <iostream>
#include <fstream>
#include "LeapHelper.h"
#include "kiss_fft.h"
#include "SDLTimer.h"

using namespace Leap;
using namespace std;


struct ShakeGesture {
	
	float frequency, magnitude;

	ShakeGesture(float _frequency, float _magnitude) :
		frequency(_frequency),
		magnitude(_magnitude)	
	{

	}

};

class ShakeGestureDetector {
	

public:
	static ShakeGestureDetector& getInstance();
	
private:
	ShakeGestureDetector();
	ShakeGestureDetector(ShakeGestureDetector const&);
	void operator=(ShakeGestureDetector const&); 
	~ShakeGestureDetector();

private:
	ofstream rawDataOut, resultDataOut;
	long lastFrameId;
	list<float> pointList;
	bool DataLoggingRaw, DataLoggingResult, DrawingEnabled;
	
	int sampleCount;

	struct VelocitySample {

		float Value;
		long Time;
		int SampleId;
		Vector TipVelocity;
		
		VelocitySample(Vector _TipVelocity, float _Value, long _Time, int _SampleId) :
			Value(_Value),
			Time(_Time),
			SampleId(_SampleId),
			TipVelocity(_TipVelocity)
		{}
	};

	map<int,list<VelocitySample> > lastSampleMap;

	set<long> drawZeros;

	Timer cooldownTimer;

	vector<ShakeGesture> completedGestures; 


public:
	int gestureState;	
	

	void resetDetector();
	ShakeGesture * getLatestShakeGesture();

	//void calculateZeroCrossings(int pId, list<VelocitySample> & samples)
	//{

	//	glfwPollEvents();
	//	bool isHint = glfwGetKey(GLFW_KEY_TAB) == GLFW_PRESS;


	//	vector<long> zc;		
	//	vector<double> peakValues;

	//	if (samples.size() < 3)
	//		return;

	//	auto n0 = samples.rbegin();
	//	auto n1 = samples.rbegin();
	//	auto n2 = samples.rbegin();

	//	n1++;
	//	
	//	n2++;
	//	n2++;

	//	float averageTipSpeed = 0;
	//	Vector averageTipVelocity(0,0,0);

	//	for (; n2 != samples.rend(); n1++,n0++,n2++)
	//	{
	//		if (abs(n1->Value) < abs(n2->Value) && abs(n1->Value) < abs(n0->Value))			
	//		{
	//			if (DrawingEnabled) drawZeros.insert(n1->SampleId);
	//			zc.push_back(n1->Time);							
	//		}
	//		else if (abs(n1->Value) > abs(n2->Value) && abs(n1->Value) > abs(n0->Value))			
	//		{
	//			if (DrawingEnabled) drawZeros.insert(n1->SampleId);
	//			peakValues.push_back(n1->Value);							
	//		}

	//		averageTipSpeed += n0->TipVelocity.magnitude();
	//		averageTipVelocity += n0->TipVelocity;
	//		

	//		if (zc.size() >= 5)
	//			break;
	//	}

	//	if (zc.size() >= 2)
	//	{
	//		double speedVelocityRatio = averageTipSpeed/averageTipVelocity.magnitude();

	//		vector<double> v;
	//		for (int i=1;i<zc.size();i++)
	//		{
	//			v.push_back(zc[i-1] - zc[i]);
	//		}		
	//		double periodMean,periodStdDev;
	//		calculateSignalMetrics(v,periodMean,periodStdDev);

	//		double peakMagMean,peakMagStdDev;		
	//		calculateSignalMetrics(peakValues,peakMagMean,peakMagStdDev);

	//		if (speedVelocityRatio > 6 && periodMean > 40000.0 && (periodMean/periodStdDev) > 3.0 && peakMagMean > 40 && (peakMagMean/peakMagStdDev) > 3.0)
	//		{
	//			cooldownTimer.start();
	//			gestureState = 1;
	//		}
	//		//cout << speedVelocityRatio << "," << periodMean <<"," << periodStdDev<< "," << (periodMean/periodStdDev) << "," << peakMagMean << "," << peakMagStdDev << "," << peakMagMean/peakMagStdDev << "\n";
	//		//cout << speedVelocityRatio << "," << periodMean <<"," << periodStdDev<< "," << (periodMean/periodStdDev) << "," << peakMagMean << "," << peakMagStdDev << "," << peakMagMean/peakMagStdDev << "\n";
	//		if (DataLogging) dataOut << pId << "," << isHint << "," << gestureState << "," << speedVelocityRatio << "," << periodMean <<"," << periodStdDev<< "," << (periodMean/periodStdDev) << "," << peakMagMean << "," << peakMagStdDev << "," << peakMagMean/peakMagStdDev << "\n";
	//	}
	//	else 
	//		if (DataLogging) dataOut << pId << "," << isHint << "," << gestureState << "," << 0 << "," << 0 <<"," << 0 << "," << 0 << "," << 0 << "," << 0 << "," << 0 << endl;

	//	//dataOut << isHint << "," << speedVelocityRatio << "," << periodMean <<"," << periodStdDev<< "," << (periodMean/periodStdDev) << "," << peakMagMean << "," << peakMagStdDev << "," << peakMagMean/peakMagStdDev << "\n";
	//}

	//void calculateZeroCrossingsForAngle(int pId, list<VelocitySample> & samples)
	//{

	//	glfwPollEvents();
	//	bool isHint = glfwGetKey(GLFW_KEY_TAB) == GLFW_PRESS;


	//	vector<long> zc;		
	//	vector<double> peakValues;

	//	if (samples.size() < 3)
	//		return;

	//	auto n0 = samples.rbegin();
	//	auto n1 = samples.rbegin();
	//	auto n2 = samples.rbegin();

	//	n1++;
	//	
	//	n2++;
	//	n2++;
	//	
	//	for (; n2 != samples.rend(); n1++,n0++,n2++)
	//	{
	//		if (abs(n1->Value) < abs(n2->Value) && abs(n1->Value) < abs(n0->Value))			
	//		{
	//			if (DrawingEnabled) drawZeros.insert(n1->SampleId);
	//			zc.push_back(n1->Time);							
	//		}
	//		else if (abs(n1->Value) > abs(n2->Value) && abs(n1->Value) > abs(n0->Value))			
	//		{
	//			if (DrawingEnabled) drawZeros.insert(n1->SampleId);
	//			peakValues.push_back(n1->Value);							
	//		}			

	//		if (zc.size() >= 5)
	//			break;
	//	}

	//	if (zc.size() >= 2)
	//	{
	//		vector<double> v;
	//		for (int i=1;i<zc.size();i++)
	//		{
	//			v.push_back(zc[i-1] - zc[i]);
	//		}		
	//		double periodMean,periodStdDev;
	//		calculateSignalMetrics(v,periodMean,periodStdDev);
	//		
	//		//if (speedVelocityRatio > 6 && periodMean > 40000.0 && (periodMean/periodStdDev) > 3.0 && peakMagMean > 40 && (peakMagMean/peakMagStdDev) > 3.0)
	//		//{
	//		//	cooldownTimer.start();
	//		//	gestureState = 1;
	//		//}
	//		if (DataLogging) 
	//			dataOut << pId << "," << isHint << "," << gestureState << "," << periodMean <<"," << periodStdDev<< "," << (periodMean/periodStdDev) << "\n";
	//	}
	//	else 
	//		if (DataLogging)
	//			dataOut << pId << "," << isHint << "," << gestureState << "," << 0 <<"," << 0<< "," << 0 << "\n";

	//}

	void calculateFFT(int pId, list<VelocitySample> & samples);

	void calculateSignalMetrics(vector<double> & v, double & mean, double & stdev);
	
	void onFrame(const Controller & controller);

	void draw();

};


#endif