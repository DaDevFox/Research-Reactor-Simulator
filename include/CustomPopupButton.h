
#pragma once

#include <nanogui/button.h>
#include <nanogui/popup.h>
#include <nanogui/entypo.h>


class CustomPopupButton : public nanogui::Button
{
public:
	CustomPopupButton(Widget* parent, const std::string& caption = "Untitled",
		int buttonIcon = 0,
		int chevronIcon = ENTYPO_ICON_CHEVRON_SMALL_RIGHT);

	void setChevronIcon(int icon) { mChevronIcon = icon; }
	int chevronIcon() const { return mChevronIcon; }

	nanogui::Popup* popup() { return mPopup; }
	const nanogui::Popup* popup() const { return mPopup; }

	void setMoveWindow(bool move) { mMoveWindow = move; }
	bool moveWindow() { return mMoveWindow; }

	virtual void draw(NVGcontext* ctx) override;
	virtual nanogui::Vector2i preferredSize(NVGcontext* ctx) const override;
	virtual void performLayout(NVGcontext* ctx) override;

	virtual void save(nanogui::Serializer& s) const override;
	virtual bool load(nanogui::Serializer& s) override;
protected:
	nanogui::Popup* mPopup;
	int mChevronIcon;
	bool mMoveWindow = false;
};

