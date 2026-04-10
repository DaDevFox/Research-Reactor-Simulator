#pragma once
#include "../../include/CustomGraph.h"
#include "../../include/CustomLabel.h"
#include "../../include/CustomWidget.h"
#include "../../include/RelativeGridLayout.h"
#include "pane.hpp"
#include <nanogui/common.h>

class LowerLeftPane : public Pane
{
public:
  LowerLeftPane(Settings *properties, Simulator *reactor)
      : Pane(properties, reactor) {}

  nanogui::Vector2i consoleCoordinates() override { return Vector2i(-1, -1); }

  nanogui::Vector2i consoleDimensions() override { return Vector2i(1, 1); }

  void show(CustomWindow &baseWindow) override
  {
    if (root)
      return;

    root = baseWindow.add<CustomWidget>();
    root->setDrawBackground(true);
    root->setBackgroundColor(Color(32, 255));
    auto *layout = new RelativeGridLayout();
    layout->appendCol(0.62f);
    layout->appendCol(0.38f);
    layout->appendRow(1.f);
    root->setLayout(layout);

    auto *annunciators = root->add<CustomWidget>();
    annunciators->setDrawBackground(true);
    annunciators->setBackgroundColor(Color(40, 255));
    auto *annLayout = new GridLayout(Orientation::Horizontal, 3,
                                     Alignment::Middle, 10, 10);
    annunciators->setLayout(annLayout);
    for (int i = 0; i < 9; ++i)
    {
      auto *pair = annunciators->add<CustomWidget>();
      pair->setDrawBackground(true);
      pair->setBackgroundColor(Color(85, 255));
      pair->setFixedSize(Vector2i(120, 88));
      auto *pairLayout = new RelativeGridLayout();
      pairLayout->appendCol(1.f);
      pairLayout->appendRow(0.5f);
      pairLayout->appendRow(0.5f);
      pair->setLayout(pairLayout);

      auto *high = pair->add<CustomLabel>("HIGH");
      high->setColor(Color(90, 180, 90, 255));
      auto *low = pair->add<CustomLabel>("LOW");
      low->setColor(Color(160, 160, 160, 255));
      pairLayout->setAnchor(high, RelativeGridLayout::makeAnchor(0, 0));
      pairLayout->setAnchor(low, RelativeGridLayout::makeAnchor(0, 1));
    }
    layout->setAnchor(annunciators, RelativeGridLayout::makeAnchor(0, 0));

    auto *readings = root->add<CustomWidget>();
    readings->setDrawBackground(true);
    readings->setBackgroundColor(Color(52, 255));
    auto *readLayout = new GridLayout(Orientation::Horizontal, 1,
                                      Alignment::Minimum, 6, 6);
    readings->setLayout(readLayout);
    readings->add<CustomLabel>("Radiation Readings");
    for (int i = 0; i < 6; ++i)
    {
      auto *r = readings->add<CustomLabel>("CH" + std::to_string(i + 1) +
                                           ": -- uSv/h");
      r->setColor(Color(220, 255));
    }
    layout->setAnchor(readings, RelativeGridLayout::makeAnchor(1, 0));

    if (auto *layout = dynamic_cast<RelativeGridLayout *>(baseWindow.layout()))
    {
      layout->setAnchor(root, RelativeGridLayout::makeAnchor(0, 0));
    }
  }

  void hide(CustomWindow &baseWindow) override
  {
    spdlog::info("Pane hide lower-left");
    safeRemoveFromBaseWindow(baseWindow, root,
                             "LowerLeftPane::hide(root)");
  }

private:
  CustomWidget *root = nullptr;
};
