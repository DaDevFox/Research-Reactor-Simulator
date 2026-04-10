#pragma once
#include "../../include/CustomLabel.h"
#include "../../include/CustomWidget.h"
#include "../../include/RelativeGridLayout.h"
#include "pane.hpp"

class UpperRightPane : public Pane
{
public:
    UpperRightPane(Settings *properties, Simulator *reactor)
        : Pane(properties, reactor) {}

    nanogui::Vector2i consoleCoordinates() override { return Vector2i(1, 0); }

    nanogui::Vector2i consoleDimensions() override { return Vector2i(1, 1); }

    void show(CustomWindow &baseWindow) override
    {
        if (root)
            return;
        root = baseWindow.add<CustomWidget>();
        root->setDrawBackground(true);
        root->setBackgroundColor(Color(28, 255));

        auto *layout = new RelativeGridLayout();
        layout->appendCol(0.5f);
        layout->appendCol(0.5f);
        layout->appendRow(1.f);
        root->setLayout(layout);

        auto *leftGauges = root->add<CustomWidget>();
        leftGauges->setDrawBackground(true);
        leftGauges->setBackgroundColor(Color(42, 255));
        leftGauges->setLayout(new GridLayout(Orientation::Horizontal, 1,
                                             Alignment::Middle, 8, 8));
        auto *g1 = leftGauges->add<CustomLabel>("Pool Conductivity\n(radial gauge)");
        auto *g2 = leftGauges->add<CustomLabel>("Flow Rate\n(radial gauge)");
        g1->setColor(Color(220, 255));
        g2->setColor(Color(220, 255));
        layout->setAnchor(leftGauges, RelativeGridLayout::makeAnchor(0, 0));

        auto *rightGrid = root->add<CustomWidget>();
        rightGrid->setDrawBackground(true);
        rightGrid->setBackgroundColor(Color(52, 255));
        rightGrid->setLayout(new GridLayout(Orientation::Horizontal, 2,
                                            Alignment::Middle, 10, 10));
        const std::string sensors[4] = {
            "Fuel temp", "Pool level", "Pool temp 1", "Pool temp 2"};
        for (const auto &name : sensors)
        {
            auto *cell = rightGrid->add<CustomWidget>();
            cell->setDrawBackground(true);
            cell->setBackgroundColor(Color(75, 255));
            cell->setFixedSize(Vector2i(170, 120));
            auto *lbl = cell->add<CustomLabel>(name + "\n--");
            lbl->setPosition(Vector2i(8, 8));
            lbl->setColor(Color(220, 255));
        }
        layout->setAnchor(rightGrid, RelativeGridLayout::makeAnchor(1, 0));

        if (auto *layout = dynamic_cast<RelativeGridLayout *>(baseWindow.layout()))
        {
            layout->setAnchor(root, RelativeGridLayout::makeAnchor(0, 0));
        }
    }

    void hide(CustomWindow &baseWindow) override
    {
        spdlog::info("Pane hide upper-right");
        safeRemoveFromBaseWindow(baseWindow, root,
                                 "UpperRightPane::hide(root)");
    }

private:
    CustomWidget *root = nullptr;
};
