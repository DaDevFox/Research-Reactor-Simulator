#include "../include/UiConfigProvider.h"

UiStyleConfig DefaultUiConfigProvider::getStyleConfig() const {
  UiStyleConfig cfg{};
  cfg.accentColor = nanogui::Color(77, 184, 255, 255);
  cfg.scramAlertColor = nanogui::Color(255, 0, 0, 255);
  cfg.scramWarningColor = nanogui::Color(175, 100, 0, 255);
  cfg.scramNormalColor = nanogui::Color(120, 120);
  cfg.baseWindowBackgroundColor = nanogui::Color(80, 255);
  cfg.bottomPanelBackgroundColor = nanogui::Color(35, 255);
  cfg.themeTextColor = nanogui::Color(0.92f, 1.f);
  cfg.tabInnerMargin = 0.f;
  cfg.standardFontSize = 18.f;
  cfg.buttonCornerRadius = 2.f;
  cfg.tabMaxButtonWidth = 250.f;
  cfg.tabButtonVerticalPadding = 7.f;
  cfg.textBoxFontSize = 18.f;
  return cfg;
}

UiPanelLayoutConfig DefaultUiConfigProvider::getPanelLayoutConfig() const {
  UiPanelLayoutConfig cfg{};
  cfg.panelMargin = 0;
  cfg.panelSpacing = 10;
  return cfg;
}
