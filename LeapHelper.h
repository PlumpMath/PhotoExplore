#include <Leap.h>
#include <LeapMath.h>
#include <map>
#include <vector>
#include "Types.h"
#include "GlobalConfig.hpp"


#ifndef LeapHelper_H
#define LeapHelper_H

using namespace Leap;

class LeapHelper {

public:
	static double lowpass(double previous, double input, double RC, double dT)
	{
		double alpha = dT/(RC + dT);
		return alpha * input + (1.0 - alpha) * previous;
	}

	static Vector FindScreenPoint(const Controller & c, Pointable pointable, bool make2D = !false)
	{
		Screen screen;
		return FindScreenPoint(c, pointable, screen, make2D);
	}

	static Vector FindScreenPoint(const Controller & c, Pointable pointable, Screen &screen, bool make2D = !false)
	{
		return FindScreenPoint(c,pointable.stabilizedTipPosition(),pointable.direction(),screen,make2D);
	}

	static Vector FindScreenPoint(const Controller & c, Vector position, Vector direction, Screen &screen, bool make2D = !false)
	{
		Vector screenPoint = FindNormalizedScreenPoint(c, position,direction, screen);

		if (make2D)
		{
			float x = screen.widthPixels() * screenPoint.x;
			float y = screen.heightPixels() * (1.0f - screenPoint.y);
			return Vector(x, y, 0);
		}
		else
		{
			float x = screen.widthPixels() * screenPoint.x;
			float y = screen.heightPixels() * screenPoint.y;
			return Vector(x, y, 0);
		}
	}

	static Vector FindNormalizedScreenPoint(const Controller & c, Vector position, Vector direction, Screen & screen)
	{
		if (GlobalConfig::PreferredScreenIndex() < 0)
			screen = c.calibratedScreens().closestScreenHit(position,direction);
		else
			screen = c.calibratedScreens()[GlobalConfig::PreferredScreenIndex()];

		return screen.intersect(position,direction, true);
	}

	static Screen ClosestScreen(const Controller & c, Vector position)
	{
		if (GlobalConfig::PreferredScreenIndex() < 0)
			return c.calibratedScreens().closestScreen(position);
		else
			return c.calibratedScreens()[GlobalConfig::PreferredScreenIndex()];
	}


	static Vector FindClosestNormalizedScreenPoint(const Controller & c, Vector position, Screen & screen)
	{
		if (GlobalConfig::PreferredScreenIndex() < 0)
			screen = c.calibratedScreens().closestScreen(position);
		else
			screen = c.calibratedScreens()[GlobalConfig::PreferredScreenIndex()];

		return screen.project(position, true);
	}


	static Vector FindClosestScreenPoint(const Controller & c, Vector point, bool make2D = !false)
	{
		Screen screen;
		Vector screenPoint = FindClosestNormalizedScreenPoint(c, point, screen);

		if (make2D)
		{
			float x = screen.widthPixels() * screenPoint.x;
			float y = screen.heightPixels() * (1.0f - screenPoint.y);
			return Vector(x, y, 0);
		}
		else
		{
			float x = screen.widthPixels() * screenPoint.x;
			float y = screen.heightPixels() * screenPoint.y;
			return Vector(x, y, 0);
		}
	}


	static Vector GetHandPlaneProjection(Vector point, Hand hand)
	{
		Vector v = point;// -hand.palmPosition();
		float dist = v.dot(hand.palmNormal()) + GetHandPlaneConstant(hand);
		Vector proj = point - (dist * hand.palmNormal());
		return proj;
	}

	static float GetHandPlaneConstant(Hand hand)
	{
		Vector n = hand.palmNormal();
		Vector p = hand.palmPosition();

		return -(n.x * p.x + n.y * p.y + p.z * n.z);
	}

	static float GetDistanceFromHand(Finger finger)
	{
		Hand hand = finger.hand();
		Vector point = GetFingerBase(finger);
		return point.dot(hand.palmNormal()) + GetHandPlaneConstant(hand);
	}


	static float GetDistanceFromHand(Vector point, Hand hand)
	{
		return point.dot(hand.palmNormal()) + GetHandPlaneConstant(hand);
	}

	static Vector GetFingerBase(Finger finger)
	{
		return finger.stabilizedTipPosition() - (finger.direction() * finger.length());
	}

	static Vector NormalHandDirection(Hand hand)
	{
		Vector v = GetHandPlaneProjection(hand.palmPosition() + hand.direction(), hand) - hand.palmPosition();
		return (v / v.magnitude());
	}


	static float GetFingerTipAngle(Hand hand, Finger finger)
	{
		Vector fingerDirection = GetHandPlaneProjection(GetFingerBase(finger), hand) - hand.palmPosition();
		fingerDirection /= fingerDirection.magnitude();

		Vector refAngle = NormalHandDirection(hand);

		Vector cross = refAngle.cross(fingerDirection);


		float sign = sgn(hand.palmNormal().dot(cross));

		float angle = refAngle.angleTo(fingerDirection);
		angle *= sign;

		return angle;
	}

	static std::vector<int> * SortFingers(Hand hand)
	{
		std::multimap<float,int> sortMap;

		for (int i =0;i<hand.fingers().count();i++)
		{
			Finger finger = hand.fingers()[i];

			float result = GetFingerTipAngle(hand, finger);
			sortMap.insert(std::pair<float,int>(result,finger.id()));
		}

		std::vector<int> * fingerList = new std::vector<int>();
		auto keyIt = sortMap.begin();

		for (; keyIt != sortMap.end();keyIt++)
		{
			fingerList->push_back((*keyIt).second);
		}

		return fingerList;
	}
};
#endif