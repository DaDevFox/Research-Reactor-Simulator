#pragma once
#include "../../include/CustomLabel.h"
#include "../../include/CustomWidget.h"
#include "../../include/RelativeGridLayout.h"
#include "pane.hpp"

class UpperLeftPane : public Pane
{
public:
  UpperLeftPane(Settings *properties, Simulator *reactor)
      : Pane(properties, reactor) {}

  nanogui::Vector2i consoleCoordinates() override { return Vector2i(-1, 0); }

  nanogui::Vector2i consoleDimensions() override { return Vector2i(1, 1); }

  void show(CustomWindow &baseWindow) override
  {
    if (root)
      return;
    root = baseWindow.add<CustomWidget>();
    root->setDrawBackground(true);
    root->setBackgroundColor(Color(28, 255));

    auto *layout = new RelativeGridLayout();
    layout->appendCol(1.f);
    layout->appendRow(1.f);
    root->setLayout(layout);

    auto *bars = root->add<CustomWidget>();
    bars->setDrawBackground(true);
    bars->setBackgroundColor(Color(36, 255));
    bars->setLayout(new GridLayout(Orientation::Horizontal, 5,
                                   Alignment::Middle, 10, 10));

    const std::string names[5] = {
        "Source\nNeutron", "Source\nPeriod", "Wide\nLog Power",
        "Wide\nPeriod", "Power\n%"};
    for (int i = 0; i < 5; ++i)
    {
      auto *bar = bars->add<CustomWidget>();
      bar->setDrawBackground(true);
      bar->setBackgroundColor(Color(70, 255));
      bar->setFixedSize(Vector2i(110, 300));
      auto *lbl = bar->add<CustomLabel>(names[i]);
      lbl->setPosition(Vector2i(8, 10));
      lbl->setColor(Color(220, 255));
    }
    layout->setAnchor(bars, RelativeGridLayout::makeAnchor(0, 0));

    if (auto *layout = dynamic_cast<RelativeGridLayout *>(baseWindow.layout()))
    {
      layout->setAnchor(root, RelativeGridLayout::makeAnchor(0, 0));
    }
  }

  void hide(CustomWindow &baseWindow) override
  {
    spdlog::info("Pane hide upper-left");
    safeRemoveFromBaseWindow(baseWindow, root,
                             "UpperLeftPane::hide(root)");
  }

private:
  CustomWidget *root = nullptr;
};
