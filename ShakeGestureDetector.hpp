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

using namespace Leap;
using namespace std;

#define DrawingEnabled false
#define DataLogging false
#define AVG_SHAKE_VELOCITY 50

class ShakeGestureDetector {
	

public:
	static ShakeGestureDetector& getInstance()
	{
		static ShakeGestureDetector instance; 
		return instance;
	}
	
private:
	ShakeGestureDetector()
	{
		if (DataLogging) dataOut.open("shake_data.csv");
		if (DataLogging)  dataOut << "Pointable,Hint,Detect,Ratio,PeriodMean,PeriodDev,PeriodRatio,PeakMean,PeakDev,PeakRatio \n";			
		sampleCount = 120;
		lastFrameId = 0;
	}
	ShakeGestureDetector(ShakeGestureDetector const&);
	void operator=(ShakeGestureDetector const&); 

	~ShakeGestureDetector()
	{
		if (DataLogging)  dataOut.close();
	}

private:
	ofstream dataOut;
	long lastFrameId;
	int gestureState;	
	list<float> pointList;
	
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


public:
	
	bool shakeGestureOccured()
	{
		return gestureState == 1;
	}

	void resetDetector()
	{
		lastSampleMap.clear();
		drawZeros.clear();
		gestureState = 0;
	}


	void calculateZeroCrossings(int pId, list<VelocitySample> & samples)
	{

		glfwPollEvents();
		bool isHint = glfwGetKey(GLFW_KEY_TAB) == GLFW_PRESS;


		vector<long> zc;		
		vector<double> peakValues;

		if (samples.size() < 3)
			return;

		auto n0 = samples.rbegin();
		auto n1 = samples.rbegin();
		auto n2 = samples.rbegin();

		n1++;
		
		n2++;
		n2++;

		float averageTipSpeed = 0;
		Vector averageTipVelocity(0,0,0);

		for (; n2 != samples.rend(); n1++,n0++,n2++)
		{
			if (abs(n1->Value) < abs(n2->Value) && abs(n1->Value) < abs(n0->Value))			
			{
				if (DrawingEnabled) drawZeros.insert(n1->SampleId);
				zc.push_back(n1->Time);							
			}
			else if (abs(n1->Value) > abs(n2->Value) && abs(n1->Value) > abs(n0->Value))			
			{
				if (DrawingEnabled) drawZeros.insert(n1->SampleId);
				peakValues.push_back(n1->Value);							
			}

			averageTipSpeed += n0->TipVelocity.magnitude();
			averageTipVelocity += n0->TipVelocity;
			

			if (zc.size() >= 5)
				break;
		}

		if (zc.size() >= 2)
		{
			double speedVelocityRatio = averageTipSpeed/averageTipVelocity.magnitude();

			vector<double> v;
			for (int i=1;i<zc.size();i++)
			{
				v.push_back(zc[i-1] - zc[i]);
			}		
			double periodMean,periodStdDev;
			calculateSignalMetrics(v,periodMean,periodStdDev);

			double peakMagMean,peakMagStdDev;		
			calculateSignalMetrics(peakValues,peakMagMean,peakMagStdDev);

			if (speedVelocityRatio > 6 && periodMean > 40000.0 && (periodMean/periodStdDev) > 3.0 && peakMagMean > 40 && (peakMagMean/peakMagStdDev) > 3.0)
			{
				cooldownTimer.start();
				gestureState = 1;
			}
			//cout << speedVelocityRatio << "," << periodMean <<"," << periodStdDev<< "," << (periodMean/periodStdDev) << "," << peakMagMean << "," << peakMagStdDev << "," << peakMagMean/peakMagStdDev << "\n";
			//cout << speedVelocityRatio << "," << periodMean <<"," << periodStdDev<< "," << (periodMean/periodStdDev) << "," << peakMagMean << "," << peakMagStdDev << "," << peakMagMean/peakMagStdDev << "\n";
			if (DataLogging) dataOut << pId << "," << isHint << "," << gestureState << "," << speedVelocityRatio << "," << periodMean <<"," << periodStdDev<< "," << (periodMean/periodStdDev) << "," << peakMagMean << "," << peakMagStdDev << "," << peakMagMean/peakMagStdDev << "\n";
		}
		else if (DataLogging) dataOut << pId << "," << isHint << "," << gestureState << "," << 0 << "," << 0 <<"," << 0 << "," << 0 << "," << 0 << "," << 0 << "," << 0 << endl;

		//dataOut << isHint << "," << speedVelocityRatio << "," << periodMean <<"," << periodStdDev<< "," << (periodMean/periodStdDev) << "," << peakMagMean << "," << peakMagStdDev << "," << peakMagMean/peakMagStdDev << "\n";
	}


