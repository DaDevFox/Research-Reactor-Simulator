#include "CustomWidget.h"
/*
        nanogui/window.h -- Top-level window widget

        NanoGUI was developed by Wenzel Jakob <wenzel.jakob@epfl.ch>.
        The widget drawing code is based on the NanoVG demo application
        by Mikko Mononen.

        All rights reserved. Use of this source code is governed by a
        BSD-style license that can be found in the LICENSE.txt file.
*/

#pragma once

#include <nanogui/widget.h>
#include <nanogui/window.h>

class CustomWindow : public nanogui::Window {
  friend class nanogui::Popup;

 public:
  CustomWindow(Widget* parent, const std::string& title = "Untitled");

  /// Return the panel used to house window buttons
  CustomWidget* buttonPanel();

  /// Draw the window
  virtual void draw(NVGcontext* ctx) override;

  /// Handle window drag events
  virtual bool mouseDragEvent(const nanogui::Vector2i& p,
                              const nanogui::Vector2i& rel, int button,
                              int modifiers) override;
  /// Handle mouse events recursively and bring the current window to the top
  virtual bool mouseButtonEvent(const nanogui::Vector2i& p, int button,
                                bool down, int modifiers) override;
  /// Accept scroll events and propagate them to the widget under the mouse
  /// cursor
  virtual bool scrollEvent(const nanogui::Vector2i& p,
                           const nanogui::Vector2f& rel) override;

  virtual bool mouseMotionEvent(const nanogui::Vector2i& p,
                                const nanogui::Vector2i& rel, int button,
                                int modifiers) override;

  virtual bool mouseEnterEvent(const nanogui::Vector2i& p, bool enter) override;

  void setBorder(bool value);
  bool border();
  void setBorderColor(nanogui::Color value);
  nanogui::Color borderColor();
  void setBorderThickness(float value);
  float borderThickness();

  CustomWidget* mButtonPanel;
  bool mBorder = false;
  nanogui::Color mBorderColor = nanogui::Color(1.f, 1.f);
  float mBorderWidth = 2.f;
  float mBorderThickness = 1.5f;

  // CustomWidget properties:
  nanogui::Color mBackgroundColor = nanogui::Color(0, 0);
  bool drawBackground = false;

  nanogui::ref<CustomTheme> mCustomTheme;

  // CustomWidget methods:
  CustomTheme* theme();
  const CustomTheme* theme() const;
  virtual void setTheme(CustomTheme* theme);

  /// Returns the background color of this widget
  const nanogui::Color& backgroundColor() const { return mBackgroundColor; }
  void setBackgroundColor(const nanogui::Color& backgroundColor) {
    mBackgroundColor = backgroundColor;
  }

  const bool& getDrawBackground() const { return drawBackground; }
  void setDrawBackground(bool value) { drawBackground = value; }

  nanogui::Screen* parentScreen();

  /// Return current font size. If not set the default of the current theme will
  /// be returned
  float fontSize() const;

  /// Set the font size of this widget
  void setFontSize(int fontSize) { nanogui::Widget::setFontSize(fontSize); }
  /// Return whether the font size is explicitly specified for this widget
  bool hasFontSize() const { return mFontSize > 0; }

  /// Determine the widget located at the given position value (recursive)
  virtual Widget* findWidget(const nanogui::Vector2i& p);
};
