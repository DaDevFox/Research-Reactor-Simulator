#pragma once

#include <nanogui/widget.h>
#include <nanogui/opengl.h>

#define SCROLL_BAR_THICKNESS 12
#define SCROLL_BAR_ROUND .8f

/**
 * \brief Adds a vertical scrollbar around a widget that is too big to fit
 *        into a certain area. Based on original ijs-f8 VScrollPanel with
 *        background drawing support.
 */
class CustomVScrollPanel : public nanogui::Widget
{
public:
	CustomVScrollPanel(Widget *parent);

	/// Return the current scroll amount as a value between 0 and 1. 0 means scrolled to the top and 1 to the bottom.
	float scroll() const { return mScroll; }
	/// Set the scroll amount to a value between 0 and 1. 0 means scrolled to the top and 1 to the bottom.
	void setScroll(float scroll) { mScroll = scroll; }

	/// Background color support
	const nanogui::Color &backgroundColor() const { return mBackgroundColor; }
	void setBackgroundColor(const nanogui::Color &color) { mBackgroundColor = color; }

	bool getDrawBackground() const { return mDrawBackground; }
	void setDrawBackground(bool value) { mDrawBackground = value; }

	virtual void performLayout(NVGcontext *ctx) override;
	virtual nanogui::Vector2i preferredSize(NVGcontext *ctx) const override;
	virtual bool mouseDragEvent(const nanogui::Vector2i &p, const nanogui::Vector2i &rel, int button, int modifiers) override;
	virtual bool scrollEvent(const nanogui::Vector2i &p, const nanogui::Vector2f &rel) override;
	virtual bool mouseButtonEvent(const nanogui::Vector2i &p, int button, bool down, int modifiers) override;
	virtual bool mouseMotionEvent(const nanogui::Vector2i &p, const nanogui::Vector2i &rel, int button, int modifiers) override;
	virtual void draw(NVGcontext *ctx) override;
	virtual void save(nanogui::Serializer &s) const override;
	virtual bool load(nanogui::Serializer &s) override;
	nanogui::Vector2i absolutePosition() const;

protected:
	int getSizeDefect() const;

	int mChildPreferredHeight;
	float mScroll;
	nanogui::Color mBackgroundColor = nanogui::Color(0, 0);
	bool mDrawBackground = false;
};
