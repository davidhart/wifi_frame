#include "colorcurve.h"

ColorCurve::ColorCurve(const CurvePoint* pPoints, int count)
{
  m_pPoints = pPoints;
  m_pointCount = count;
  m_totalWidth = 0;
  for (int i = 0; i < count; i++)
  {
    m_totalWidth += pPoints[i].width;
  }
}

void ColorCurve::sample(int point, Color& colorOut) const
{
  int x = (point * m_totalWidth) / 255;
  int i = 0;
  for (; i < m_pointCount; i++)
  {
    if (x < m_pPoints[i].width)
    {
      break;
    }

    x -= m_pPoints[i].width;
  }

  if (x < 0 || i >= m_pointCount - 1)
  {
    colorOut = m_pPoints[m_pointCount - 1].color;
  }
  else
  {
    colorOut = blend(m_pPoints[i].color, m_pPoints[i+1].color, (x * 255) / m_pPoints[i].width);
  }
}
