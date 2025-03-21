#include <nanogui/widget.h>

class IntervalSlider : public nanogui::Widget
{
public:
	IntervalSlider(Widget* parent);

	// Returns two values representing the positions of the two borders
	float value(int i) const { return mValue[i]; }
	void setValue(int i, float value)
	{
		if (i)
		{
			mValue[i] = std::max(value, mValue[0] + mSepMin);
		}
		else
		{
			mValue[i] = std::min(value, mValue[1] - mSepMin);
		}
	}

	const unsigned int& steps() const { return mSteps; }
	void setSteps(unsigned int steps) { mSteps = steps; }

	const nanogui::Color& highlightColor() const { return mHighlightColor; }
	void setHighlightColor(const nanogui::Color& highlightColor) { mHighlightColor = highlightColor; }

	std::function<void(float)> callback(int i) const { return mCallback[i]; }
	void setCallback(int i, const std::function<void(float)>& callback) { mCallback[i] = callback; }

	std::function<void(float)> finalCallback(int i) const { return mFinalCallback[i]; }
	void setFinalCallback(int i, const std::function<void(float)>& callback) { mFinalCallback[i] = callback; }

	virtual bool mouseDragEvent(const nanogui::Vector2i& p, const nanogui::Vector2i& rel, int button, int modifiers) override;
	virtual bool mouseButtonEvent(const nanogui::Vector2i& p, int button, bool down, int modifiers) override;
	virtual void draw(NVGcontext* ctx) override;
	virtual void save(nanogui::Serializer& s) const override;
	virtual bool load(nanogui::Serializer& s) override;

protected:
	float mValue[2] = { 0.0f, 1.f };
	float mSepMin = .001f;
	unsigned int mSteps = 0U;
	std::function<void(float)> mCallback[2];
	std::function<void(float)> mFinalCallback[2];
	nanogui::Color mHighlightColor;
	char mouseSelect = 0;
};
