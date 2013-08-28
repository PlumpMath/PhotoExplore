#include "HandModel.h"
#include "GlobalConfig.hpp"
#include "GLImport.h"
#include "LeapDebug.h"

HandProcessor * HandProcessor::getInstance()
{
	if (instance == NULL)
		instance = new HandProcessor();

	return instance;
}

HandModel * HandProcessor::LastModel(int handId)
{
	if (instance == NULL)
		instance = new HandProcessor();

	return instance->lastModel(handId);
}

HandModel * HandProcessor::LastModel()
{
	if (instance == NULL)
		instance = new HandProcessor();

	return instance->lastModel();
}


HandProcessor::HandProcessor()
{
	lastDominantHandId = -1;
	
	logData = GlobalConfig::tree()->get<bool>("Leap.HandModel.LogData");
	
	if (logData)
	{
		data.open("./hand_pose.csv");
		
		for (int i = 0;i < 5;i++)
		{
			data << "JointAngle_" << i << ",PalmDist_" << i << ",";
		}
		
		data << "HandSphereRadius,PalmDistRange,JointAngleRange,Result";
	}
	data << endl;
	
}


HandModel * HandProcessor::lastModel()
{
	if (modelMap.size() == 0 || modelMap.count(lastDominantHandId) == 0)
		return &defaultModel;

	return &modelMap[lastDominantHandId];
}


HandModel * HandProcessor::lastModel(int id)
{
	if (modelMap.size() == 0 || modelMap.count(id) == 0)
		return &defaultModel;

	return &modelMap[id];
}

void HandProcessor::processFrame(Frame frame)
{
	if (frame.id() == lastFrameId)
		return;
	
	if (frame.hands().count() == 0)
	{
		if (frame.pointables().count() > 0)
		{
			Pointable intent;
			if (frame.tools().count() > 0)
				intent = frame.tools().frontmost();
			else
				intent = frame.fingers().frontmost();
			
			modelMap[-2] = HandModel(-2,intent.id());
			lastDominantHandId = -2;			
		}
	}
	else
	{
		Hand dominantHand;
		if (GlobalConfig::LeftHanded)
			dominantHand = frame.hands().leftmost();
		else
			dominantHand = frame.hands().rightmost();

		drawHand = dominantHand;

		lastDominantHandId = dominantHand.id();	

		for (int i=0;i<frame.hands().count();i++)
		{
			Hand hand = frame.hands()[i];

			bool isLeft = frame.hands().count() >= 2 && frame.hands().leftmost().id() == hand.id();

			modelMap[hand.id()] = buildModel(hand, isLeft);
		}
	}
	lastFrameId = frame.id();
}


