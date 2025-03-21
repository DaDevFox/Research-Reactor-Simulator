#include "../include/CustomWidget.h"
#include <nanogui/layout.h>
#include <nanogui/theme.h>
#include <nanogui/window.h>
#include <nanogui/opengl.h>
#include <nanogui/screen.h>
#include <iostream>
#include <nanogui/serializer/core.h>
using namespace nanogui;

CustomWidget::CustomWidget(Widget* parent)
	: Widget(parent)
{}

CustomWidget::~CustomWidget()
{
	// TODO: determine if this is necessary
	//for (auto child : mChildren)
	//{
	//	if (child)
	//		child->decRef();
	//}
}

float CustomWidget::fontSize() const
{
	return (mFontSize < 0 && mTheme) ? mTheme->mStandardFontSize : mFontSize;
}

Widget* CustomWidget::findWidget(const Vector2i& p)
{
	for (auto it = mChildren.rbegin(); it != mChildren.rend(); ++it)
	{
		Widget* child = *it;
		if (child->visible() && child->contains(p - mPos))
			return child->findWidget(p - mPos);
	}
	return contains(p) ? this : nullptr;
}

Screen* CustomWidget::parentScreen()
{
	Screen* screen = dynamic_cast<Screen*>(parent());
	if (screen)
	{
		return screen;
	}
	else
	{
		return window()->screen();
	}
}

void CustomWidget::draw(NVGcontext* ctx)
{
	if (drawBackground)
	{
		nvgFillColor(ctx, mBackgroundColor);
		nvgBeginPath(ctx);
		nvgRect(ctx, mPos.x(), mPos.y(), mSize.x(), mSize.y());
		nvgFill(ctx);
	}

	if (mBorder != 0)
	{
		nvgStrokeColor(ctx, mBorderColor);
		nvgStrokeWidth(ctx, mBorderWidth);
		if (mBorder & Border::LEFT)
		{
			nvgBeginPath(ctx);
			nvgMoveTo(ctx, mPos.x(), mPos.y());
			nvgLineTo(ctx, mPos.x(), mPos.y() + mSize.y());
			nvgStroke(ctx);
		}
		if (mBorder & Border::TOP)
		{
			nvgBeginPath(ctx);
			nvgMoveTo(ctx, mPos.x(), mPos.y());
			nvgLineTo(ctx, mPos.x() + mSize.x(), mPos.y());
			nvgStroke(ctx);
		}
		if (mBorder & Border::RIGHT)
		{
			nvgBeginPath(ctx);
			nvgMoveTo(ctx, mPos.x() + mSize.x(), mPos.y());
			nvgLineTo(ctx, mPos.x() + mSize.x(), mPos.y() + mSize.y());
			nvgStroke(ctx);
		}
		if (mBorder & Border::BOTTOM)
		{
			nvgBeginPath(ctx);
			nvgMoveTo(ctx, mPos.x(), mPos.y() + mSize.y());
			nvgLineTo(ctx, mPos.x() + mSize.x(), mPos.y() + mSize.y());
			nvgStroke(ctx);
		}
	}

#if NANOGUI_SHOW_WIDGET_BOUNDS
	nvgStrokeWidth(ctx, 1.0f);
	nvgBeginPath(ctx);
	nvgRect(ctx, mPos.x() - 0.5f, mPos.y() - 0.5f, mSize.x() + 1, mSize.y() + 1);
	nvgStrokeColor(ctx, nvgRGBA(255, 0, 0, 255));
	nvgStroke(ctx);
#endif

	if (mChildren.empty())
		return;

	nvgTranslate(ctx, mPos.x(), mPos.y());
	for (Widget* child : mChildren)
		if (child->visible())
			child->draw(ctx);
	nvgTranslate(ctx, -mPos.x(), -mPos.y());
}
