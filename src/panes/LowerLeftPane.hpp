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
    root->setLayout(new GridLayout(Orientation::Horizontal, 2,
                                   Alignment::Middle, 12, 12));

    auto makePanel = [&](const std::string &title) {
      auto *panel = root->add<CustomWidget>();
      panel->setDrawBackground(true);
      panel->setBackgroundColor(Color(45, 255));
      panel->setLayout(new GridLayout(Orientation::Horizontal, 3,
                                      Alignment::Middle, 6, 6));
      panel->add<CustomLabel>(title);
      for (int i = 0; i < 9; ++i) {
        auto *label = panel->add<CustomLabel>("LIGHT " + std::to_string(i + 1));
        label->setColor(Color(220, 255));
      }
    };

    makePanel("Warnings A");
    makePanel("Warnings B");

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