HandModel HandProcessor::buildComplexModel(Hand hand, bool isLeft)
{
	static float minThumbFingerSpacing = GlobalConfig::tree()->get<float>("Leap.HandModel.MinThumbFingerSpacing");
	static float minThumbFingerAngle = GlobalConfig::tree()->get<float>("Leap.HandModel.MinThumbFingerAngle")*GeomConstants::DegToRad;
	static float minThumbFingerDirOffset = GlobalConfig::tree()->get<float>("Leap.HandModel.MinThumbFingerDirectionOffset")*GeomConstants::DegToRad;
	static bool enableAutoLeftDetection = GlobalConfig::tree()->get<bool>("Leap.HandModel.EnableAutoLeftDetection");
	
	static float minFingerHandDotProduct = GlobalConfig::tree()->get<float>("Leap.HandModel.MinFingerHandDotProduct");

	vector<pair<Finger,float> > fingerAngles;

	for (int i=0;i<hand.fingers().count();i++)
	{
		Finger f = hand.fingers()[i];

		if (f.direction().dot(hand.direction()) >= minFingerHandDotProduct)
			fingerAngles.push_back(make_pair(f,LeapHelper::GetFingerTipAngle(hand,f)));
	}

	std::sort(fingerAngles.begin(),fingerAngles.end(),[isLeft](const pair<Finger,float> & v0, const pair<Finger,float> & v1) -> bool {
		if (isLeft)
			return v1.second < v0.second;
		else
			return v0.second < v1.second;
	});


	HandModel handModel;

	if (fingerAngles.empty())
	{
		handModel.IntentFinger = hand.fingers().frontmost().id();
	}
	else if (fingerAngles.size() == 1)
	{
		handModel.IntentFinger = fingerAngles.at(0).first.id();
	}
	else if (fingerAngles.size() < 5)
	{
		auto f0 = fingerAngles.at(0);
		auto f1 = fingerAngles.at(1);

		float angleSpacing = abs(f0.second - f1.second);

		handModel.IntentFinger = f0.first.id(); //Assume thumb is not visible

		bool minAnglePassed = angleSpacing > minThumbFingerAngle;
		bool minDirectionOffsetPassed = f0.first.direction().angleTo(f1.first.direction()) > minThumbFingerDirOffset;

		Vector interFingerSpacing = f0.first.stabilizedTipPosition() - f1.first.stabilizedTipPosition();
		bool minSpacingPassed = abs(interFingerSpacing.dot(hand.direction())) > minThumbFingerSpacing;

		if ((minAnglePassed || minDirectionOffsetPassed) && minSpacingPassed)
		{			
			handModel.ThumbId = f0.first.id();
			handModel.IntentFinger = f1.first.id();
		}

		//If two fingers, always choose frontmost for intent
		if (handModel.ThumbId >= 0 && hand.fingers().count() == 2)
		{
			if (handModel.IntentFinger != hand.fingers().frontmost().id())
			{
				handModel.ThumbId = f1.first.id();
				handModel.IntentFinger = f0.first.id();
			}
		}	
	}	
	else
	{				
		if (!isLeft && enableAutoLeftDetection) //Might be a sole left hand
		{
			auto f3 = fingerAngles.at(3);
			auto f4 = fingerAngles.at(4); //Left thumb
			
			float angleSpacing = abs(f3.second - f4.second);
		
			if (angleSpacing > minThumbFingerAngle)
			{
				handModel.ThumbId = f4.first.id();
				handModel.IntentFinger = f3.first.id();
			}
			else
			{
				handModel.ThumbId = fingerAngles.at(0).first.id();
				handModel.IntentFinger = fingerAngles.at(1).first.id();
			}
		}
		else
		{
			handModel.ThumbId = fingerAngles.at(0).first.id();
			handModel.IntentFinger = fingerAngles.at(1).first.id();
		}
	}

	handModel.Pose = determineHandPose(hand,&handModel, fingerAngles);



	return handModel;
}

int HandProcessor::determineHandPose(Hand hand, HandModel * model, vector<pair<Finger,float> > & fingerAngles)
{	
	auto poseConfig = GlobalConfig::tree()->get_child("Leap.HandModel.Pose");

	int handState = HandModel::Invalid;

	if (!hand.isValid() || fingerAngles.size() == 1)
	{
		handState = HandModel::Pointing;
	}
	else if (fingerAngles.size() >= 2)
	{
		int i=0;

		Finger intentFinger = hand.finger(model->IntentFinger);
		float closestTip = 1000;

		for (; i < fingerAngles.size(); i++)
		{
			Finger f = fingerAngles.at(i).first;

			if (f.id() != model->ThumbId && f.id() != model->IntentFinger)
			{
				closestTip = min<float>(closestTip,f.stabilizedTipPosition().distanceTo(intentFinger.stabilizedTipPosition()));
			}
		}
				
		//Case 1 - 2 fingers, thumb
		if (fingerAngles.size() == 2 && model->ThumbId >= 0)
		{
			handState = HandModel::Pointing;
		}
		//Case 2 - 2+ fingers
		else
		{		
			float point_middleFingerDist = poseConfig.get<float>("Point.MinDistanceToMiddleFinger");

			float sphereRadiusThreshold = poseConfig.get<float>("SphereRadiusThreshold");		
			
			//LeapDebug::getInstance().showValue("01. Tip distance",closestTip); 
			//LeapDebug::getInstance().showValue("02. Sphere",hand.sphereRadius()); 

			if (hand.sphereRadius() > sphereRadiusThreshold || closestTip < point_middleFingerDist)
			{
				handState = HandModel::Spread;
			}
			else 
			{				
				handState = HandModel::Pointing;
			}
		}
	}

	if (logData) data << "," << handState << endl;

	return handState;	
}

