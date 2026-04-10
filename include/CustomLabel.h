#pragma once
#include "CustomWidget.h"

/**
 * \brief Text label widget
 *
 * The font and color can be customized. When \ref Widget::setFixedWidth()
 * is used, the text is wrapped when it surpasses the specified width
 */
class CustomLabel : public CustomWidget {
 public:
  enum TextAlign {
    // Horizontal align
    LEFT = 1 << 0,               // Default, align text horizontally to left.
    HORIZONTAL_CENTER = 1 << 1,  // Align text horizontally to center.
    RIGHT = 1 << 2,              // Align text horizontally to right.
                     // Vertical align
    TOP = 1 << 3,              // Align text vertically to top.
    VERTICAL_CENTER = 1 << 4,  // Align text vertically to middle.
    BOTTOM = 1 << 5,           // Align text vertically to bottom.
    BASELINE = 1 << 6,         // Default, align text vertically to baseline.
  };

  CustomLabel(Widget* parent, const std::string& caption,
              const std::string& font = "sans", int fontSize = -1);

  /// Get the label's text caption
  const std::string& caption() const { return mCaption; }
  /// Set the label's text caption
  void setCaption(const std::string& caption) { mCaption = caption; }

  /// Set the currently active font (2 are available by default: 'sans' and
  /// 'sans-bold')
  void setFont(const std::string& font) { mFont = font; }
  /// Get the currently active font
  const std::string& font() const { return mFont; }

  /// Get the label color
  nanogui::Color color() const { return mColor; }
  /// Set the label color
  void setColor(const nanogui::Color& color) { mColor = color; }

  nanogui::Color glowColor() const { return mGlowColor; }
  void setGlowColor(const nanogui::Color& glowColor) { mGlowColor = glowColor; }

  bool glowEnabled() const { return mGlow; }
  void setGlow(const bool& enabled) { mGlow = enabled; }

  float glowAmount() const { return mFeather; }
  void setGlowAmount(const float& value) { mFeather = value; }

  float edgeRounding() const { return mRoundedEdges; }
  void setEdgeRounding(const float& value) { mRoundedEdges = value; }

  int textAlignment() const { return mTextAlignment; }
  void setTextAlignment(const int value) { mTextAlignment = value; }

  void setPadding(size_t i, float padding) { mPadding[i] = padding; }

  /// Set the \ref Theme used to draw this widget
  virtual void setTheme(nanogui::Theme* theme) override;

  /// Compute the size needed to fully display the label
  virtual nanogui::Vector2i preferredSize(NVGcontext* ctx) const override;

  /// Draw the label
  virtual void draw(NVGcontext* ctx) override;

  virtual void save(nanogui::Serializer& s) const override;
  virtual bool load(nanogui::Serializer& s) override;

 protected:
  std::string mCaption;
  std::string mFont;
  nanogui::Color mColor;
  bool mGlow = false;
  nanogui::Color mGlowColor = nanogui::Color(255, 0, 0, 255);
  float mFeather = 7.f;
  float mRoundedEdges = 2.f;
  float mPadding[4] = {0.f, 0.f, 0.f, 0.f};
  int mTextAlignment = TextAlign::BASELINE | TextAlign::LEFT;
};
