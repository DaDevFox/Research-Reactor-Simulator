#pragma once
#include "../../include/CustomGraph.h"
#include "../../include/CustomLabel.h"
#include "../../include/CustomWidget.h"
#include "../../include/RelativeGridLayout.h"
#include "pane.hpp"
#include <nanogui/common.h>

class LowerCenterPane : public Pane
{
public:
  LowerCenterPane(Settings *properties, Simulator *reactor)
      : Pane(properties, reactor) {}

  LowerCenterPane(Settings *properties, Simulator *reactor,
                  CustomGraph **guiCanvas, Plot **guiPowerPlot,
                  Plot **guiTemperaturePlot, Plot **guiReactivityPlot,
                  Plot **guiRodReactivityPlot)
      : Pane(properties, reactor), guiCanvas(guiCanvas),
        guiPowerPlot(guiPowerPlot), guiTemperaturePlot(guiTemperaturePlot),
        guiReactivityPlot(guiReactivityPlot),
        guiRodReactivityPlot(guiRodReactivityPlot) {}

  nanogui::Vector2i consoleCoordinates() override { return nanogui::Vector2i(0, -1); }

  nanogui::Vector2i consoleDimensions() override { return nanogui::Vector2i(1, 1); }

  void createGraph(CustomWidget &graphHost)
  {
    // Create a graph object
    canvas = graphHost.add<CustomGraph>(4, "Main graph");
    if (auto *layout = dynamic_cast<RelativeGridLayout *>(graphHost.layout()))
    {
      layout->setAnchor(canvas, RelativeGridLayout::makeAnchor(0, 0));
    }

    canvas->setBackgroundColor(Color(250, 255));
    canvas->setDrawBackground(true);
    canvas->setPadding(90.f, 25.f,
                       properties->reactivityHardcore ? 120.f : 220.f, 50.f);

    // Create and save the plots
    rodReactivityPlot = canvas->addPlot(reactor->getDataLength(), true);
    temperaturePlot = canvas->addPlot(reactor->getDataLength(), true);
    reactivityPlot = canvas->addPlot(reactor->getDataLength(), true);
    powerPlot = canvas->addPlot(reactor->getDataLength(), true);

    // Bridge to SimulatorGUI's existing update pipeline.
    if (guiCanvas)
      *guiCanvas = canvas;
    if (guiPowerPlot)
      *guiPowerPlot = powerPlot;
    if (guiTemperaturePlot)
      *guiTemperaturePlot = temperaturePlot;
    if (guiReactivityPlot)
      *guiReactivityPlot = reactivityPlot;
    if (guiRodReactivityPlot)
      *guiRodReactivityPlot = rodReactivityPlot;

    // Styling
    canvas->setTextColor(Color(0, 255));
    reactivityPlot->setEnabled(!properties->reactivityHardcore);
    reactivityPlot->setName("Reactivity");
    reactivityPlot->setUnits("pcm");
    reactivityPlot->setColor(Color(0, 0, 255, 255));
    reactivityPlot->setFillColor(Color(0, 0, 255, 50));
    reactivityPlot->setPointerColor(Color(0, 0, 255, 255));
    reactivityPlot->setAxisShown(true);
    reactivityPlot->setMainLineShown(true);
    reactivityPlot->setTextShown(true);
    reactivityPlot->setNumberFormatMode(GraphElement::FormattingMode::Normal);
    reactivityPlot->setRoundFloating(true);
    reactivityPlot->setMajorTickNumber(7);
    reactivityPlot->setMinorTickNumber(2);
    reactivityPlot->setAxisPosition(GraphElement::AxisLocation::Right);
    reactivityPlot->setAxisOffset(110.f);
    reactivityPlot->setTextOffset(60.f);
    reactivityPlot->setFill(properties->curveFill);
    rodReactivityPlot->setEnabled(properties->rodReactivityPlot &&
                                  !properties->reactivityHardcore);
    rodReactivityPlot->setName("Rod position");
    rodReactivityPlot->setColor(Color(200, 255));
    rodReactivityPlot->setFillColor(Color(0, 0, 255, 10));
    rodReactivityPlot->setPointerColor(Color(180, 255));
    rodReactivityPlot->setAxisPosition(GraphElement::AxisLocation::Right);
    rodReactivityPlot->setAxisOffset(110.f);
    rodReactivityPlot->setFill(properties->curveFill);
    powerPlot->setName("Power");
    powerPlot->setUnits("W");
    powerPlot->setColor(Color(255, 0, 0, 255));
    powerPlot->setFillColor(Color(255, 0, 0, 50));
    powerPlot->setAxisShown(true);
    powerPlot->setYlog(properties->yAxisLog);
    powerPlot->setAxisPosition(GraphElement::AxisLocation::Right);
    powerPlot->setTextOffset(60.f);
    powerPlot->setMajorTickNumber(4);
    powerPlot->setMinorTickNumber(4);
    powerPlot->setTextShown(true);
    powerPlot->setNumberFormatMode(GraphElement::FormattingMode::Exponential);
    powerPlot->setDrawMode(DrawMode::Smart);
    powerPlot->setHorizontalAxisShown(true);
    powerPlot->setHorizontalMinorTickNumber(4);
    powerPlot->setHorizontalName("Time");
    powerPlot->setHorizontalTextOffset(20.f);
    powerPlot->setLimitOverride(0, "30 seconds ago");
    powerPlot->setLimitOverride(1, "now");
    powerPlot->setFill(properties->curveFill);
    temperaturePlot->setName("Temperature");
    temperaturePlot->setUnits(degCelsiusUnit);
    temperaturePlot->setColor(Color(0, 255, 0, 255));
    temperaturePlot->setFillColor(Color(0, 255, 0, 50));
    temperaturePlot->setPointerColor(Color(0, 255, 0, 255));
    temperaturePlot->setAxisShown(true);
    temperaturePlot->setMainLineShown(true);
    temperaturePlot->setTextShown(true);
    temperaturePlot->setNumberFormatMode(GraphElement::FormattingMode::Normal);
    temperaturePlot->setMajorTickNumber(3);
    temperaturePlot->setMinorTickNumber(4);
    temperaturePlot->setFill(properties->curveFill);

    // Link plots to data
    reactivityPlot->setXdata(reactor->time_);
    reactivityPlot->setYdata(reactor->reactivity_);
    rodReactivityPlot->setXdata(reactor->time_);
    rodReactivityPlot->setYdata(reactor->rodReactivity_);
    powerPlot->setXdata(reactor->time_);
    powerPlot->setYdata(reactor->state_vector_[0]);
    powerPlot->setValueComputing([this](double *val, const size_t /*index*/)
                                 { *val = reactor->powerFromNeutrons(*val); });
    temperaturePlot->setXdata(reactor->time_);
    temperaturePlot->setYdata(reactor->temperature_);
  }