HandModel HandProcessor::buildModel(Hand hand, bool invert)
{
	HandModel handModel;

	if (hand.pointables().count() == 0)
	{
		handModel = HandModel(hand.id());
	}
	else if (hand.tools().count() > 0)
	{
		Pointable intent = hand.tools().frontmost();
		handModel = HandModel(hand.id(),intent.id());
	}
	else if (hand.fingers().count() == 1)
	{
		Pointable intent = hand.fingers()[0];	
		handModel = HandModel(hand.id(),intent.id());
		handModel.Pose = HandModel::Pointing;
	}
	else
	{
		handModel = buildComplexModel(hand,invert);
	}
	handModel.HandId = hand.id();

	return handModel;
}

void HandProcessor::draw()
{
	if (GlobalConfig::tree()->get<bool>("Leap.HandModel.DebugDraw"))
	{
		Vector handCenter = Vector(500,300,100);
		glBindTexture(GL_TEXTURE_2D,NULL);
		glLineWidth(2);
		Pointable intentFinger = drawHand.pointable(lastModel()->IntentFinger);

		
		float minFingerHandDotProduct = GlobalConfig::tree()->get<float>("Leap.HandModel.MinFingerHandDotProduct");

		float scale = 5;

		for (int i=0;i< drawHand.fingers().count();i++)
		{
			Finger f = drawHand.fingers()[i];

			float angle = LeapHelper::GetFingerTipAngle(drawHand,f);

			Vector delta = f.stabilizedTipPosition() - drawHand.stabilizedPalmPosition();
			float jointAngle = drawHand.palmNormal().angleTo(delta)*sgn(drawHand.palmNormal().cross(delta).x);


			float length = 30 + LeapHelper::GetDistanceFromHand(f); //(jointAngle * GeomConstants::RadToDeg); 

			
			float alpha = 1.0f;

			if (f.direction().dot(f.hand().direction()) < minFingerHandDotProduct)
				alpha = 0.5f;

			if (intentFinger.id() == f.id())
			{
				glColor4fv(Colors::HoloBlueBright.withAlpha(alpha).getFloat());
			}
			else if (lastModel()->ThumbId == f.id())
			{
				glColor4fv(Colors::LeapGreen.withAlpha(alpha).getFloat());
			}
			else
				glColor4fv(Colors::OrangeRed.withAlpha(alpha).getFloat());
		
			Vector drawFinger = Vector(length * cos(angle), length * sin(angle), 0) + handCenter;

			glBegin(GL_LINES);
				glVertex3f(handCenter.x, handCenter.y, handCenter.z);
				glVertex3f(drawFinger.x, drawFinger.y, drawFinger.z);
			glEnd();

		}

	
		glColor4fv(Colors::Lime.withAlpha(0.6f).getFloat());
		float rad = drawHand.sphereRadius();
		int vertices = 16;
		float anglePerVertex = (Leap::PI*2.0f)/vertices;
		glBegin(GL_LINE_LOOP);
		for (float v=0;v<vertices;v++)
		{
			float angle = v*anglePerVertex;
			glVertex3f(handCenter.x + sinf(angle)*rad,handCenter.y + cosf(angle)*rad,handCenter.z);	
		}
		glEnd();
	}
}

