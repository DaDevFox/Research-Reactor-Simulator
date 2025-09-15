
#include "../include/CustomMessageDialog.h"
#include <nanogui/layout.h>
#include <nanogui/button.h>
#include <nanogui/entypo.h>
#include <nanogui/label.h>
#include "../include/RelativeGridLayout.h"

using namespace nanogui;

CustomMessageDialog::CustomMessageDialog(Widget* parent, Type type, const std::string& title,
	const std::string& message,
	const std::string& buttonText,
	const std::string& altButtonText, bool altButton) : Window(parent, title)
{
	setLayout(new BoxLayout(Orientation::Vertical,
		Alignment::Fill, 10, 0));
	setModal(true);

	Widget* panel1 = new Widget(this);
	panel1->setLayout(new BoxLayout(Orientation::Horizontal,
		Alignment::Middle, 10, 10));
	int icon = 0;
	switch (type)
	{
	case Type::Information: icon = ENTYPO_ICON_CIRCLED_INFO; break;
	case Type::Question: icon = ENTYPO_ICON_CIRCLED_HELP; break;
	case Type::Warning: icon = ENTYPO_ICON_WARNING; break;
	}
	CustomLabel* iconLabel = new CustomLabel(panel1, std::string(utf8(icon).data()), "icons");
	iconLabel->setFontSize(50);
	mMessageLabel = new CustomLabel(panel1, message);
	mMessageLabel->setTextAlignment(CustomLabel::TextAlign::LEFT || CustomLabel::TextAlign::VERTICAL_CENTER);
	Widget* panel2 = new Widget(this);
	RelativeGridLayout* relLayout = new RelativeGridLayout();
	relLayout->appendRow(1.f);
	relLayout->appendCol(1.f);
	panel2->setLayout(relLayout);

	if (altButton)
	{
		relLayout->appendCol(1.f);
		Button* button = new Button(panel2, altButtonText, ENTYPO_ICON_CIRCLED_CROSS);
		relLayout->setAnchor(button, RelativeGridLayout::makeAnchor(1, 0, 1, 1, Alignment::Middle, Alignment::Middle));
		button->setCallback([&] { if (mCallback) mCallback(1); dispose(); });
	}
	Button* button = new Button(panel2, buttonText, ENTYPO_ICON_CHECK);
	relLayout->setAnchor(button, RelativeGridLayout::makeAnchor(0, 0, 1, 1, Alignment::Middle, Alignment::Middle));
	button->setCallback([&] { if (mCallback) mCallback(0); dispose(); });

	center();
	requestFocus();
}
