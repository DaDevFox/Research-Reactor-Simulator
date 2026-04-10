#include "../include/IntervalSlider.h"

#include <nanogui/serializer/core.h>
#include <nanovg.h>
using namespace nanogui;

IntervalSlider::IntervalSlider(Widget* parent) : Widget(parent) {
  mHighlightColor = Color(255, 80, 80, 70);
}

bool IntervalSlider::mouseDragEvent(const Vector2i& p,
                                    const Vector2i& /* rel */, int /* button */,
                                    int /* modifiers */) {
  if (!(mEnabled && mouseSelect)) return false;
  float relPos = (p.x() - mPos.x()) / (float)mSize.x();
  if (mSteps) relPos = std::round(relPos * mSteps) / mSteps;
  if (mouseSelect == 1) {
    mValue[mouseSelect - 1] =
        std::min(std::max(relPos, 0.0f),
                 mSteps ? (std::floor((mValue[1] - mSepMin) * mSteps) / mSteps)
                        : (mValue[1] - mSepMin));
  } else {
    mValue[mouseSelect - 1] = std::min(
        std::max(relPos,
                 mSteps ? (std::ceil((mValue[0] + mSepMin) * mSteps) / mSteps)
                        : (mValue[0] + mSepMin)),
        1.f);
  }
  if (mCallback[mouseSelect - 1])
    mCallback[mouseSelect - 1](mValue[mouseSelect - 1]);
  return true;
}

bool IntervalSlider::mouseButtonEvent(const Vector2i& p, int /* button */,
                                      bool down, int /* modifiers */) {
  if (!mEnabled) return false;
  if (!mouseSelect && down) {
    float diff[2];
    for (int i = 0; i < 2; i++) {
      diff[i] = std::abs(p.x() - (mValue[i] * mSize.x() + mPos.x()));
    }
    mouseSelect = (diff[0] > diff[1]) ? 2 : 1;
  } else if (!down)
    mouseSelect = 0;
  if (!mouseSelect) return false;
  float relPos = (p.x() - mPos.x()) / (float)mSize.x();
  if (mSteps) relPos = std::round(relPos * mSteps) / mSteps;
  if (mouseSelect == 1) {
    mValue[mouseSelect - 1] =
        std::min(std::max(relPos, 0.0f),
                 mSteps ? (std::floor((mValue[1] - mSepMin) * mSteps) / mSteps)
                        : (mValue[1] - mSepMin));
  } else {
    mValue[mouseSelect - 1] = std::min(
        std::max(relPos,
                 mSteps ? (std::ceil((mValue[0] + mSepMin) * mSteps) / mSteps)
                        : (mValue[0] + mSepMin)),
        1.f);
  }
  if (mCallback[mouseSelect - 1])
    mCallback[mouseSelect - 1](mValue[mouseSelect - 1]);
  if (mFinalCallback[mouseSelect - 1] && !down)
    mFinalCallback[mouseSelect - 1](mValue[mouseSelect - 1]);
  return true;
}

void IntervalSlider::draw(NVGcontext* ctx) {
  nvgTranslate(ctx, (float)mPos.x(), (float)mPos.y());
  Vector2f center = mSize.cast<float>() * 0.5f;
  Vector2f knobPos[2] = {Vector2f(mValue[0] * mSize.x(), center.y()),
                         Vector2f(mValue[1] * mSize.x(), center.y())};
  float kr = (int)(mSize.y() * 0.5f);
  NVGpaint bg = nvgBoxGradient(ctx, 0.f, center.y() - 3, mSize.x(), 6, 1, 3,
                               Color(0, mEnabled ? 32 : 10),
                               Color(0, mEnabled ? 128 : 210));

  // Draw track
  nvgBeginPath(ctx);
  nvgRoundedRect(ctx, 0.f, center.y() - 4.f, mSize.x(), 8.f, 2.f);
  nvgFillPaint(ctx, bg);
  nvgFill(ctx);

  // Draw highlight
  nvgBeginPath(ctx);
  nvgRoundedRect(ctx, mValue[0] * mSize.x(), center.y() - 2.5f,
                 mSize.x() * (mValue[1] - mValue[0]), 5.f, 2.f);
  nvgFillColor(ctx, mHighlightColor);
  nvgFill(ctx);

  // Draw steps if desired
  if (mSteps) {
    float drawStep =
        (float)(std::pow(10., std::ceil(std::log10(mSteps)) - 1) / mSteps);
    const float fromTo[2] = {2.f, mSize.y() - 2.f};
    nvgBeginPath(ctx);
    for (float drawX = 0.f; drawX < mSize.x(); drawX += drawStep) {
      nvgMoveTo(ctx, drawX, fromTo[0]);
      nvgLineTo(ctx, drawX, fromTo[1]);
    }
    nvgStrokeColor(ctx, Color(255, 50));
    nvgStrokeWidth(ctx, 1.5f);
    nvgStroke(ctx);
  }

  NVGpaint knobShadow[2], knob, knobReverse;
  knob = nvgLinearGradient(ctx, 0.f, center.y() - kr, 0.f, center.y() + kr,
                           mTheme->mBorderLight, mTheme->mBorderMedium);
  knobReverse =
      nvgLinearGradient(ctx, 0.f, center.y() - kr, 0.f, center.y() + kr,
                        mTheme->mBorderMedium, mTheme->mBorderLight);
  for (int i = 0; i < 2; i++) {
    // Draw knob shadows
    knobShadow[i] =
        nvgRadialGradient(ctx, knobPos[i].x(), knobPos[i].y(), kr - 3, kr + 3,
                          Color(0, 64), mTheme->mTransparent);
    nvgBeginPath(ctx);
    nvgRect(ctx, knobPos[i].x() - kr - 5, knobPos[i].y() - kr - 5, kr * 2 + 10,
            kr * 2 + 10 + 3);
    nvgCircle(ctx, knobPos[i].x(), knobPos[i].y(), kr);
    nvgPathWinding(ctx, NVG_HOLE);
    nvgFillPaint(ctx, knobShadow[i]);
    nvgFill(ctx);

    // Draw knobs
    nvgBeginPath(ctx);
    nvgCircle(ctx, knobPos[i].x(), knobPos[i].y(), kr);
    nvgStrokeColor(ctx, mTheme->mBorderDark);
    nvgFillPaint(ctx, knob);
    nvgStroke(ctx);
    nvgFill(ctx);
    nvgBeginPath(ctx);
    nvgCircle(ctx, knobPos[i].x(), knobPos[i].y(), kr / 2);
    nvgFillColor(ctx, Color(150, mEnabled ? 255 : 100));
    nvgStrokePaint(ctx, knobReverse);
    nvgStroke(ctx);
    nvgFill(ctx);
  }
  nvgTranslate(ctx, (float)-mPos.x(), (float)-mPos.y());
}

void IntervalSlider::save(Serializer& s) const {
  Widget::save(s);
  s.set("value1", mValue[0]);
  s.set("value2", mValue[1]);
  s.set("highlightColor", mHighlightColor);
}

bool IntervalSlider::load(Serializer& s) {
  if (!Widget::load(s)) return false;
  if (!s.get("value1", mValue[0])) return false;
  if (!s.get("value2", mValue[1])) return false;
  if (!s.get("highlightColor", mHighlightColor)) return false;
  return true;
}
