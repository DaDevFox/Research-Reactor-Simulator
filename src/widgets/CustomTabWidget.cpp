#include "../../include/CustomTabWidget.h"

#include <nanogui/layout.h>
#include <nanogui/opengl.h>
#include <nanogui/screen.h>
#include <nanogui/stackedwidget.h>
#include <nanogui/tabheader.h>
#include <nanogui/theme.h>
#include <nanogui/window.h>

#include <algorithm>

#include "../../include/CustomTabHeader.h"
#include "../../include/CustomVScrollPanel.h"
#include "../../include/CustomWidget.h"
#include "../../include/RelativeGridLayout.h"

using namespace nanogui;

CustomTabWidget::CustomTabWidget(Widget* parent)
    : Widget(parent),
      mHeader(new CustomTabHeader(this)),
      mContent(new StackedWidget(this)) {
  mHeader->setCallback([this](int i) {
    mContent->setSelectedIndex(i);
    if (mCallback) mCallback(i);
  });
}

void CustomTabWidget::setActiveTab(int tabIndex) {
  mHeader->setActiveTab(tabIndex);
  mContent->setSelectedIndex(tabIndex);
}

int CustomTabWidget::activeTab() const {
  assert(mHeader->activeTab() == mContent->selectedIndex());
  return mContent->selectedIndex();
}

int CustomTabWidget::tabCount() const {
  assert(mContent->childCount() == mHeader->tabCount());
  return mHeader->tabCount();
}

CustomWidget* CustomTabWidget::createTab(int index, const std::string& label,
                                         bool scrolling) {
  if (scrolling) {
    CustomVScrollPanel* scrollMain = new CustomVScrollPanel(nullptr);
    addTab(index, label, scrollMain);
    RelativeGridLayout* rel2 = new RelativeGridLayout();
    rel2->appendCol(RelativeGridLayout::Size(1.f));
    rel2->appendRow(RelativeGridLayout::Size(1.f));
    scrollMain->setLayout(rel2);
    CustomWidget* mainTabBase = scrollMain->add<CustomWidget>();
    rel2->setAnchor(mainTabBase,
                    RelativeGridLayout::makeAnchor(
                        0, 0, 1, 1, Alignment::Fill, Alignment::Fill,
                        RelativeGridLayout::FillMode::Always,
                        RelativeGridLayout::FillMode::IfLess));
    return mainTabBase;
  } else {
    CustomWidget* mainTabBase = new CustomWidget(nullptr);
    addTab(index, label, mainTabBase);
    return mainTabBase;
  }
}

CustomWidget* CustomTabWidget::createTab(const std::string& label,
                                         bool scrolling) {
  return createTab(tabCount(), label, scrolling);
}

void CustomTabWidget::addTab(const std::string& name, Widget* tab) {
  addTab(tabCount(), name, tab);
}

void CustomTabWidget::addTab(int index, const std::string& label, Widget* tab) {
  assert(index <= tabCount());
  // It is important to add the content first since the callback
  // of the header will automatically fire when a new tab is added.
  mContent->addChild(index, tab);
  mHeader->addTab(index, label);
  assert(mHeader->tabCount() == mContent->childCount());
}

int CustomTabWidget::tabLabelIndex(const std::string& label) {
  return mHeader->tabIndex(label);
}

int CustomTabWidget::tabIndex(Widget* tab) { return mContent->childIndex(tab); }

void CustomTabWidget::ensureTabVisible(int index) {
  if (!mHeader->isTabVisible(index)) mHeader->ensureTabVisible(index);
}

const Widget* CustomTabWidget::tab(const std::string& tabName) const {
  int index = mHeader->tabIndex(tabName);
  if (index == mContent->childCount()) return nullptr;
  return mContent->children()[index];
}

Widget* CustomTabWidget::tab(const std::string& tabName) {
  int index = mHeader->tabIndex(tabName);
  if (index == mContent->childCount()) return nullptr;
  return mContent->children()[index];
}

bool CustomTabWidget::removeTab(const std::string& tabName) {
  int index = mHeader->removeTab(tabName);
  if (index == -1) return false;
  mContent->removeChild(index);
  return true;
}

void CustomTabWidget::removeTab(int index) {
  assert(mContent->childCount() < index);
  mHeader->removeTab(index);
  mContent->removeChild(index);
  if (activeTab() == index) setActiveTab(index == (index - 1) ? index - 1 : 0);
}

const std::string& CustomTabWidget::tabLabelAt(int index) const {
  return mHeader->tabLabelAt(index);
}

void CustomTabWidget::performLayout(NVGcontext* ctx) {
  int headerHeight = mHeader->preferredSize(ctx).y();
  int margin = mTheme->mTabInnerMargin;
  mHeader->setPosition({0, 0});
  mHeader->setSize({mSize.x(), headerHeight});
  mHeader->performLayout(ctx);
  mContent->setPosition({margin, headerHeight + margin});
  mContent->setSize(
      {mSize.x() - 2 * margin, mSize.y() - 2 * margin - headerHeight});
  mContent->performLayout(ctx);
}

Vector2i CustomTabWidget::preferredSize(NVGcontext* ctx) const {
  auto contentSize = mContent->preferredSize(ctx);
  auto headerSize = mHeader->preferredSize(ctx);
  int margin = mTheme->mTabInnerMargin;
  auto borderSize = Vector2i(2 * margin, 2 * margin);
  Vector2i tabPreferredSize =
      contentSize + borderSize + Vector2i(0, headerSize.y());
  return tabPreferredSize;
}

void CustomTabWidget::draw(NVGcontext* ctx) {
  // Draw children like the original Widget::draw (without scissor clipping)
  // The ext/nanogui Widget::draw applies scissor to each child which can cause
  // clipping issues
  if (!mChildren.empty()) {
    nvgTranslate(ctx, mPos.x(), mPos.y());
    for (Widget* child : mChildren)
      if (child->visible()) child->draw(ctx);
    nvgTranslate(ctx, -mPos.x(), -mPos.y());
  }

  int tabHeight = mHeader->preferredSize(ctx).y();
  std::pair<Vector2i, Vector2i> activeArea = mHeader->activeButtonArea();
  nvgSave(ctx);

  // FLAG: chnaged BorderWidth to TabBorderWidth
  float thisBorder = mTheme->mTabBorderWidth * 1.8f;

  nvgStrokeWidth(ctx, thisBorder);
  for (int i = 0; i < 2; i++) {
    float offset = i ? 0.f : thisBorder / 2.f;
    nvgStrokeColor(ctx, i ? mTheme->mBorderDark : mTheme->mBorderLight);
    // Up to the button
    nvgBeginPath(ctx);
    nvgMoveTo(ctx, mPos.x(), mPos.y() + tabHeight + offset);
    nvgLineTo(ctx, mPos.x() + activeArea.first.x(),
              mPos.y() + tabHeight + offset);
    nvgStroke(ctx);

    // From the button to the end
    nvgBeginPath(ctx);
    nvgMoveTo(ctx, mPos.x() + activeArea.second.x(),
              mPos.y() + tabHeight + offset);
    nvgLineTo(ctx, mPos.x() + mSize.x(), mPos.y() + tabHeight + offset);
    nvgStroke(ctx);
  }

  nvgRestore(ctx);
}
