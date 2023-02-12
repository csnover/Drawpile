// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: Calle Laakkonen

#include "libclient/tools/strokesmoother.h"

StrokeSmoother::StrokeSmoother()
{
	reset();
}

void StrokeSmoother::setSmoothing(int strength)
{
	Q_ASSERT(strength>0);
	_points.resize(strength);
	reset();
}

void StrokeSmoother::addOffset(const QPointF &offset)
{
	for(int i=0;i<_points.size();++i)
		_points[i] += offset;
}

void StrokeSmoother::addPoint(const canvas::Point &point)
{
	Q_ASSERT(_points.size()>0);

	if(_count == 0) {
		/* Pad the buffer with this point, so we blend away from it
		 * gradually as we gain more points. We still only count this
		 * as one point so we know how much real data we have to
		 * drain if it was a very short stroke. */
		_points.fill(point);
	} else {
		if(--_pos < 0)
			_pos = _points.size()-1;
		_points[_pos] = point;
	}

	if(_count < _points.size())
		++_count;
}

canvas::Point StrokeSmoother::at(int i) const
{
	return _points.at((_pos+i) % _points.size());
}

void StrokeSmoother::reset()
{
	_count=0;
	_pos = 0;
}

bool StrokeSmoother::hasSmoothPoint() const
{
	return _count > 0;
}

canvas::Point StrokeSmoother::smoothPoint() const
{
	Q_ASSERT(hasSmoothPoint());

	// A simple unweighted sliding-average smoother
	auto p = at(0);

	qreal pressure = p.pressure();
	for(int i=1;i<_points.size();++i) {
		const auto pi = at(i);
		p.rx() += pi.x();
		p.ry() += pi.y();
		pressure += pi.pressure();
	}

	const qreal c = _points.size();
	p.rx() /= c;
	p.ry() /= c;
	p.setPressure(pressure / c);

	return p;
}

void StrokeSmoother::removePoint()
{
	Q_ASSERT(!_points.isEmpty());
	/* Pad the buffer with the final point, overwriting the oldest first,
	 * for symmetry with starting. For very short strokes this should
	 * really set all points between --_count and _points.size()-1. */
	_points[(_pos + --_count) % _points.size()] = latestPoint();
}
