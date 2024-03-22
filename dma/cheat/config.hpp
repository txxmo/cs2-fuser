#pragma once

inline struct configA
{
	bool teamCheck = true; // deathmatch mode
	bool visCheck = false;

	int aimbotSpeed = 2; // lower = faster
	int aimbotSmooth = 1;
	int aimbotCurve = 0;
	int aimbotFov = 350;
	int aimbotKey = 0;
	int aimbotBone = 0;

	bool drawFov = true;
	bool drawAimPoint = true;

	bool skeletonESP = true;
	bool headESP = true;
	bool nameESP = true;
	
}config;