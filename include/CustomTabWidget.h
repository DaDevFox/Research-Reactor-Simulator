#include <nanogui/widget.h>
#include <nanogui/stackedwidget.h>
#include "CustomTabHeader.h"

class CustomTabWidget : public nanogui::Widget
{
public:
	CustomTabWidget(Widget* parent);

	void setActiveTab(int tabIndex);
	int activeTab() const;
	int tabCount() const;

	/**
	 * Sets the callable objects which is invoked when a tab is changed.
	 * The argument provided to the callback is the index of the new active tab.
	*/
	void setCallback(const std::function<void(int)>& callback) { mCallback = callback; };
	const std::function<void(int)>& callback() const { return mCallback; }

	/// Creates a new tab with the specified name and returns a pointer to the layer.
	CustomWidget* createTab(const std::string& label, bool scrolling = true);
	CustomWidget* createTab(int index, const std::string& label, bool scrolling = true);

	/// Inserts a tab at the end of the tabs collection and associates it with the provided widget.
	void addTab(const std::string& label, Widget* tab);

	/// Inserts a tab into the tabs collection at the specified index and associates it with the provided widget.
	void addTab(int index, const std::string& label, Widget* tab);

	/**
	 * Removes the tab with the specified label and returns the index of the label.
	 * Returns whether the removal was successful.
	 */
	bool removeTab(const std::string& label);

	/// Removes the tab with the specified index.
	void removeTab(int index);

	/// Retrieves the label of the tab at a specific index.
	const std::string& tabLabelAt(int index) const;

	/**
	 * Retrieves the index of a specific tab using its tab label.
	 * Returns -1 if there is no such tab.
	 */
	int tabLabelIndex(const std::string& label);

	/**
	 * Retrieves the index of a specific tab using a widget pointer.
	 * Returns -1 if there is no such tab.
	 */
	int tabIndex(Widget* tab);

	/**
	 * This function can be invoked to ensure that the tab with the provided
	 * index the is visible, i.e to track the given tab. Forwards to the tab
	 * header widget. This function should be used whenever the client wishes
	 * to make the tab header follow a newly added tab, as the content of the
	 * new tab is made visible but the tab header does not track it by default.
	 */
	void ensureTabVisible(int index);

	CustomTabHeader* header() const { return mHeader; }

	const Widget* tab(const std::string& label) const;
	Widget* tab(const std::string& label);

	virtual void performLayout(NVGcontext* ctx) override;
	virtual nanogui::Vector2i preferredSize(NVGcontext* ctx) const override;
	virtual void draw(NVGcontext* ctx) override;

private:
	CustomTabHeader* mHeader;
	nanogui::StackedWidget* mContent;
	std::function<void(int)> mCallback;
};

