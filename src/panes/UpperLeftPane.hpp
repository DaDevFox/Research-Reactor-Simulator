#pragma once
#include "../../include/CustomLabel.h"
#include "../../include/CustomWidget.h"
#include "../../include/RelativeGridLayout.h"
#include "pane.hpp"

class UpperLeftPane : public Pane {
 public:
  UpperLeftPane(Settings *properties, Simulator *reactor)
      : Pane(properties, reactor) {}

  nanogui::Vector2i consoleCoordinates() override { return Vector2i(-1, 0); }

  nanogui::Vector2i consoleDimensions() override { return Vector2i(1, 1); }

  void show(CustomWindow &baseWindow) override {
    spdlog::info("Pane show upper-left start");
    if (root) return;
    root = baseWindow.add<CustomWidget>();
    root->setDrawBackground(true);
    root->setBackgroundColor(Color(28, 255));
    auto *label = root->add<CustomLabel>("Upper-left pane");
    label->setPosition(Vector2i(18, 18));

    if (auto *layout = dynamic_cast<RelativeGridLayout *>(baseWindow.layout())) {
      layout->setAnchor(root, RelativeGridLayout::makeAnchor(0, 0));
    }
    spdlog::info("Pane show upper-left done");
  }

  void hide(CustomWindow &baseWindow) override {
    spdlog::info("Pane hide upper-left");
    safeRemoveFromBaseWindow(baseWindow, root,
                             "UpperLeftPane::hide(root)");
  }

 private:
  CustomWidget *root = nullptr;
};
