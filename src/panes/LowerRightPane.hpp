#pragma once
#include "../../include/CustomGraph.h"
#include "../../include/CustomLabel.h"
#include "../../include/CustomWidget.h"
#include "../../include/RelativeGridLayout.h"
#include "pane.hpp"
#include <nanogui/common.h>

class LowerRightPane : public Pane {
public:
  LowerRightPane(Settings *properties, Simulator *reactor)
      : Pane(properties, reactor) {}

  nanogui::Vector2i consoleCoordinates() override { return Vector2i(1, -1); }

  nanogui::Vector2i consoleDimensions() override { return Vector2i(1, 1); }

  void show(CustomWindow &baseWindow) override {
    spdlog::info("Pane show lower-right start");
    if (root) return;

    root = baseWindow.add<CustomWidget>();
    root->setDrawBackground(true);
    root->setBackgroundColor(Color(34, 255));
    root->setLayout(new BoxLayout(Orientation::Horizontal, Alignment::Middle,
                                  12, 12));

    auto addTile = [&](const std::string &text) {
      auto *tile = root->add<CustomWidget>();
      tile->setDrawBackground(true);
      tile->setBackgroundColor(Color(245, 255));
      tile->setFixedSize(Vector2i(210, 210));
      auto *label = tile->add<CustomLabel>(text);
      label->setColor(Color(32, 255));
      label->setPosition(Vector2i(16, 16));
    };

    addTile("Log power gauge\nTODO needle dial");
    addTile("Pool temperature");
    addTile("Fuel temperature");

    if (auto *layout = dynamic_cast<RelativeGridLayout *>(baseWindow.layout())) {
      layout->setAnchor(root, RelativeGridLayout::makeAnchor(0, 0));
    }
    spdlog::info("Pane show lower-right done");

  }

  void hide(CustomWindow &baseWindow) override {
    spdlog::info("Pane hide lower-right");
    safeRemoveFromBaseWindow(baseWindow, root,
                             "LowerRightPane::hide(root)");
  }

 private:
  CustomWidget *root = nullptr;
};
