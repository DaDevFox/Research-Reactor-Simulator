#pragma once

#include <nanogui/common.h>
#include <nanogui/layout.h>
#include <nanogui/object.h>
#include <nanogui/opengl.h>
#include <nanogui/serializer/core.h>
#include <nanogui/theme.h>
#include <nanogui/widget.h>
#include <nanogui/window.h>

#include <iostream>
#include <vector>

#include "CustomTheme.h"

enum class Cursor;

enum Border { NONE = 0, LEFT = 1, TOP = 2, RIGHT = 4, BOTTOM = 8 };

/**
 * \brief Base class of all extended nanogui widgets (have borders + fill
 * control settings)
 *
 * \ref Widget is the base class of all widgets in \c nanogui. It can
 * also be used as an panel to arrange an arbitrary number of child
 * widgets using a layout generator (see \ref Layout).
 */
class CustomWidget : public nanogui::Widget {
 public:
  /// Construct a new widget with the given parent widget
  CustomWidget(Widget *parent);

  /// Returns the background color of this widget
  const nanogui::Color &backgroundColor() const { return mBackgroundColor; }
  void setBackgroundColor(const nanogui::Color &backgroundColor) {
    mBackgroundColor = backgroundColor;
  }

  const bool &getDrawBackground() const { return drawBackground; }
  void setDrawBackground(bool value) { drawBackground = value; }

  nanogui::Screen *parentScreen();

  /// Return current font size. If not set the default of the current theme will
  /// be returned
  float fontSize() const;

  /// Set the font size of this widget
  void setFontSize(int fontSize) { mFontSize = fontSize; }
  /// Return whether the font size is explicitly specified for this widget
  bool hasFontSize() const { return mFontSize > 0; }

  /// Determine the widget located at the given position value (recursive)
  virtual Widget *findWidget(const nanogui::Vector2i &p);

  CustomTheme *theme();
  const CustomTheme *theme() const;
  virtual void setTheme(CustomTheme *theme);

  void draw(NVGcontext *ctx) override;

  int border() { return mBorder; }
  void setBorder(int border) { mBorder = border; }

  nanogui::Color borderColor() { return mBorderColor; }
  void setBorderColor(nanogui::Color borderColor) {
    mBorderColor = borderColor;
  }

  float borderWidth() { return mBorderWidth; }
  void setBorderWidth(float borderWidth) { mBorderWidth = borderWidth; }

 protected:
  nanogui::ref<CustomTheme> mCustomTheme;
  /// Free all resources used by the widget and any children
  virtual ~CustomWidget();

  nanogui::Color mBackgroundColor = nanogui::Color(0, 0);
  bool drawBackground = false;

  nanogui::Color mBorderColor =
      nanogui::Color(0, 0);  // Initialize to transparent to avoid garbage blue
  int mBorder = 0;
  float mBorderWidth = 2.f;
};
