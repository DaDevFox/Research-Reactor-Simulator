/*
	src/window.cpp -- Top-level window widget

	NanoGUI was developed by Wenzel Jakob <wenzel.jakob@epfl.ch>.
	The widget drawing code is based on the NanoVG demo application
	by Mikko Mononen.

	All rights reserved. Use of this source code is governed by a
	BSD-style license that can be found in the LICENSE.txt file.
*/

#include "../include/CustomWindow.h"
#include <nanogui/window.h>
#include <nanogui/theme.h>
#include <nanogui/opengl.h>
#include <nanogui/screen.h>
#include <nanogui/layout.h>
#include <nanogui/serializer/core.h>

using namespace nanogui;
using namespace std;

CustomWindow::CustomWindow(Widget *parent, const std::string &title)
	: Window(parent, title)
{
}

CustomWidget *CustomWindow::buttonPanel()
{
	if (!mButtonPanel)
	{
		mButtonPanel = new CustomWidget(this);
		mButtonPanel->setLayout(new BoxLayout(Orientation::Horizontal, Alignment::Middle, 0, 4));
	}
	return mButtonPanel;
}

void CustomWindow::draw(NVGcontext *ctx)
{
	int ds = mTheme->mWindowDropShadowSize, cr = mTheme->mWindowCornerRadius;
	int hh = mTheme->mWindowHeaderHeight;

	/* Draw window */
	nvgSave(ctx);
	nvgBeginPath(ctx);
	nvgRoundedRect(ctx, mPos.x(), mPos.y(), mSize.x(), mSize.y(), cr);

	nvgFillColor(ctx, mMouseFocus ? mTheme->mWindowFillFocused
								  : mTheme->mWindowFillUnfocused);
	nvgFill(ctx);

	/* Draw a drop shadow */
	NVGpaint shadowPaint = nvgBoxGradient(
		ctx, mPos.x(), mPos.y(), mSize.x(), mSize.y(), cr * 2, ds * 2,
		mTheme->mDropShadow, mTheme->mTransparent);

	nvgBeginPath(ctx);
	nvgRect(ctx, mPos.x() - ds, mPos.y() - ds, mSize.x() + 2 * ds, mSize.y() + 2 * ds);
	nvgRoundedRect(ctx, mPos.x(), mPos.y(), mSize.x(), mSize.y(), cr);
	nvgPathWinding(ctx, NVG_HOLE);
	nvgFillPaint(ctx, shadowPaint);
	nvgFill(ctx);

	if (!mTitle.empty())
	{
		/* Draw header */
		NVGpaint headerPaint = nvgLinearGradient(
			ctx, mPos.x(), mPos.y(), mPos.x(),
			mPos.y() + hh,
			mTheme->mWindowHeaderGradientTop,
			mTheme->mWindowHeaderGradientBot);

		nvgBeginPath(ctx);
		nvgRoundedRect(ctx, mPos.x(), mPos.y(), mSize.x(), hh, cr);

		nvgFillPaint(ctx, headerPaint);
		nvgFill(ctx);

		nvgBeginPath(ctx);
		nvgRoundedRect(ctx, mPos.x(), mPos.y(), mSize.x(), hh, cr);
		nvgStrokeColor(ctx, mTheme->mWindowHeaderSepTop);
		nvgScissor(ctx, mPos.x(), mPos.y(), mSize.x(), 0.5f);
		nvgStroke(ctx);
		nvgResetScissor(ctx);

		nvgBeginPath(ctx);
		nvgMoveTo(ctx, mPos.x(), mPos.y() + hh);
		nvgLineTo(ctx, mPos.x() + mSize.x(), mPos.y() + hh);
		nvgStrokeColor(ctx, mTheme->mWindowHeaderSepBot);
		nvgStroke(ctx);

		nvgFontSize(ctx, 18.0f);
		nvgFontFace(ctx, "sans-bold");
		nvgTextAlign(ctx, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);

		nvgFontBlur(ctx, 2.f);
		nvgFillColor(ctx, mTheme->mDropShadow);
		nvgText(ctx, mPos.x() + mSize.x() / 2.f,
				mPos.y() + hh / 2.f, mTitle.c_str(), nullptr);

		nvgFontBlur(ctx, 0);
		nvgFillColor(ctx, mFocused ? mTheme->mWindowTitleFocused
								   : mTheme->mWindowTitleUnfocused);
		nvgText(ctx, mPos.x() + mSize.x() / 2, mPos.y() + hh / 2 - 1,
				mTitle.c_str(), nullptr);
	}

	nvgRestore(ctx);
	nvgSave(ctx);
	nvgIntersectScissor(ctx, mPos.x() + mBorderThickness / 2.f, mPos.y() + mBorderThickness / 2.f, mSize.x() - mBorderThickness, mSize.y() - mBorderThickness);

	// TODO: decide usefulness of non-inheritance based implementation (below)
	// COPY OF CustomWidget::draw(ctx) for inheritance purposes

	// START special implementation

#pragma region special copied implementation

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
	for (Widget *child : mChildren)
		if (child->visible())
			child->draw(ctx);
	nvgTranslate(ctx, -mPos.x(), -mPos.y());

#pragma endregion

	nvgRestore(ctx);

	if (mBorder)
	{
		nvgBeginPath(ctx);
		nvgRoundedRect(ctx, mPos.x(), mPos.y(), mSize.x(), mSize.y(), cr);
		nvgStrokeColor(ctx, mBorderColor);
		nvgStrokeWidth(ctx, mBorderThickness);
		nvgStroke(ctx);
	}

	if (!mEnabled)
	{
		nvgBeginPath(ctx);
		nvgRoundedRect(ctx, mPos.x(), mPos.y(), mSize.x(), mSize.y(), cr);
		nvgFillColor(ctx, Color(0.1f, 0.7f));
		nvgFill(ctx);
	}
}

