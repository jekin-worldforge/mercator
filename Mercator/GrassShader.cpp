// This file may be redistributed and modified only under the terms of
// the GNU General Public License (See COPYING for details).
// Copyright (C) 2003 Alistair Riddoch

#include <Mercator/GrassShader.h>

#include <Mercator/Segment.h>
#include <Mercator/Surface.h>

namespace Mercator {

GrassShader::GrassShader(float lowThreshold, float highThreshold,
                         float cutoff, float intercept) : Shader(true, true),
             m_lowThreshold(lowThreshold), m_highThreshold(highThreshold),
             m_cutoff(cutoff), m_intercept(intercept)
{
}

GrassShader::~GrassShader()
{
}

inline ColorT GrassShader::slopeToAlpha(float height, float slope) const
{
    if ((height < m_lowThreshold) ||
        (height > m_highThreshold) ||
        (slope > m_intercept)) {
        return colorMin;
    } else if (slope < m_cutoff) {
        return colorMax;
    } else {
        return (ColorT)(colorMax * ((slope - m_cutoff) / (m_intercept - m_cutoff)));
    }
}

static const unsigned int chanAlpha = 3;

void GrassShader::shade(Surface & s) const
{
    unsigned int channels = s.getChannels();
    assert(channels > 3);
    Segment & seg = s.getSegment();
    ColorT * data = s.getData();
    const float * height_data = seg.getPoints();
    if (height_data == 0) {
        std::cerr << "WARNING: Mercator: Attempting to shade empty segment."
                  << std::endl << std::flush;
        return;
    }
    unsigned int size = seg.getSize();
    unsigned int res = seg.getResolution();

    unsigned int data_count = size * size * channels;
    for (unsigned int i = 0; i < data_count; ++i) {
        data[i] = colorMax;
    }

    // Deal with corner points
    // s(0, 0, chanAlpha) = slopeToAlpha(seg.get(0,0), 0.f);
    s(res, 0, chanAlpha) = slopeToAlpha(seg.get(res,0), 0.f);
    s(0, res, chanAlpha) = slopeToAlpha(seg.get(0,res), 0.f);
    s(res, res, chanAlpha) = slopeToAlpha(seg.get(res,res), 0.f);

    for (unsigned int i = 1; i < res; ++i) {
        float height = seg.get(i, 0);
        float avgSlope = (fabsf(seg.get(i - 1, 0) - height) +
                          fabsf(seg.get(i + 1, 0) - height)) / 2.f;
        s(i, 0, chanAlpha) = slopeToAlpha(height, avgSlope);

        height = seg.get(i, res);
        avgSlope = (fabsf(seg.get(i - 1, res) - height) +
                    fabsf(seg.get(i + 1, res) - height)) / 2.f;
        s(i, res, chanAlpha) = slopeToAlpha(height, avgSlope);

        height = seg.get(0, i);
        avgSlope = (fabsf(seg.get(0, i - 1) - height) +
                    fabsf(seg.get(0, i + 1) - height)) / 2.f;
        s(0, i, chanAlpha) = slopeToAlpha(height, avgSlope);

        height = seg.get(res, i);
        avgSlope = (fabsf(seg.get(res, i - 1) - height) +
                    fabsf(seg.get(res, i + 1) - height)) / 2.f;
        s(res, i, chanAlpha) = slopeToAlpha(height, avgSlope);
        for (unsigned int j = 1; j < res; ++j) {
            height = seg.get(i, j);
            avgSlope = (fabsf(seg.get(i + 1, j    ) - height) +
                        fabsf(seg.get(i    , j + 1) - height) +
                        fabsf(seg.get(i - 1, j    ) - height) +
                        fabsf(seg.get(i    , j - 1) - height)) / 4.f;
            s(i, j, chanAlpha) = slopeToAlpha(height, avgSlope);
        }
    }
}

} // namespace Mercator