#pragma once

#include <nanogui/widget.h>
#include <nanovg.h>

#include "CustomWidget.h"

class CustomTabHeader : public CustomWidget {
 public:
  CustomTabHeader(Widget* parent, const std::string& font = "sans-bold");

  void setFont(const std::string& font) { mFont = font; }
  const std::string& font() const { return mFont; }
  bool overflowing() const { return mOverflowing; }

  void setStretch(bool stretch) { mStretch = stretch; }
  bool stretch() const { return mStretch; }
  void setButtonAlignment(int align) { mButtonAlignment = align; }
  int buttonAlignment() const { return mButtonAlignment; }

  /**
   * Sets the callable objects which is invoked when a tab button is pressed.
   *  The argument provided to the callback is the index of the tab.
   */
  void setCallback(const std::function<void(int)>& callback) {
    mCallback = callback;
  };
  const std::function<void(int)>& callback() const { return mCallback; }

  void setActiveTab(int tabIndex);
  int activeTab() const;
  bool isTabVisible(int index) const;
  int tabCount() const { return (int)mTabButtons.size(); }

  /// Inserts a tab at the end of the tabs collection.
  void addTab(const std::string& label);

  /// Inserts a tab into the tabs collection at the specified index.
  void addTab(int index, const std::string& label);

  /**
   * Removes the tab with the specified label and returns the index of the
   * label. Returns -1 if there was no such tab
   */
  int removeTab(const std::string& label);

  /// Removes the tab with the specified index.
  void removeTab(int index);

  /// Retrieves the label of the tab at a specific index.
  const std::string& tabLabelAt(int index) const;

  /**
   * Retrieves the index of a specific tab label.
   * Returns the number of tabs (tabsCount) if there is no such tab.
   */
  int tabIndex(const std::string& label);

  /**
   * Recalculate the visible range of tabs so that the tab with the specified
   * index is visible. The tab with the specified index will either be the
   * first or last visible one depending on the position relative to the
   * old visible range.
   */
  void ensureTabVisible(int index);

  /**
   * Returns a pair of nanogui::Vectors describing the top left (pair.first) and
   * the bottom right (pair.second) positions of the rectangle containing the
   * visible tab buttons.
   */
  std::pair<nanogui::Vector2i, nanogui::Vector2i> visibleButtonArea() const;

  /**
   * Returns a pair of nanogui::Vectors describing the top left (pair.first) and
   * the bottom right (pair.second) positions of the rectangle containing the
   * active tab button. Returns two zero vectors if the active button is not
   * visible.
   */
  std::pair<nanogui::Vector2i, nanogui::Vector2i> activeButtonArea() const;

  virtual void performLayout(NVGcontext* ctx) override;
  virtual nanogui::Vector2i preferredSize(NVGcontext* ctx) const override;
  virtual bool mouseButtonEvent(const nanogui::Vector2i& p, int button,
                                bool down, int modifiers) override;

  virtual void draw(NVGcontext* ctx) override;

  nanogui::Color activeTextColor() { return activeTxtColor; }
  void setActiveTextColor(nanogui::Color col) { activeTxtColor = col; }

 private:
  nanogui::Color activeTxtColor = nanogui::Color(77, 184, 255, 255);
  /// Implementation class of the actual tab buttons.
  class TabButton {
   public:
    constexpr static const char* dots = "...";

    TabButton(CustomTabHeader& header, const std::string& label);

    void setLabel(const std::string& label) { mLabel = label; }
    const std::string& label() const { return mLabel; }
    void setSize(const nanogui::Vector2i& size) { mSize = size; }
    const nanogui::Vector2i& size() const { return mSize; }

    nanogui::Vector2i preferredSize(NVGcontext* ctx) const;
    void calculateVisibleString(NVGcontext* ctx);
    void drawAtPosition(NVGcontext* ctx, const nanogui::Vector2i& position,
                        bool active);
    void drawActiveBorderAt(NVGcontext* ctx, const nanogui::Vector2i& position,
                            float offset, const nanogui::Color& color);
    void drawInactiveBorderAt(NVGcontext* ctx,
                              const nanogui::Vector2i& position, float offset,
                              const nanogui::Color& color);

   private:
    CustomTabHeader* mHeader;
    std::string mLabel;
    nanogui::Vector2i mSize;
    struct StringView {
      const char* first = nullptr;
      const char* last = nullptr;
    };
    StringView mVisibleText;
    int mVisibleWidth = 0;
  };

  using TabIterator = std::vector<TabButton>::iterator;
  using ConstTabIterator = std::vector<TabButton>::const_iterator;

  enum class ClickLocation { LeftControls, RightControls, TabButtons };

  TabIterator visibleBegin() {
    return std::next(mTabButtons.begin(), mVisibleStart);
  }
  TabIterator visibleEnd() {
    return std::next(mTabButtons.begin(), mVisibleEnd);
  }
  TabIterator activeIterator() {
    return std::next(mTabButtons.begin(), mActiveTab);
  }
  TabIterator tabIterator(int index) {
    return std::next(mTabButtons.begin(), index);
  }

  ConstTabIterator visibleBegin() const {
    return std::next(mTabButtons.begin(), mVisibleStart);
  }
  ConstTabIterator visibleEnd() const {
    return std::next(mTabButtons.begin(), mVisibleEnd);
  }
  ConstTabIterator activeIterator() const {
    return std::next(mTabButtons.begin(), mActiveTab);
  }
  ConstTabIterator tabIterator(int index) const {
    return std::next(mTabButtons.begin(), index);
  }

  /// Given the beginning of the visible tabs, calculate the end.
  void calculateVisibleEnd();

  void drawControls(NVGcontext* ctx);
  ClickLocation locateClick(const nanogui::Vector2i& p);
  void onArrowLeft();
  void onArrowRight();

  std::function<void(int)> mCallback;
  std::vector<TabButton> mTabButtons;
  int mVisibleStart = 0;
  int mVisibleEnd = 0;
  int mActiveTab = 0;
  bool mOverflowing = false;

  std::string mFont;

  bool mStretch = false;
  int mButtonAlignment = NVGalign::NVG_ALIGN_LEFT | NVGalign::NVG_ALIGN_TOP;
};