bool CustomWindow::mouseDragEvent(const Vector2i &, const Vector2i &rel,
								  int button, int /* modifiers */)
{
	if (!mEnabled)
		return false;
	if (mDrag && (button & (1 << GLFW_MOUSE_BUTTON_1)) != 0)
	{
		mPos += rel;
		mPos = mPos.cwiseMax(Vector2i::Zero());
		mPos = mPos.cwiseMin(parent()->size() - mSize);
		return true;
	}
	return false;
}

bool CustomWindow::mouseButtonEvent(const Vector2i &p, int button, bool down, int modifiers)
{
	if (!mEnabled)
		return false;
	if (Widget::mouseButtonEvent(p, button, down, modifiers))
		return true;
	if (button == GLFW_MOUSE_BUTTON_1)
	{
		mDrag = down && (p.y() - mPos.y()) < mTheme->mWindowHeaderHeight;
		return true;
	}
	return false;
}

bool CustomWindow::scrollEvent(const Vector2i &p, const Vector2f &rel)
{
	if (!mEnabled)
		return false;
	Widget::scrollEvent(p, rel);
	return true;
}

bool CustomWindow::mouseMotionEvent(const Vector2i &p, const Vector2i &rel, int button, int modifiers)
{
	if (mEnabled)
	{
		Widget::mouseMotionEvent(p, rel, button, modifiers);
		return true;
	}
	else
	{
		return false;
	}
}

bool CustomWindow::mouseEnterEvent(const Vector2i &p, bool enter)
{
	if (mEnabled)
	{
		Widget::mouseEnterEvent(p, enter);
		return true;
	}
	else
	{
		return false;
	}
}

void CustomWindow::setBorder(bool value)
{
	mBorder = value;
}

bool CustomWindow::border()
{
	return mBorder;
}

void CustomWindow::setBorderColor(Color value)
{
	mBorderColor = value;
}

Color CustomWindow::borderColor()
{
	return mBorderColor;
}

void CustomWindow::setBorderThickness(float value)
{
	mBorderThickness = value;
}

float CustomWindow::borderThickness()
{
	return mBorderThickness;
}

// CustomWidget reimplementation:

float CustomWindow::fontSize() const
{
	return (mFontSize < 0 && mTheme) ? mTheme->mStandardFontSize : mFontSize;
}

CustomTheme *CustomWindow::theme()
{
	CustomTheme *converted;
	auto test = mTheme.get();
	auto test2 = mCustomTheme.get();
	if (!mCustomTheme.get())
	{
		converted = dynamic_cast<CustomTheme *>(mTheme.get());
		if (converted)
			return converted;
		else
			return (mCustomTheme = new CustomTheme(*mTheme.get()));
	}
	return mCustomTheme.get();
}

const CustomTheme *CustomWindow::theme() const
{
	if (!mCustomTheme && typeid(*mTheme.get()) == typeid(CustomTheme))
		return (const CustomTheme *)mTheme.get();
	return mCustomTheme.get();
}

void CustomWindow::setTheme(CustomTheme *theme)
{
	if (mCustomTheme.get() == theme)
		return;
	mCustomTheme = theme;
	for (auto child : mChildren)
	{
		CustomWidget *converted;
		if ((converted = dynamic_cast<CustomWidget *>(child)) != nullptr)
			converted->setTheme(theme);
		else
			child->setTheme(theme);
	}
}

Widget *CustomWindow::findWidget(const Vector2i &p)
{
	for (auto it = mChildren.rbegin(); it != mChildren.rend(); ++it)
	{
		Widget *child = *it;
		if (child->visible() && child->contains(p - mPos))
			return child->findWidget(p - mPos);
	}
	return contains(p) ? this : nullptr;
}

Screen *CustomWindow::parentScreen()
{
	Screen *screen = dynamic_cast<Screen *>(parent());
	if (screen)
	{
		return screen;
	}
	else
	{
		return window()->screen();
	}
}
