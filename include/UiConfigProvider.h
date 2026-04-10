#pragma once

#include <nanogui/common.h>

struct UiStyleConfig {
  nanogui::Color accentColor;
  nanogui::Color scramAlertColor;
  nanogui::Color scramWarningColor;
  nanogui::Color scramNormalColor;
  nanogui::Color baseWindowBackgroundColor;
  nanogui::Color bottomPanelBackgroundColor;
  nanogui::Color themeTextColor;
  float tabInnerMargin;
  float standardFontSize;
  float buttonCornerRadius;
  float tabMaxButtonWidth;
  float tabButtonVerticalPadding;
  float textBoxFontSize;
};

struct UiPanelLayoutConfig {
  int panelMargin;
  int panelSpacing;
};

class IUiConfigProvider {
 public:
  virtual ~IUiConfigProvider() = default;
  virtual UiStyleConfig getStyleConfig() const = 0;
  virtual UiPanelLayoutConfig getPanelLayoutConfig() const = 0;
};

class DefaultUiConfigProvider : public IUiConfigProvider {
 public:
  UiStyleConfig getStyleConfig() const override;
  UiPanelLayoutConfig getPanelLayoutConfig() const override;
};
