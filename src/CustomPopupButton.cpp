
#include "../include/CustomPopupButton.h"
#include <nanogui/entypo.h>
#include <nanogui/theme.h>
#include <nanogui/opengl.h>
#include <nanogui/serializer/core.h>
#include <iostream>
#include <nanovg/src/nanovg.h>

using namespace nanogui;

CustomPopupButton::CustomPopupButton(Widget* parent, const std::string& caption,
	int buttonIcon, int chevronIcon)
	: Button(parent, caption, buttonIcon), mChevronIcon(chevronIcon)
{

	setFlags(Flags::ToggleButton | Flags::PopupButton);

	Window* parentWindow = window();
	mPopup = new Popup(parentWindow->parent(), parentWindow);
	mPopup->setVisible(false);
}

Vector2i CustomPopupButton::preferredSize(NVGcontext* ctx) const
{
	return Button::preferredSize(ctx) + Vector2i(15, 0);
}

void CustomPopupButton::draw(NVGcontext* ctx)
{
	if (!mEnabled && mPushed)
		mPushed = false;

	if (!mPopup->visible() && mPushed) mPopup->performLayout(ctx);

	mPopup->setVisible(mPushed);
	if (mMoveWindow)
	{
		Vector2i posPopup = absolutePosition();
		int diff = (posPopup + mSize + mPopup->size()).x() + 15 - window()->size().x();
		if (diff > 0)
		{
			mPopup->setAnchorPos(Vector2i(posPopup.x() + mSize.x() + 15 - diff,
				posPopup.y() + mSize.y() / 2));
		}
		else
		{
			mPopup->setAnchorPos(Vector2i(posPopup.x() + mSize.x() + 15,
				posPopup.y() + mSize.y() / 2));
		}
	}
	Button::draw(ctx);

	if (mChevronIcon)
	{
		auto icon = utf8(mChevronIcon);
		NVGcolor textColor =
			mTextColor.w() == 0 ? mTheme->mTextColor : mTextColor;

		nvgFontSize(ctx, (mFontSize < 0 ? mTheme->mButtonFontSize : mFontSize) * 1.5f);
		nvgFontFace(ctx, "icons");
		nvgFillColor(ctx, mEnabled ? textColor : mTheme->mDisabledTextColor);
		nvgTextAlign(ctx, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE);

		float iw = nvgTextBounds(ctx, 0, 0, icon.data(), nullptr, nullptr);
		Vector2f iconPos(mPos.x() + mSize.x() - iw - 8,
			mPos.y() + mSize.y() * 0.5f - 1);

		nvgText(ctx, iconPos.x(), iconPos.y(), icon.data(), nullptr);
	}
}

void CustomPopupButton::performLayout(NVGcontext* ctx)
{
	Widget::performLayout(ctx);

	const Window* parentWindow = window();

	if (mMoveWindow)
	{
		Vector2i posPopup = absolutePosition();
		int diff = (posPopup + mSize + mPopup->size()).x() + 15 - window()->size().x();
		if (diff > 0)
		{
			mPopup->setAnchorPos(Vector2i(posPopup.x() + mSize.x() + 15 - diff,
				posPopup.y() + mSize.y() / 2));
		}
		else
		{
			mPopup->setAnchorPos(Vector2i(posPopup.x() + mSize.x() + 15,
				posPopup.y() + mSize.y() / 2));
		};
	}
	else
	{
		mPopup->setAnchorPos(Vector2i(parentWindow->width() + 15,
			absolutePosition().y() + mSize.y() / 2));
	}
}

void CustomPopupButton::save(Serializer& s) const
{
	Button::save(s);
	s.set("chevronIcon", mChevronIcon);
}

bool CustomPopupButton::load(Serializer& s)
{
	if (!Button::load(s))
		return false;
	if (!s.get("chevronIcon", mChevronIcon))
		return false;
	return true;
}

