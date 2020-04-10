#pragma once

#include "color.h"

struct CurvePoint
{
	int width;
	Color color;
};

class ColorCurve
{
public:
  ColorCurve(const CurvePoint* pPoints, int count);
  void sample(int point, Color& colorOut) const;

private:
  const CurvePoint* m_pPoints;
  int m_pointCount;
  int m_totalWidth;
};