  void show(CustomWindow &baseWindow) override
  {
    root->setDrawBackground(false);
    auto *rootLayout = new RelativeGridLayout();
    rootLayout->appendCol(1.f);
    rootLayout->appendRow(0.08f);
    rootLayout->appendRow(0.60f);
    rootLayout->appendRow(0.32f);
    root->setLayout(rootLayout);

    row = root->add<CustomWidget>();
    row->setDrawBackground(false);
    auto *rowLayout = new RelativeGridLayout();
    rowLayout->appendCol(0.30f);
    rowLayout->appendCol(0.45f);
    rowLayout->appendCol(0.25f);
    rowLayout->appendRow(1.f);
    row->setLayout(rowLayout);

    // Left side: 3 dial placeholders aligned to row center.
    dialStrip = row->add<CustomWidget>();
    dialStrip->setDrawBackground(true);
    dialStrip->setBackgroundColor(Color(35, 255));
    dialStrip->setLayout(
        new nanogui::BoxLayout(nanogui::Orientation::Horizontal, nanogui::Alignment::Middle, 10, 12));

    auto *shim1 = dialStrip->add<CustomLabel>("SHIM-1: -- %");
    auto *shim2 = dialStrip->add<CustomLabel>("SHIM-2: -- %");
    auto *reg = dialStrip->add<CustomLabel>("REG: -- %");
    shim1->setColor(Color(220, 255));
    shim2->setColor(Color(220, 255));
    reg->setColor(Color(220, 255));
    rowLayout->setAnchor(dialStrip, RelativeGridLayout::makeAnchor(0, 0));

    // Right side: graph in dark bordered frame.
    graphFrame = row->add<CustomWidget>();
    graphFrame->setDrawBackground(true);
    graphFrame->setBackgroundColor(Color(55, 255));
    auto *frameLayout = new RelativeGridLayout();
    frameLayout->appendCol(1.f);
    frameLayout->appendRow(1.f);
    graphFrame->setLayout(frameLayout);
    rowLayout->setAnchor(graphFrame, RelativeGridLayout::makeAnchor(1, 0));

    graphInset = graphFrame->add<CustomWidget>();
    graphInset->setDrawBackground(true);
    graphInset->setBackgroundColor(Color(245, 255));
    auto *insetLayout = new RelativeGridLayout();
    insetLayout->appendCol(1.f);
    insetLayout->appendRow(1.f);
    graphInset->setLayout(insetLayout);
    frameLayout->setAnchor(graphInset, RelativeGridLayout::makeAnchor(0, 0));

    createGraph(*graphInset);

    // Upright right-side blocks from plan: pump controls + automatic power.
    rightSidePanel = row->add<CustomWidget>();
    rightSidePanel->setDrawBackground(true);
    rightSidePanel->setBackgroundColor(Color(40, 255));
    auto *rightSideLayout = new RelativeGridLayout();
    rightSideLayout->appendCol(1.f);
    rightSideLayout->appendRow(1.f);
    rightSideLayout->appendRow(1.f);
    rightSidePanel->setLayout(rightSideLayout);

    auto *pumpControls = rightSidePanel->add<CustomWidget>();
    pumpControls->setDrawBackground(true);
    pumpControls->setBackgroundColor(Color(75, 255));
    auto *pumpLabel = pumpControls->add<CustomLabel>("Pump Controls");
    pumpLabel->setPosition(nanogui::Vector2i(10, 10));
    pumpLabel->setColor(Color(220, 255));
    rightSideLayout->setAnchor(pumpControls,
                               RelativeGridLayout::makeAnchor(0, 0));

    auto *autoPower = rightSidePanel->add<CustomWidget>();
    autoPower->setDrawBackground(true);
    autoPower->setBackgroundColor(Color(85, 255));
    auto *autoPowerLabel =
        autoPower->add<CustomLabel>("Automatic Power Settings (xScale)");
    autoPowerLabel->setPosition(nanogui::Vector2i(10, 10));
    autoPowerLabel->setColor(Color(220, 255));
    rightSideLayout->setAnchor(autoPower, RelativeGridLayout::makeAnchor(0, 1));
    rowLayout->setAnchor(rightSidePanel, RelativeGridLayout::makeAnchor(2, 0));

    rootLayout->setAnchor(row, RelativeGridLayout::makeAnchor(0, 1));

    // Lower panel blocks from plan.
    lowerPanel = root->add<CustomWidget>();
    lowerPanel->setDrawBackground(true);
    lowerPanel->setBackgroundColor(Color(28, 255));
    auto *lowerLayout = new RelativeGridLayout();
    lowerLayout->appendCol(0.20f);
    lowerLayout->appendCol(0.50f);
    lowerLayout->appendCol(0.30f);
    lowerLayout->appendRow(1.f);
    lowerPanel->setLayout(lowerLayout);

    auto *centerControls = lowerPanel->add<CustomWidget>();
    centerControls->setDrawBackground(true);
    centerControls->setBackgroundColor(Color(55, 255));
    auto *centerControlsLayout = new RelativeGridLayout();
    centerControlsLayout->appendCol(1.f);
    centerControlsLayout->appendRow(0.22f);
    centerControlsLayout->appendRow(0.26f);
    centerControlsLayout->appendRow(0.26f);
    centerControlsLayout->appendRow(0.26f);
    centerControls->setLayout(centerControlsLayout);

    auto *scram = centerControls->add<CustomWidget>();
    scram->setDrawBackground(true);
    scram->setBackgroundColor(Color(150, 120, 0, 255));
    auto *scramLabel = scram->add<CustomLabel>("MANUAL SCRAM");
    scramLabel->setPosition(nanogui::Vector2i(10, 8));
    centerControlsLayout->setAnchor(scram, RelativeGridLayout::makeAnchor(0, 0));

    auto addButtonRow = [&](int rowIndex, const std::string &caption)
    {
      auto *r = centerControls->add<CustomWidget>();
      r->setDrawBackground(true);
      r->setBackgroundColor(Color(75, 255));
      auto *lbl = r->add<CustomLabel>(caption);
      lbl->setPosition(nanogui::Vector2i(10, 6));
      lbl->setColor(Color(220, 255));
      centerControlsLayout->setAnchor(r,
                                      RelativeGridLayout::makeAnchor(0, rowIndex));
    };
    addButtonRow(1, "Button Grid Row 1 (Rod Up/Down)");
    addButtonRow(2, "Button Grid Row 2");
    addButtonRow(3, "Button Grid Row 3");
    lowerLayout->setAnchor(centerControls, RelativeGridLayout::makeAnchor(1, 0));

    auto *cic = lowerPanel->add<CustomWidget>();
    cic->setDrawBackground(true);
    cic->setBackgroundColor(Color(70, 255));
    auto *cicLabel = cic->add<CustomLabel>("CIC Scale Power Control");
    cicLabel->setPosition(nanogui::Vector2i(10, 10));
    cicLabel->setColor(Color(220, 255));
    lowerLayout->setAnchor(cic, RelativeGridLayout::makeAnchor(2, 0));

    rootLayout->setAnchor(lowerPanel, RelativeGridLayout::makeAnchor(0, 2));

    if (auto *layout = dynamic_cast<RelativeGridLayout *>(baseWindow.layout()))
    {
      layout->setAnchor(root, RelativeGridLayout::makeAnchor(0, 0));
    }

    spdlog::info("Pane show lower-center done");
  }

