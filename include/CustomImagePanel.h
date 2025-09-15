
#pragma once

#include <nanogui/widget.h>

class CustomImagePanel : public nanogui::Widget
{
public:
	typedef std::vector<std::pair<int, std::string>> Images;
public:
	CustomImagePanel(Widget* parent);

	void setImages(const Images& data) { mImages = data; }
	const Images& images() const { return mImages; }

	std::function<void(int)> callback() const { return mCallback; }
	void setCallback(const std::function<void(int)>& callback) { mCallback = callback; }

	virtual bool mouseMotionEvent(const nanogui::Vector2i& p, const nanogui::Vector2i& rel, int button, int modifiers) override;
	virtual bool mouseButtonEvent(const nanogui::Vector2i& p, int button, bool down, int modifiers) override;
	virtual nanogui::Vector2i preferredSize(NVGcontext* ctx) const override;
	virtual void draw(NVGcontext* ctx) override;
protected:
	nanogui::Vector2i gridSize() const;
	int indexForPosition(const nanogui::Vector2i& p) const;
protected:
	Images mImages;
	std::function<void(int)> mCallback;
	int mThumbSize;
	int mSpacing;
	int mMargin;
	int mMouseIndex;
};

