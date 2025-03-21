#include <nanogui/widget.h>

class CustomImageView : public nanogui::Widget
{
public:
	enum class SizePolicy
	{
		Fixed,
		Expand
	};

	CustomImageView(Widget* parent, int image = 0, SizePolicy policy = SizePolicy::Fixed);

	void setImage(int img) { mImage = img; }
	int  image() const { return mImage; }

	void       setPolicy(SizePolicy policy) { mPolicy = policy; }
	SizePolicy policy() const { return mPolicy; }

	virtual nanogui::Vector2i preferredSize(NVGcontext* ctx) const override;
	virtual void draw(NVGcontext* ctx) override;
protected:
	int mImage;
	SizePolicy mPolicy;
};
