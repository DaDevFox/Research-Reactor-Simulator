#pragma once

#include <nanogui/window.h>
#include "CustomLabel.h"

/**
 * \class MessageDialog messagedialog.h nanogui/messagedialog.h
 *
 * \brief Simple "OK" or "Yes/No"-style modal dialogs.
 */
class CustomMessageDialog : public nanogui::Window
{
public:
	/// Classification of the type of message this MessageDialog represents.
	enum class Type
	{
		Information,
		Question,
		Warning
	};

	CustomMessageDialog(Widget* parent, Type type, const std::string& title = "Untitled",
		const std::string& message = "Message",
		const std::string& buttonText = "OK",
		const std::string& altButtonText = "Cancel", bool altButton = false);

	CustomLabel* messageLabel() { return mMessageLabel; }
	const CustomLabel* messageLabel() const { return mMessageLabel; }

	std::function<void(int)> callback() const { return mCallback; }
	void setCallback(const std::function<void(int)>& callback) { mCallback = callback; }
protected:
	std::function<void(int)> mCallback;
	CustomLabel* mMessageLabel;
public:
	EIGEN_MAKE_ALIGNED_OPERATOR_NEW
};
