#pragma once

#include <nanogui/widget.h>

#define ANIM_TIME 0.4

class SliderCheckBox : public nanogui::Widget {
 public:
  SliderCheckBox(Widget* parent, const std::function<void(bool)>& callback =
                                     std::function<void(bool)>());

  const bool& checked() const { return mChecked; }
  void setChecked(const bool& checked) { mChecked = checked; }

  const bool& pushed() const { return mPushed; }
  void setPushed(const bool& pushed) { mPushed = pushed; }

  std::function<void(bool)> callback() const { return mCallback; }
  void setCallback(const std::function<void(bool)>& callback) {
    mCallback = callback;
  }

  virtual bool mouseButtonEvent(const nanogui::Vector2i& p, int button,
                                bool down, int modifiers) override;
  virtual bool mouseEnterEvent(const nanogui::Vector2i& p, bool enter) override;
  virtual nanogui::Vector2i preferredSize(NVGcontext* ctx) const override;
  virtual void draw(NVGcontext* ctx) override;

  virtual void save(nanogui::Serializer& s) const override;
  virtual bool load(nanogui::Serializer& s) override;

 protected:
  std::string mCaption;
  bool mPushed, mChecked;
  std::function<void(bool)> mCallback;
  double timeAtChange;
  int state;
  bool mouseE;
};
