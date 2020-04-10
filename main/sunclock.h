#pragma once

class Color;

void sunClockTimeToPhaseAndProgress(struct tm* timeNow, int& clockPhase, float& clockProgress);
void sampleSunClock(int led, int ledCount, int phase, float progress, Color& colorOut);