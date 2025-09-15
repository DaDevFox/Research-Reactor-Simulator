#include "../include/CustomTheme.h"

CustomTheme::CustomTheme(NVGcontext* ctx) : Theme(ctx), mBorderWidth(1.f)
{
	// TODO: decide on default values (changed ones require floating point for currently int fields such as mStandardFontSize)
}

CustomTheme::CustomTheme(const Theme& original) : Theme(original),
mBorderWidth(1.f)
{}
