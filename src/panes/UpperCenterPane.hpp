#pragma once
#include "../../include/CustomGraph.h"
#include "../../include/CustomLabel.h"
#include "../../include/CustomWidget.h"
#include "../../include/RelativeGridLayout.h"
#include "pane.hpp"

class UpperCenterPane : public Pane
{
public:
  UpperCenterPane(Settings *properties, Simulator *reactor)
      : Pane(properties, reactor) {}

  UpperCenterPane(Settings *properties, Simulator *reactor,
                  CustomGraph **guiCanvas, Plot **guiPowerPlot,
                  Plot **guiTemperaturePlot, Plot **guiReactivityPlot,
                  Plot **guiRodReactivityPlot)
      : Pane(properties, reactor), guiCanvas(guiCanvas),
        guiPowerPlot(guiPowerPlot), guiTemperaturePlot(guiTemperaturePlot),
        guiReactivityPlot(guiReactivityPlot),
        guiRodReactivityPlot(guiRodReactivityPlot) {}

  nanogui::Vector2i consoleCoordinates() override { return Vector2i(0, 0); }

  nanogui::Vector2i consoleDimensions() override { return Vector2i(1, 1); }

  void show(CustomWindow &baseWindow) override
  {
    if (root)
      return;
    root = baseWindow.add<CustomWidget>();
    root->setDrawBackground(false);
    auto *layout = new RelativeGridLayout();
    layout->appendCol(1.f);
    layout->appendRow(1.f);
    root->setLayout(layout);

    canvas = root->add<CustomGraph>(4, "Main graph");
    layout->setAnchor(canvas, RelativeGridLayout::makeAnchor(0, 0));
    canvas->setBackgroundColor(Color(250, 255));
    canvas->setDrawBackground(true);
    canvas->setPadding(90.f, 25.f,
                       properties->reactivityHardcore ? 120.f : 220.f, 50.f);

    rodReactivityPlot = canvas->addPlot(reactor->getDataLength(), true);
    temperaturePlot = canvas->addPlot(reactor->getDataLength(), true);
    reactivityPlot = canvas->addPlot(reactor->getDataLength(), true);
    powerPlot = canvas->addPlot(reactor->getDataLength(), true);

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

    if (auto *layout = dynamic_cast<RelativeGridLayout *>(baseWindow.layout()))
    {
      layout->setAnchor(root, RelativeGridLayout::makeAnchor(0, 0));
    }
  }

  void hide(CustomWindow &baseWindow) override
  {
    spdlog::info("Pane hide upper-center");

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

    canvas = nullptr;
    powerPlot = nullptr;
    temperaturePlot = nullptr;
    reactivityPlot = nullptr;
    rodReactivityPlot = nullptr;
    safeRemoveFromBaseWindow(baseWindow, root,
                             "UpperCenterPane::hide(root)");
  }

private:
  CustomGraph **guiCanvas = nullptr;
  Plot **guiPowerPlot = nullptr;
  Plot **guiTemperaturePlot = nullptr;
  Plot **guiReactivityPlot = nullptr;
  Plot **guiRodReactivityPlot = nullptr;
  CustomGraph *canvas = nullptr;
  Plot *powerPlot = nullptr;
  Plot *temperaturePlot = nullptr;
  Plot *reactivityPlot = nullptr;
  Plot *rodReactivityPlot = nullptr;
  const std::string degCelsiusUnit = std::string(utf8(0xBA).data()) + "C";
  CustomWidget *root = nullptr;
};
