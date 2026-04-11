#include "../../include/SliderCheckBox.h"

#include <nanogui/opengl.h>
#include <nanogui/serializer/core.h>

#include "util.h"

using namespace nanogui;

SliderCheckBox::SliderCheckBox(Widget *parent,
                               const std::function<void(bool)> &callback)
    : Widget(parent),
      mPushed(false),
      mChecked(false),
      state(0),
      mouseE(false),
      mCallback(callback) {
  mCursor = Cursor::Hand;
}

bool SliderCheckBox::mouseButtonEvent(const Vector2i &p, int button, bool down,
                                      int modifiers) {
  Widget::mouseButtonEvent(p, button, down, modifiers);
  if (!mEnabled) return false;

  if (button == GLFW_MOUSE_BUTTON_1) {
    if (down) {
      mPushed = true;
    } else if (mPushed) {
      if (contains(p)) {
        mChecked = !mChecked;
        if (state == 0) {
          state = mChecked ? 1 : -1;
          timeAtChange = get_seconds_since_epoch();
        } else {
          state = -state;
          timeAtChange =
              2 * get_seconds_since_epoch() - timeAtChange - ANIM_TIME;
        }
        if (mCallback) mCallback(mChecked);
      }
      mPushed = false;
    }
    return true;
  }
  return false;
}

bool SliderCheckBox::mouseEnterEvent(const Vector2i & /*p*/, bool enter) {
  mouseE = enter;
  return true;
}

Vector2i SliderCheckBox::preferredSize(NVGcontext * /* ctx */) const {
  if (mFixedSize != Vector2i::Zero()) return mFixedSize;
  return Vector2i((int)(fontSize() * 2.6f), (fontSize() * 1.3f));
}

void SliderCheckBox::draw(NVGcontext *ctx) {
  // Note: SliderCheckBox has no children, skip Widget::draw() to avoid scissor
  // clipping

  float cornerR = height() / 2.f;

  NVGpaint bg = nvgBoxGradient(
      ctx, mPos.x() + 1.5f, mPos.y() + 1.5f, mSize.x() - 3.0f, mSize.y() - 3.0f,
      cornerR, 3,
      mPushed ? Color(0, 100) : (mouseE ? Color(0, 16) : Color(0, 32)),
      Color(0, 0, 0, 180));

  nvgBeginPath(ctx);
  nvgRoundedRect(ctx, mPos.x() + 1.f, mPos.y() + 1.f, mSize.x() - 2.f,
                 mSize.y() - 2.f, cornerR);
  nvgFillPaint(ctx, bg);
  nvgFill(ctx);

  float amp0 = width() - 2.f * cornerR;
  if (state != 0) {
    double t = (get_seconds_since_epoch() - timeAtChange) / ANIM_TIME;
    if (t >= 1.) {
      t = 1.;
      state = 0;
    }
    float x = (1.f - pow(exp(10 * (t - 0.5)) + 1, -1)) * 1.01 - 0.005;
    Color sliderCol;
    if (x < 0.5) {
      sliderCol =
          Color(mChecked ? 1.f : (x * 2), mChecked ? (x * 2) : 1.f, 0.f, 1.f);
    } else {
      sliderCol = Color(mChecked ? (1.f - x) * 2 : 1.f,
                        mChecked ? 1.f : (1.f - x) * 2, 0.f, 1.f);
    }
    float loc[2] = {mPos.x() + cornerR + amp0 * (mChecked ? x : (1 - x)),
                    mPos.y() + height() / 2.f};
    NVGpaint shineOn = nvgRadialGradient(
        ctx, loc[0], loc[1], 4.f, 6.5f, sliderCol,
        Color(sliderCol.r(), sliderCol.g(), sliderCol.b(), 0.f));
    // shine
    nvgFillPaint(ctx, shineOn);
    nvgBeginPath(ctx);
    nvgEllipse(ctx, loc[0], loc[1], 6.5f, 6.5f);
    nvgFill(ctx);
  } else {
    float loc[2] = {mPos.x() + (mChecked ? (width() - cornerR) : cornerR),
                    mPos.y() + height() / 2.f};
    Color sliderCol = mChecked ? Color(0, 255, 0, 255) : Color(255, 0, 0, 255);
    NVGpaint shineOn = nvgRadialGradient(
        ctx, loc[0], loc[1], 4.f, 6.5f, sliderCol,
        Color(sliderCol.r(), sliderCol.g(), sliderCol.b(), 0.f));
    // shine
    nvgFillPaint(ctx, shineOn);
    nvgBeginPath(ctx);
    nvgEllipse(ctx, loc[0], loc[1], 6.5f, 6.5f);
    nvgFill(ctx);
  }
}

void SliderCheckBox::save(Serializer &s) const {
  Widget::save(s);
  s.set("pushed", mPushed);
  s.set("checked", mChecked);
}

bool SliderCheckBox::load(Serializer &s) {
  if (!Widget::load(s)) return false;
  if (!s.get("pushed", mPushed)) return false;
  if (!s.get("checked", mChecked)) return false;
  return true;
}