  void hide(CustomWindow &baseWindow) override
  {
    spdlog::info("Pane hide lower-center");

    if (guiCanvas && *guiCanvas == canvas)
      *guiCanvas = nullptr;
    if (guiPowerPlot && *guiPowerPlot == powerPlot)
      *guiPowerPlot = nullptr;
    if (guiTemperaturePlot && *guiTemperaturePlot == temperaturePlot)
      *guiTemperaturePlot = nullptr;
    if (guiReactivityPlot && *guiReactivityPlot == reactivityPlot)
      *guiReactivityPlot = nullptr;
    if (guiRodReactivityPlot && *guiRodReactivityPlot == rodReactivityPlot)
      *guiRodReactivityPlot = nullptr;

    powerPlot = nullptr;
    temperaturePlot = nullptr;
    reactivityPlot = nullptr;
    rodReactivityPlot = nullptr;
    canvas = nullptr;
    graphInset = nullptr;
    graphFrame = nullptr;
    rightSidePanel = nullptr;
    lowerPanel = nullptr;
    row = nullptr;
    dialStrip = nullptr;
    safeRemoveFromBaseWindow(baseWindow, root, "LowerCenterPane::hide(root)");
  }

private:
  CustomGraph **guiCanvas = nullptr;
  Plot **guiPowerPlot = nullptr;
  Plot **guiTemperaturePlot = nullptr;
  Plot **guiReactivityPlot = nullptr;
  Plot **guiRodReactivityPlot = nullptr;
  CustomWidget *root = nullptr;
  CustomWidget *row = nullptr;
  CustomWidget *graphFrame = nullptr;
  CustomWidget *graphInset = nullptr;
  CustomWidget *rightSidePanel = nullptr;
  CustomWidget *lowerPanel = nullptr;
  CustomGraph *canvas = nullptr;
  CustomWidget *dialStrip = nullptr;
  Plot *powerPlot = nullptr;
  Plot *temperaturePlot = nullptr;
  Plot *reactivityPlot = nullptr;
  Plot *rodReactivityPlot = nullptr;
  const std::string degCelsiusUnit = std::string(nanogui::utf8(0xBA).data()) + "C";
};
