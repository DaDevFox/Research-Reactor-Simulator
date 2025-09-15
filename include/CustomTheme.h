#include <nanogui/theme.h>

class CustomTheme : public nanogui::Theme
{
public:
	float mBorderWidth;

	CustomTheme(NVGcontext* ctx);
	CustomTheme(const Theme& original);
};