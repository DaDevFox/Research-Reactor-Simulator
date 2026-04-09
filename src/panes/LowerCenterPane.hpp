#pragma once
#include "../../include/CustomGraph.h"
#include "../../include/CustomLabel.h"
#include "../../include/CustomWidget.h"
#include "../../include/RelativeGridLayout.h"
#include "pane.hpp"
#include <nanogui/common.h>

class LowerCenterPane : public Pane {
public:
  LowerCenterPane(Settings *properties, Simulator *reactor)
      : Pane(properties, reactor) {}

  nanogui::Vector2i consoleCoordinates() override { return Vector2i(0, -1); }

  nanogui::Vector2i consoleDimensions() override { return Vector2i(1, 1); }

  void createGraph(CustomWindow &baseWindow) {
    // Create a graph object
    canvas = baseWindow.add<CustomGraph>(4, "Main graph");
    if (auto *layout = dynamic_cast<RelativeGridLayout *>(baseWindow.layout())) {
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
    powerPlot->setValueComputing([this](double *val, const size_t /*index*/) {
      *val = reactor->powerFromNeutrons(*val);
    });
    temperaturePlot->setXdata(reactor->time_);
    temperaturePlot->setYdata(reactor->temperature_);
  }

  void show(CustomWindow &baseWindow) override {
    spdlog::info("Pane show lower-center start");
    createGraph(baseWindow);

    // Placeholder band for the 3 rod dials until the real dial widget lands.
    dialStrip = baseWindow.add<CustomWidget>();
    dialStrip->setDrawBackground(true);
    dialStrip->setBackgroundColor(Color(35, 255));
    dialStrip->setFixedHeight(52);
    dialStrip->setLayout(new BoxLayout(Orientation::Horizontal, Alignment::Middle,
                                       10, 8));

    auto *shim1 = dialStrip->add<CustomLabel>("SHIM-1: -- %");
    auto *shim2 = dialStrip->add<CustomLabel>("SHIM-2: -- %");
    auto *reg = dialStrip->add<CustomLabel>("REG: -- %");
    shim1->setColor(Color(220, 255));
    shim2->setColor(Color(220, 255));
    reg->setColor(Color(220, 255));

    if (auto *layout = dynamic_cast<RelativeGridLayout *>(baseWindow.layout())) {
      layout->setAnchor(dialStrip, RelativeGridLayout::makeAnchor(0, 0));
    }
    spdlog::info("Pane show lower-center done");
  }

  void hide(CustomWindow &baseWindow) override {
    spdlog::info("Pane hide lower-center");
    safeRemoveFromBaseWindow(baseWindow, dialStrip,
                             "LowerCenterPane::hide(dialStrip)");
    safeRemoveFromBaseWindow(baseWindow, canvas,
                             "LowerCenterPane::hide(canvas)");
  }

private:
  CustomGraph *canvas = nullptr;
  CustomWidget *dialStrip = nullptr;
  Plot *powerPlot = nullptr;
  Plot *temperaturePlot = nullptr;
  Plot *reactivityPlot = nullptr;
  Plot *rodReactivityPlot = nullptr;
  const std::string degCelsiusUnit = std::string(utf8(0xBA).data()) + "C";
};
