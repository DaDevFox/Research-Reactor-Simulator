
#include "../../include/CustomPopup.h"

#include <nanogui/opengl.h>
#include <nanogui/serializer/core.h>
#include <nanogui/theme.h>

using namespace nanogui;

CustomPopup::CustomPopup(Widget *parent, Window *parentWindow)
    : Window(parent, ""),
      mParentWindow(parentWindow),
      mAnchorPos(Vector2i::Zero()),
      mAnchorHeight(30) {}

void CustomPopup::performLayout(NVGcontext *ctx) {
  if (mLayout || (mChildren.size() != 1)) {
    Widget::performLayout(ctx);
  } else {
    Vector2i ps = mChildren[0]->preferredSize(ctx);
    setWidth(ps.x());
    setHeight(std::min(mParentWindow->position().y() + mParentWindow->height() -
                           absolutePosition().y(),
                       ps.y()));
    mChildren[0]->setPosition(Vector2i::Zero());
    mChildren[0]->setSize(mSize);
    mChildren[0]->performLayout(ctx);
  }
}

void CustomPopup::refreshRelativePlacement() {
  // Note: Can't call mParentWindow->refreshRelativePlacement() as it's
  // protected This is handled by nanogui internally when mParentWindow draws
  mVisible &= mParentWindow->visibleRecursive();
  mPos = mParentWindow->position() + mAnchorPos - Vector2i(0, mAnchorHeight);
}

void CustomPopup::draw(NVGcontext *ctx) {
  refreshRelativePlacement();

  if (!mVisible) return;

  int ds = mTheme->mWindowDropShadowSize, cr = mTheme->mWindowCornerRadius;

  /* Draw a drop shadow */
  NVGpaint shadowPaint =
      nvgBoxGradient(ctx, mPos.x(), mPos.y(), mSize.x(), mSize.y(), cr * 2,
                     ds * 2, mTheme->mDropShadow, mTheme->mTransparent);

  nvgBeginPath(ctx);
  nvgRect(ctx, mPos.x() - ds, mPos.y() - ds, mSize.x() + 2 * ds,
          mSize.y() + 2 * ds);
  nvgRoundedRect(ctx, mPos.x(), mPos.y(), mSize.x(), mSize.y(), cr);
  nvgPathWinding(ctx, NVG_HOLE);
  nvgFillPaint(ctx, shadowPaint);
  nvgFill(ctx);

  /* Draw window */
  nvgBeginPath(ctx);
  nvgRoundedRect(ctx, mPos.x(), mPos.y(), mSize.x(), mSize.y(), cr);

  nvgMoveTo(ctx, mPos.x() - 15, mPos.y() + mAnchorHeight);
  nvgLineTo(ctx, mPos.x() + 1, mPos.y() + mAnchorHeight - 15);
  nvgLineTo(ctx, mPos.x() + 1, mPos.y() + mAnchorHeight + 15);

  nvgFillColor(ctx, mTheme->mWindowPopup);
  nvgFill(ctx);

  // Call Widget::draw to properly render children (matching original nanogui
  // behavior)
  Widget::draw(ctx);
}

void CustomPopup::save(Serializer &s) const {
  Window::save(s);
  s.set("anchorPos", mAnchorPos);
  s.set("anchorHeight", mAnchorHeight);
}

bool CustomPopup::load(Serializer &s) {
  if (!Window::load(s)) return false;
  if (!s.get("anchorPos", mAnchorPos)) return false;
  if (!s.get("anchorHeight", mAnchorHeight)) return false;
  return true;
}