	void calculateSignalMetrics(vector<double> & v, double & mean, double & stdev)
	{	
		double sum = std::accumulate(v.begin(), v.end(), 0.0);
		mean = sum / v.size();

		double accum = 0.0;
		std::for_each (std::begin(v), std::end(v), [&](const double d) {
			accum += (d - mean) * (d - mean);
		});

		stdev = sqrt(accum / (v.size()-1));

		//std::vector<double> diff(v.size());
		//std::transform(v.begin(), v.end(), diff.begin(), std::bind2nd(std::minus<double>(), mean));
		//double sq_sum = std::inner_product(diff.begin(), diff.end(), diff.begin(), 0.0);
		//stdev = std::sqrt(sq_sum / v.size());
	}

	void onFrame(const Controller & controller)
	{
		static int sampleId = 0;

		
		if (gestureState == 1)
		{
			if (cooldownTimer.seconds() > 1)
				resetDetector();
		}

		Frame frame = controller.frame();

		if (frame.id() == lastFrameId)
			return;

		lastFrameId = frame.id();

	
		//Hand h = frame.hands().rightmost();	


		//if (h.isValid())
		//{
		//	Vector cp1=  h.palmNormal().cross(p.direction());
		//	float sample = cp1.magnitude();
		//	int key = p.id();

		//	lastSampleMap[key].push_back(VelocitySample(p.tipVelocity(),sample,frame.timestamp(), sampleId++));
		//	if (lastSampleMap[key].size() > sampleCount)
		//	{
		//		if (DrawingEnabled)
		//		{
		//			auto e =drawZeros.find(lastSampleMap[key].front().SampleId);
		//			if (e != drawZeros.end())
		//				drawZeros.erase(e);
		//		}
		//		lastSampleMap[key].pop_front();

		//		calculateZeroCrossings(key, lastSampleMap[key]);

		//		if (gestureState == 1)
		//			break;
		//	}
		//}
		
		for (int i = 0;i < frame.pointables().count();i++)
		{				
			Pointable p = frame.pointables()[i];

			if (p.isValid())
			{
				Vector cp1=  p.tipVelocity().cross(p.direction());
				float sample = cp1.magnitude(); //p.tipVelocity().magnitude();
				int key = p.id();

				lastSampleMap[key].push_back(VelocitySample(p.tipVelocity(),sample,frame.timestamp(), sampleId++));
				if (lastSampleMap[key].size() > sampleCount)
				{
					if (DrawingEnabled)
					{
						auto e =drawZeros.find(lastSampleMap[key].front().SampleId);
						if (e != drawZeros.end())
							drawZeros.erase(e);
					}
					lastSampleMap[key].pop_front();

					calculateZeroCrossings(key, lastSampleMap[key]);

					if (gestureState == 1)
						break;
				}
			}
		}
	}


	void draw()
	{
		if (DrawingEnabled)
		{
			static Color colors [] = {Colors::Red,Colors::Blue,Colors::Orange,Colors::HoloBlueBright,Colors::LeapGreen};

			glBindTexture(GL_TEXTURE_2D,NULL);

			queue<pair<float,float> > zeroQueue;
		
			for (auto it = lastSampleMap.begin(); it != lastSampleMap.end();it++)
			{
				float x = 0;
				glColor4fv(colors[(it->first)%5].getFloat());
			
				glLineWidth(3);
				glBegin(GL_LINE_STRIP);
				for (auto point = it->second.begin(); point != it->second.end(); point++)
				{
					float xD = (x++)*(GlobalConfig::ScreenWidth/sampleCount);
					glVertex3f(xD,point->Value+1000,10);

					if (drawZeros.count(point->SampleId) != 0)
					{
						zeroQueue.push(make_pair(xD,point->Value+1000));
					}
				}
				glEnd();
			}


			glColor4fv(Colors::Lime.getFloat());
			glLineWidth(5);
			glBegin(GL_LINES);
			while (!zeroQueue.empty())
			{
				glVertex3f(zeroQueue.front().first,zeroQueue.front().second+20,10);
				glVertex3f(zeroQueue.front().first,zeroQueue.front().second-20,10);
				zeroQueue.pop();
			}
			glEnd();

		}
	}

};


#endif