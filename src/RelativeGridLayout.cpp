#include "../include/RelativeGridLayout.h"
#include <nanogui/window.h>
#include <nanogui/theme.h>
#include <nanogui/label.h>
#include <numeric>
#include <iostream>

using namespace nanogui;

Vector2i RelativeGridLayout::preferredSize(NVGcontext* ctx, const Widget* widget) const
{
	int* tempTable[2];
	for (size_t axis = 0; axis < 2; axis++)
	{
		tempTable[axis] = new int[mTable[axis].size()];
		memset(tempTable[axis], 0, mTable[axis].size() * sizeof(int));
	}


	Anchor a;
	Vector2i ps;
	for (Widget* w : widget->children())
	{
		if (!w->visible()) continue;
		a = anchor(w);
		ps = w->preferredSize(ctx);
		for (size_t axis = 0; axis < 2; axis++)
		{
			if (mTable[axis][a.pos[axis]].type() == SizeType::Relative)
			{
				tempTable[axis][a.pos[axis]] = std::max(tempTable[axis][a.pos[axis]], ps[axis]);
			}
			else
			{
				continue;
			}
		}
	}



	int relativeFill[2] = { 0,0 };
	for (size_t axis = 0; axis < 2; axis++)
	{
		// Sum of all preferred sizes
		for (size_t i = 0; i < mTable[axis].size(); i++)
		{
			relativeFill[axis] += tempTable[axis][i];
		}

		// Free memory
		delete[] tempTable[axis];
	}



	float yOffset = 0.f;
	if (dynamic_cast<const Window*>(widget)) yOffset += widget->theme()->mWindowHeaderHeight - mMargin / 2.f;
	return Vector2i(fixedSum[0] + relativeFill[0], fixedSum[1] + relativeFill[1] + (int)std::roundf(yOffset));
}

void RelativeGridLayout::performLayout(NVGcontext* ctx, Widget* widget) const
{
	for (Widget* w : widget->children())
	{

		if (!w->visible())
			continue;
		Anchor anchor = this->anchor(w);
		if (anchor.pos[0] < 0 || anchor.pos[1] < 0) continue;
		Vector4i bounds = boundsOf(widget, anchor.pos[0], anchor.pos[1], anchor.span[0], anchor.span[1]);
		Vector2i usePos = Vector2i();
		Vector2i useSize = Vector2i();
		Vector2i ps = w->preferredSize(ctx);
		Vector2i fs = w->fixedSize();

		for (size_t axis = 0; axis < 2; axis++)
		{

			if (anchor.alignment[axis] != Alignment::Fill)
			{

				useSize[axis] = fs[axis] ? fs[axis] : ps[axis];
			}

			switch (anchor.alignment[axis])
			{

			case Alignment::Minimum:
				usePos[axis] = bounds[axis] + anchor.padding[axis];
				break;
			case Alignment::Middle:
				usePos[axis] = bounds[axis] + (bounds[axis + 2] - useSize[axis]) / 2 + anchor.padding[axis];
				break;
			case Alignment::Maximum:
				usePos[axis] = bounds[axis] + (bounds[axis + 2] - useSize[axis]) - anchor.padding[2 + axis];
				break;
			case Alignment::Fill:
				usePos[axis] = bounds[axis] + anchor.padding[axis];
				switch (anchor.expansion[axis])
				{

				case FillMode::Always:
					useSize[axis] = bounds[axis + 2] - anchor.padding[axis] - anchor.padding[2 + axis];
					break;
				case FillMode::IfLess:
					if (ps[axis] < bounds[axis + 2] - anchor.padding[axis] - anchor.padding[2 + axis])
					{

						useSize[axis] = bounds[axis + 2] - anchor.padding[axis] - anchor.padding[2 + axis];
					}

					else
					{
						useSize[axis] = ps[axis];
					}
					break;
				case FillMode::IfMore:
					if (ps[axis] > bounds[axis + 2] - anchor.padding[axis] - anchor.padding[2 + axis])
					{
						useSize[axis] = bounds[axis + 2] - anchor.padding[axis] - anchor.padding[2 + axis];
					}
					else
					{
						useSize[axis] = ps[axis];
					}
					break;
				}
				break;
			}
		}
		w->setPosition(usePos);
		w->setSize(useSize);
		w->performLayout(ctx);

	}
}

RelativeGridLayout::Anchor RelativeGridLayout::makeAnchor(int x, int y, int w, int h, Alignment alignX, Alignment alignY, FillMode modeX, FillMode modeY)
{
	Anchor a = Anchor(x, y, w, h);
	a.setAlignment(alignX, alignY);
	a.setFillMode(modeX, modeY);
	return a;
}

Vector2f RelativeGridLayout::positionOf(const Widget* widget, size_t x, size_t y) const
{
	Vector2f accumulateFixed = Vector2f::Constant(0.f);
	Vector2f accumulateRelative = Vector2f::Constant(0.f);
	for (size_t axis = 0; axis < 2; axis++)
	{

		size_t limit = axis ? y : x;
		limit = std::min(limit, mTable[axis].size());
		for (size_t i = 0; i < limit; i++)
		{
			if (mTable[axis][i].type() == SizeType::Fixed)
				accumulateFixed[axis] += mTable[axis][i].value();
			else
				accumulateRelative[axis] += mTable[axis][i].value();
		}

		if (relativeSum[axis] != 0.f)
			accumulateRelative[axis] *= (float)(widget->size()[axis] - fixedSum[axis]) / relativeSum[axis];
	}

	return accumulateFixed + accumulateRelative;
}

Vector4i RelativeGridLayout::boundsOf(const Widget* widget, size_t x, size_t y, size_t w, size_t h) const
{
	Vector2f source = positionOf(widget, x, y);
	Vector2f end = positionOf(widget, x + w, y + h);
	return Vector4i((int)std::roundf(source.x()), (int)std::roundf(source.y()), (int)std::roundf(end.x() - source.x()), (int)std::roundf(end.y() - source.y()));
}
