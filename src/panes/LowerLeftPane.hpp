#pragma once
#include "../../include/CustomGraph.h"
#include "../../include/CustomLabel.h"
#include "../../include/CustomWidget.h"
#include "../../include/RelativeGridLayout.h"
#include "pane.hpp"
#include <nanogui/common.h>

class LowerLeftPane : public Pane {
public:
  LowerLeftPane(Settings *properties, Simulator *reactor)
      : Pane(properties, reactor) {}

  nanogui::Vector2i consoleCoordinates() override { return Vector2i(-1, -1); }

  nanogui::Vector2i consoleDimensions() override { return Vector2i(1, 1); }

  void show(CustomWindow &baseWindow) override {
    spdlog::info("Pane show lower-left start");
    if (root) return;

    root = baseWindow.add<CustomWidget>();
    root->setDrawBackground(true);
    root->setBackgroundColor(Color(32, 255));
    auto *label = root->add<CustomLabel>("Lower-left pane");
    label->setPosition(Vector2i(18, 18));
    label->setColor(Color(220, 255));

    if (auto *layout = dynamic_cast<RelativeGridLayout *>(baseWindow.layout())) {
      layout->setAnchor(root, RelativeGridLayout::makeAnchor(0, 0));
    }
    spdlog::info("Pane show lower-left done");
  }

  void hide(CustomWindow &baseWindow) override {
    spdlog::info("Pane hide lower-left");
    safeRemoveFromBaseWindow(baseWindow, root,
                             "LowerLeftPane::hide(root)");
  }

 private:
  CustomWidget *root = nullptr;
};
