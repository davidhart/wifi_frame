#include "sunclock.h"
#include "colorcurve.h"
#include <algorithm>
#include <time.h>

using namespace std;

const CurvePoint sunsetGradient[] = {
  {40, {255, 180, 0}}, // Yellow
  {25, {255, 15, 15}}, // RedIsh
  {75, {38, 36, 120}}, // Blue
  {0,  {38, 36, 120}},   // Blue
};

const CurvePoint dayGradient[] = {
  {40,  {255, 255, 0}}, // Yellow
  {140, {128, 128, 255}}, // Light Blue
  {0,   {128, 128, 255}}, // Light Blue
};

const CurvePoint nightGradient[] = {
  {40, {38, 36, 120}}, // Blue
  {75, {10, 10, 80}}, // Light Blue
  {75, {0, 0, 0}}, // Light Blue
};

const ColorCurve sunsetCurve(sunsetGradient, sizeof(sunsetGradient) / sizeof(sunsetGradient[0]));
const ColorCurve dayCurve(dayGradient, sizeof(dayGradient) / sizeof(dayGradient[0]));
const ColorCurve nightCurve(nightGradient, sizeof(nightGradient) / sizeof(nightGradient[0]));

int hoursToSeconds(int hours)
{
	return hours * 60 * 60;
}
int secondsPassed(struct tm* timeNow, int hour)
{
	return (hour - timeNow->tm_hour) * 60 * 60 +
		timeNow->tm_min * 60 +
		timeNow->tm_sec;
}

void sunClockTimeToPhaseAndProgress(struct tm* timeNow, int& clockPhase, float& clockProgress)
{
	// Phase 0 - Night = 0-6
	// Phase 1 - Sunrise = 6-9
	// Phase 2 - Day = 9-21
	// Phase 3 - Sunset = 21-24
	if (timeNow->tm_hour >= 6 && timeNow->tm_hour < 9)
	{
		clockPhase = 1;
		clockProgress = secondsPassed(timeNow, 6) / (float)hoursToSeconds(9 - 6);
	}
	else if (timeNow->tm_hour >= 9 && timeNow->tm_hour < 21)
	{
		clockPhase = 2;
		clockProgress = secondsPassed(timeNow, 9) / (float)hoursToSeconds(21 - 9);
	}
	else if (timeNow->tm_hour >= 21)
	{
		clockPhase = 3;
		clockProgress = secondsPassed(timeNow, 21) / (float)hoursToSeconds(24 - 21);
	}
	else
	{
		clockPhase = 0;
		clockProgress = secondsPassed(timeNow, 0) / (float)hoursToSeconds(6);
	}
}

// Progress is time in section as fraction between 0 - 1
void sampleSunClock(int led, int ledCount, int phase, float progress, Color& colorOut)
{
	float phaseElapsed = progress;
	int step = 512 / ledCount;
	
	// Night
	if (phase == 0)
	{
		int pos = 6;
		float ledPos = (float)led;
		int dist = min(min(abs(ledPos - (pos+ledCount)), abs(ledPos - pos)), abs(ledPos - (pos - ledCount)));
		int sample = (int)(step * dist);
		nightCurve.sample(sample, colorOut);
	}
	// Sunrise
	else if (phase == 1)
	{
		int blendFactor = (int)(255 * phaseElapsed);
		int pos = 6;

		float ledPos = (float)led;
		int dist = min(min(abs(ledPos - (pos+ledCount)), abs(ledPos - pos)), abs(ledPos - (pos - ledCount)));
		int sample = (int)(step * dist);

		Color sunset;
		sunsetCurve.sample(sample, sunset);

		Color night;
		nightCurve.sample(sample, night);

		colorOut = blend(night, sunset, blendFactor);
	}
	// Day
	else if (phase == 2)
	{
		float pos = 6.0f + (float)ledCount * phaseElapsed;
		int dayBlend = (int)(255.0f - (255.0f * min(abs(pos - 30.0f), abs(pos + ledCount - 30.0f)) / 24.0f));

  		float ledPos = (float)led;
  		float dist = min(min(abs(ledPos - (pos+(float)ledCount)), abs(ledPos - pos)), abs(ledPos - (pos -(float)ledCount)));
  
  		int sample = (int)(step * dist);

		Color sunset;
		sunsetCurve.sample(sample, sunset);

		Color day;
		dayCurve.sample(sample, day);

		if (dayBlend <= 0)
	    	colorOut = sunset;
		else if (dayBlend >= 255)
	    	colorOut = day;
	  	else
	    	colorOut = blend(sunset, day, dayBlend);
	}
	// Sunset
	else if (phase == 3)
	{
		int blendFactor = (int)(255 * phaseElapsed);
		int pos = 6;

 		float ledPos = (float)led;
  		int dist = min(min(abs(ledPos - (pos+ledCount)), abs(ledPos - pos)), abs(ledPos - (pos - ledCount)));
  		int sample = (int)(step * dist);

  		Color sunset;
  		sunsetCurve.sample(sample, sunset);

  		Color night;
  		nightCurve.sample(sample, night);

		colorOut = blend(sunset, night, blendFactor);
	}
}