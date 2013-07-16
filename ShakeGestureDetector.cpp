#include "ShakeGestureDetector.hpp"

ShakeGestureDetector& ShakeGestureDetector::getInstance()
{
	static ShakeGestureDetector instance; 
	return instance;
}

ShakeGestureDetector::ShakeGestureDetector()
{
	DataLoggingRaw = GlobalConfig::tree()->get("Shake.DataLogging.Raw",false);
	DataLoggingResult = GlobalConfig::tree()->get("Shake.DataLogging.Results",false);

	DrawingEnabled = GlobalConfig::tree()->get("Shake.DebugDrawing",false);
	sampleCount = GlobalConfig::tree()->get("Shake.SampleCount",30);

	if (DataLoggingResult) { 
		resultDataOut.open(GlobalConfig::tree()->get("Shake.DataLogging.OutputDir",".") + "/Shake_ResultData.csv");		
		resultDataOut <<  "Hint,SVRatio,Ratio,PeakFreq,PeakMag,Centroid\n";		
	}

	if (DataLoggingRaw)  
	{
		rawDataOut.open(GlobalConfig::tree()->get("Shake.DataLogging.OutputDir",".") + "/Shake_RawData.csv");
		rawDataOut << "Hint";
		for (int i =0;i<sampleCount/2;i++)
		{
			rawDataOut << "," << i;
		}
		rawDataOut << " \n";			
	}

	gestureState = -1;
	lastFrameId = 0;
}

ShakeGestureDetector::~ShakeGestureDetector()
{
	if (DataLoggingRaw) rawDataOut.close();
	if (DataLoggingResult) resultDataOut.close();
}


void ShakeGestureDetector::resetDetector()
{	
	lastSampleMap.clear();
	drawZeros.clear();
	completedGestures.clear();
	cooldownTimer.countdown(GlobalConfig::tree()->get<double>("Shake.CooldownTime"));
}

ShakeGesture * ShakeGestureDetector::getLatestShakeGesture()
{
	if (completedGestures.size() > 0)
		return &completedGestures.back();
	else
		return NULL;
}

void ShakeGestureDetector::calculateFFT(int pId, list<VelocitySample> & samples)
{

	glfwPollEvents();
	int isHint = 0;
	isHint = glfwGetKey(GLFW_KEY_TAB) == GLFW_PRESS;

	kiss_fft_cpx data[30];//=  kiss_fft_cpx[30];//samples.size()];
	kiss_fft_cpx result[30];//= kiss_fft_cpx[30];//samples.size()];

	Vector averageTipVelocity = Vector();
	float averageTipSpeed = 0;
	int i = 0;
	for (auto it = samples.rbegin(); it!=samples.rend();it++)
	{
		data[i].r = it->Value;		
		data[i].i = 0;		
		result[i].r = 0;		
		result[i].i = 0;		
		i++;
		//if (DataLoggingRaw) 
		//	rawDataOut << "," << it->Value;

		averageTipSpeed += it->TipVelocity.magnitude();
		averageTipVelocity += it->TipVelocity;

	}
	float speedVelocityRatio = averageTipSpeed / averageTipVelocity.magnitude();

	kiss_fft_cfg cfg;
	if ((cfg = kiss_fft_alloc(samples.size(), false, NULL, NULL)) != NULL)
	{
		kiss_fft(cfg, data, result);
		free(cfg);  
	}


	if (DataLoggingRaw) rawDataOut << isHint;

	float peakFrequency = 0;

	float peakMag = 0; //, avgMag = 0;
	float totalMag = 0, count = 0;

	for (i = samples.size()/2; i < samples.size();i++)
	{
		float mag = sqrt(pow(result[i].r,2) + pow(result[i].i,2));
		totalMag += mag;
		count++;
	}
	//avgMag = totalMag / count;

	drawZeros.clear();
	float frequencyCentroid= 0;
	lastSampleMap[-(pId + 10)].clear();
	for (i = samples.size()/2; i < samples.size();i++)
	{
		float mag = sqrt(pow(result[i].r,2) + pow(result[i].i,2));

		lastSampleMap[-(pId + 10)].push_back(VelocitySample(Vector(),1000.0f - (mag*.5f),0,1000+i));

		if (mag > peakMag)
		{
			peakMag = mag;
			peakFrequency = i;
		}	

		frequencyCentroid += ((float)i) * mag/totalMag;

		if (DataLoggingRaw) 
			rawDataOut << "," << mag;
	}

	if (DrawingEnabled && peakFrequency > 0)
		drawZeros.insert(1000+peakFrequency);


	auto fftModes = GlobalConfig::tree()->get_child("Shake.FFTModes");
	
	
	for (auto modeIt = fftModes.begin(); modeIt != fftModes.end(); modeIt++)
	{
		float minFreq = modeIt->second.get<float>("MinFrequency");
		float maxFreq = modeIt->second.get<float>("MaxFrequency");

		if (speedVelocityRatio > modeIt->second.get<float>("MinSpeedVelocityRatio") &&
			(peakMag/totalMag) >= modeIt->second.get("AvgMagPeakMagRatio",.3f) && 
			peakFrequency >= minFreq && peakFrequency <= maxFreq)
		{
			float peakMinFreq = modeIt->second.get<float>("MinFreq_MinMagnitude"); 
			float peakMaxFreq = modeIt->second.get<float>("MaxFreq_MinMagnitude");

			float threshold = peakMinFreq;
			if (minFreq != maxFreq)
			{
				float rate = (peakMaxFreq - peakMinFreq) / (maxFreq-minFreq);
				threshold = (rate*(peakFrequency-minFreq))+peakMinFreq;
			}

			if (peakMag > threshold)
			{
				completedGestures.push_back(ShakeGesture(peakFrequency,peakMag));
				isHint = 2;
				break;
			}
		}
	}


	if (DataLoggingResult) 
		resultDataOut << isHint << "," << pId << "," << speedVelocityRatio << "," << (peakMag/totalMag) << "," << peakFrequency << "," << peakMag << "," << frequencyCentroid << endl;

	if (DataLoggingRaw)
		rawDataOut << endl;

	//delete result;
	//delete data;

}

