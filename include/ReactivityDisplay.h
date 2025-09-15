#pragma once
#include <nanogui/common.h>
#include <nanogui/widget.h>
#include <Simulator.h>
#include <iostream>
#include <algorithm>    // std::max

class PeriodDisplay : public nanogui::Widget
{
public:
	PeriodDisplay(Widget* parent);
	~PeriodDisplay();

	virtual nanogui::Vector2i preferredSize(NVGcontext* ctx) const override;
	virtual void draw(NVGcontext* ctx) override;
	void setPeriod(double period_) { period = period_; }
protected:
	std::string mFontFace = "sans-bold";
	nanogui::Color mTextColor = nanogui::Color(200, 255);
	nanogui::Color mTextDisabledColor = nanogui::Color(120, 255);
	float textFontSize = 22.f;
	float mSpacing = 20.f;
	float mPadding = 15.f;
	double period;
private:
	float posFromPeriod(float period_);
	char* axesValues[6] = { "-30", "inf", "30", "7", "5", "3" };
	float axesLocations[6] = { 1.17f, 1.f, 0.83f, 0.373f, 0.270f, 0.10f };
};