void ShakeGestureDetector::calculateSignalMetrics(vector<double> & v, double & mean, double & stdev)
{	
	double sum = std::accumulate(v.begin(), v.end(), 0.0);
	mean = sum / v.size();

	double accum = 0.0;
	std::for_each (std::begin(v), std::end(v), [&](const double d) {
		accum += (d - mean) * (d - mean);
	});

	stdev = sqrt(accum / (v.size()-1));
}


void ShakeGestureDetector::onFrame(const Controller & controller)
{
	static int sampleId = 0;

	Frame frame = controller.frame();

	if (frame.id() == lastFrameId)
		return;

	lastFrameId = frame.id();


	//Hand h = frame.hands().rightmost();	

	//Hand lastHand = controller.frame(1).hand(h.id());
	//

	//if (lastHand.isValid() && h.isValid())
	//{
	//	float sample = lastHand.palmNormal().angleTo(h.palmNormal())*2000;
	//	int key = 0;//h.id();

	//		lastSampleMap[key].push_back(VelocitySample(Vector(),sample,frame.timestamp(), sampleId++));
	//	if (lastSampleMap[key].size() > sampleCount)
	//	{
	//		if (DrawingEnabled)
	//		{
	//			auto e =drawZeros.find(lastSampleMap[key].front().SampleId);
	//			if (e != drawZeros.end())
	//				drawZeros.erase(e);
	//		}
	//		lastSampleMap[key].pop_front();

	//		calculateZeroCrossingsForAngle(key, lastSampleMap[key]);

	//	}
	//}
	//

	if (cooldownTimer.elapsed() && completedGestures.empty())
	{
		for (int i = 0;i < frame.pointables().count();i++)
		{				
			Pointable p = frame.pointables()[i];

			int minPointables = GlobalConfig::tree()->get<int>("Shake.MinimumPointables");
			if (minPointables > 1)
			{
				if (!p.hand().isValid() || p.hand().pointables().count() < minPointables)
					continue;
			}
			
			if (p.isValid())// && p.timeVisible() > GlobalConfig::tree()->get<float>("Shake.MinTimeVisible")) // && p.tipVelocity().magnitude() > GlobalConfig::tree()->get("Shake.FFT.TriggerVelocity",20.0f))
			{
				float sample;
				if (GlobalConfig::tree()->get("Shake.CancelDirectional", true))
					sample = p.tipVelocity().cross(p.direction()).magnitude();
				else
					sample = p.tipVelocity().magnitude();

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

					calculateFFT(key, lastSampleMap[key]);
					
					if (completedGestures.size() > 0)
					{
						lastSampleMap.clear();
						break;
					}
				}
			}
		}
	}
}


void ShakeGestureDetector::draw()
{
	if (DrawingEnabled) 
	{
		static Color colors [] = {Colors::Red,Colors::Blue,Colors::Orange,Colors::HoloBlueBright,Colors::LeapGreen};

		glBindTexture(GL_TEXTURE_2D,NULL);

		queue<pair<float,float> > zeroQueue;

		float originY = 200;

		glColor4fv(Colors::Red.getFloat());
		glLineWidth(1);
		glBegin(GL_LINE_STRIP);
		glVertex3f(0,originY,11);
		glVertex3f(GlobalConfig::ScreenWidth,originY,11);
		glEnd();

		for (auto it = lastSampleMap.begin(); it != lastSampleMap.end();it++)
		{
			float x = 0;
			glColor4fv(colors[(it->first)%5].getFloat());

			glLineWidth(3);
			glBegin(GL_LINE_STRIP);

			float maxV = GlobalConfig::ScreenHeight*.6f;

			for (auto point = it->second.begin(); point != it->second.end(); point++)
			{
				float xD = ((x++)*(GlobalConfig::ScreenWidth/sampleCount)*.4f) + 200;
				float yD = (point->Value*(GlobalConfig::ScreenHeight/maxV)*.5f)+originY;
				glVertex3f(xD,yD,10);

				if (drawZeros.count(point->SampleId) != 0)
				{
					zeroQueue.push(make_pair(xD,yD));
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

