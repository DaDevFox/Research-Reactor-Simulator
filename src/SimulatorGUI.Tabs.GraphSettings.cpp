#include "../include/SimulatorGUI.h"

void SimulatorGUI::createGraphSettingsTab() {
  Widget *graph_controls = tabControl->createTab("Graph controls");
  graph_controls->setId("graph controls");
  RelativeGridLayout *graphControlsLayout = new RelativeGridLayout();
  graphControlsLayout->appendRow(
      RelativeGridLayout::Size(140.f, RelativeGridLayout::SizeType::Fixed));
  graphControlsLayout->appendRow(
      RelativeGridLayout::Size(10.f, RelativeGridLayout::SizeType::Fixed));
  graphControlsLayout->appendRow(
      RelativeGridLayout::Size(2.f, RelativeGridLayout::SizeType::Fixed));
  graphControlsLayout->appendRow(
      RelativeGridLayout::Size(30.f, RelativeGridLayout::SizeType::Fixed));
  graphControlsLayout->appendRow(
      RelativeGridLayout::Size(20.f, RelativeGridLayout::SizeType::Fixed));
  graphControlsLayout->appendRow(1.f);
  graphControlsLayout->appendCol(1.f);
  graphControlsLayout->appendCol(1.f);
  graph_controls->setLayout(graphControlsLayout);
  CustomWidget *border = graph_controls->add<CustomWidget>();
  border->setBackgroundColor(coolBlue);
  border->setDrawBackground(true);
  graphControlsLayout->setAnchor(border,
                                 RelativeGridLayout::makeAnchor(0, 2, 2, 1));

  // Create a panel for graph size
  Widget *generalLeftPanel = new Widget(graph_controls);
  graphControlsLayout->setAnchor(generalLeftPanel,
                                 RelativeGridLayout::makeAnchor(0, 0));
  generalLeftPanel->setLayout(
      new BoxLayout(Orientation::Vertical, Alignment::Minimum, 10, 10));
  Widget *graphSizePanel = generalLeftPanel->add<Widget>();
  graphSizePanel->setLayout(panelsLayout);

  graphSizePanel->add<Label>("Graph size :", "sans-bold");
  graphSizeBox =
      graphSizePanel->add<IntBox<int>>((int)(100 * properties->graphSize));
  graphSizeBox->setUnits("%");
  graphSizeBox->setDefaultValue(
      to_string((int)std::roundf(properties->graphSize * 100)));
  graphSizeBox->setFontSize(16);
  graphSizeBox->setFormat("[0-9]+");
  graphSizeBox->setSpinnable(true);
  graphSizeBox->setMinValue(30);
  graphSizeBox->setMaxValue(70);
  graphSizeBox->setValueIncrement(1);
  graphSizeBox->setCallback([this](int a) {
    if (a > 70) a = 70;
    if (a < 30) a = 30;
    relativeLayout->setRowSize(0, a / 100.f);
    relativeLayout->setRowSize(1, 1.f - a / 100.f);
    performLayout();
    properties->graphSize = a / 100.f;
  });

  // Create a panel for display time
  Widget *timePanel = generalLeftPanel->add<Widget>();
  timePanel->setLayout(panelsLayout);

  // Create a panel for graph limits
  Widget *reactivityLimitsPanel = generalLeftPanel->add<Widget>();
  reactivityLimitsPanel->setLayout(panelsLayout);

  // Create another panel for graph limits
  Widget *temperatureLimitsPanel = generalLeftPanel->add<Widget>();
  temperatureLimitsPanel->setLayout(panelsLayout);

  // Create a panel for curve fill, rod reactivity line visibility, log scale
  // power and hardcore mode
  Widget *sliderPanel = graph_controls->add<Widget>();
  graphControlsLayout->setAnchor(
      sliderPanel, RelativeGridLayout::makeAnchor(
                       1, 0, 1, 1, Alignment::Maximum, Alignment::Fill));

  RelativeGridLayout *sliderLayout = new RelativeGridLayout();
  for (int i = 0; i < 4; i++)
    sliderLayout->appendCol((i % 2)
                                ? RelativeGridLayout::Size(
                                      10.f, RelativeGridLayout::SizeType::Fixed)
                                : 1.f);
  for (int i = 0; i < 4; i++) sliderLayout->appendRow(1.f);
  sliderPanel->setLayout(sliderLayout);

  sliderLayout->setAnchor(
      sliderPanel->add<Label>("Rod reactivity plot:", "sans-bold"),
      RelativeGridLayout::makeAnchor(0, 0, 1, 1, Alignment::Minimum,
                                     Alignment::Middle));
  rodReactivityBox = sliderPanel->add<SliderCheckBox>();
  sliderLayout->setAnchor(
      rodReactivityBox, RelativeGridLayout::makeAnchor(
                            2, 0, 1, 1, Alignment::Maximum, Alignment::Middle));
  rodReactivityBox->setFontSize(16);
  rodReactivityBox->setChecked(properties->rodReactivityPlot);
  rodReactivityBox->setCallback([this](bool value) {
    properties->rodReactivityPlot = value;
    rodReactivityPlot->setEnabled(value && !properties->reactivityHardcore);
  });

  sliderLayout->setAnchor(
      sliderPanel->add<Label>("Curve fill:", "sans-bold"),
      RelativeGridLayout::makeAnchor(0, 1, 1, 1, Alignment::Minimum,
                                     Alignment::Middle));
  curveFillBox = sliderPanel->add<SliderCheckBox>();
  sliderLayout->setAnchor(
      curveFillBox, RelativeGridLayout::makeAnchor(
                        2, 1, 1, 1, Alignment::Maximum, Alignment::Middle));
  curveFillBox->setFontSize(16);
  curveFillBox->setChecked(properties->curveFill);
  curveFillBox->setCallback([this](bool value) {
    rodReactivityPlot->setFill(value);
    reactivityPlot->setFill(value);
    powerPlot->setFill(value);
    temperaturePlot->setFill(value);
    properties->curveFill = value;
  });

  timePanel->add<Label>("Display time :", "sans-bold");
  displayBox = timePanel->add<FloatBox<float>>(properties->displayTime);
  displayBox->setFixedSize(Vector2i(100, 20));
  displayBox->setUnits("s");
  displayBox->setDefaultValue(
      formatDecimalsDouble((double)properties->displayTime, 1));
  displayBox->setFontSize(16);
  displayBox->setFormat("[0-9]*[.]?[0-9]?");
  displayBox->setSpinnable(true);
  displayBox->setMinMaxValues(0.5f, (float)reactor->getDeleteOldValues());
  displayBox->setValueIncrement(1.f);
  displayBox->setCallback([this](float a) {
    properties->displayTime =
        std::min((float)reactor->getDeleteOldValues(), std::max(a, 0.5f));
    std::string limit =
        formatDecimalsDouble((double)properties->displayTime, 1) +
        " seconds ago";
    powerPlot->setLimitOverride(0, limit);
    delayedGroups[0]->setLimitOverride(0, limit);
  });

  {
    Label *temp = reactivityLimitsPanel->add<Label>("Reactivity graph:  from ",
                                                    "sans-bold");
    temp->setFixedWidth(160);
    reactivityLimitBox[0] = reactivityLimitsPanel->add<FloatBox<float>>(
        properties->reactivityGraphLimits[0]);
    reactivityLimitBox[0]->setFixedSize(Vector2i(100, 20));
    reactivityLimitBox[0]->setUnits("pcm");
    reactivityLimitBox[0]->setDefaultValue(
        to_string(properties->reactivityGraphLimits[0]));
    reactivityLimitBox[0]->setFontSize(16);
    reactivityLimitBox[0]->setFormat("[-]?[0-9]*[.]?[0-9]?");
    reactivityLimitBox[0]->setSpinnable(true);
    reactivityLimitBox[0]->setValueIncrement(1.f);
    reactivityLimitBox[0]->setCallback(
        [this](float a) { properties->reactivityGraphLimits[0] = a; });

    reactivityLimitsPanel->add<Label>(" to ", "sans-bold");
    reactivityLimitBox[1] = reactivityLimitsPanel->add<FloatBox<float>>(
        properties->reactivityGraphLimits[1]);
    reactivityLimitBox[1]->setFixedSize(Vector2i(100, 20));
    reactivityLimitBox[1]->setUnits("pcm");
    reactivityLimitBox[1]->setDefaultValue(
        to_string(properties->reactivityGraphLimits[1]));
    reactivityLimitBox[1]->setFontSize(16);
    reactivityLimitBox[1]->setFormat("[-]?[0-9]*[.]?[0-9]?");
    reactivityLimitBox[1]->setSpinnable(true);
    reactivityLimitBox[1]->setValueIncrement(1.f);
    reactivityLimitBox[1]->setCallback(
        [this](float a) { properties->reactivityGraphLimits[1] = a; });
    Button *btn = reactivityLimitsPanel->add<Button>("Reset");
    btn->setCallback([this]() {
      reactivityLimitBox[0]->setValue(reactor->getExcessReactivity() -
                                      reactor->getTotalRodWorth());
      reactivityLimitBox[1]->setValue(
          static_cast<int>(reactor->getExcessReactivity()));
    });
  }

  {
    // Temperature stuff
    Label *temp = temperatureLimitsPanel->add<Label>(
        "Temperature graph:  from ", "sans-bold");
    temp->setFixedWidth(160);
    temperatureLimitBox[0] = temperatureLimitsPanel->add<FloatBox<float>>(
        properties->temperatureGraphLimits[0]);
    temperatureLimitBox[0]->setFixedSize(Vector2i(100, 20));
    temperatureLimitBox[0]->setUnits(degCelsiusUnit);
    temperatureLimitBox[0]->setDefaultValue(
        to_string(properties->temperatureGraphLimits[0]));
    temperatureLimitBox[0]->setFontSize(16);
    temperatureLimitBox[0]->setFormat("[-]?[0-9]*[.]?[0-9]?");
    temperatureLimitBox[0]->setMinValue(0.f);
    temperatureLimitBox[0]->setSpinnable(true);
    temperatureLimitBox[0]->setValueIncrement(1.f);
    temperatureLimitBox[0]->setCallback(
        [this](float a) { properties->temperatureGraphLimits[0] = a; });

    temperatureLimitsPanel->add<Label>(" to ", "sans-bold");
    temperatureLimitBox[1] = temperatureLimitsPanel->add<FloatBox<float>>(
        properties->temperatureGraphLimits[1]);
    temperatureLimitBox[1]->setFixedSize(Vector2i(100, 20));
    temperatureLimitBox[1]->setUnits(degCelsiusUnit);
    temperatureLimitBox[1]->setDefaultValue(
        to_string(properties->temperatureGraphLimits[1]));
    temperatureLimitBox[1]->setFontSize(16);
    temperatureLimitBox[1]->setFormat("[-]?[0-9]*[.]?[0-9]?");
    temperatureLimitBox[1]->setMinValue(0.f);
    temperatureLimitBox[1]->setSpinnable(true);
    temperatureLimitBox[1]->setValueIncrement(1.f);
    temperatureLimitBox[1]->setCallback(
        [this](float a) { properties->temperatureGraphLimits[1] = a; });

    Button *btn = temperatureLimitsPanel->add<Button>("Reset");
    btn->setCallback([this]() {
      temperatureLimitBox[0]->setValue(TEMPERATURE_GRAPH_FROM_DEFAULT);
      temperatureLimitBox[1]->setValue(TEMPERATURE_GRAPH_TO_DEFAULT);
    });
  }

  sliderLayout->setAnchor(
      sliderPanel->add<Label>("Power log scale:", "sans-bold"),
      RelativeGridLayout::makeAnchor(0, 2, 1, 1, Alignment::Minimum,
                                     Alignment::Middle));
  logScaleBox = sliderPanel->add<SliderCheckBox>();
  sliderLayout->setAnchor(
      logScaleBox, RelativeGridLayout::makeAnchor(
                       2, 2, 1, 1, Alignment::Maximum, Alignment::Middle));
  logScaleBox->setFontSize(16);
  logScaleBox->setChecked(properties->yAxisLog);
  logScaleBox->setCallback([this](bool value) {
    properties->yAxisLog = value;
    powerPlot->setYlog(value);
  });

  sliderLayout->setAnchor(
      sliderPanel->add<Label>("Hide reactivity:", "sans-bold"),
      RelativeGridLayout::makeAnchor(0, 3, 1, 1, Alignment::Minimum,
                                     Alignment::Middle));
  hardcoreBox = sliderPanel->add<SliderCheckBox>();
  sliderLayout->setAnchor(
      hardcoreBox, RelativeGridLayout::makeAnchor(
                       2, 3, 1, 1, Alignment::Maximum, Alignment::Middle));
  hardcoreBox->setFontSize(16);
  hardcoreBox->setChecked(properties->yAxisLog);
  hardcoreBox->setCallback([this](bool value) {
    properties->reactivityHardcore = value;
    hardcoreMode(value);
  });

  CustomLabel *timeAdjLabel =
      graph_controls->add<CustomLabel>("Edit display range", "sans-bold");
  timeAdjLabel->setFontSize(25);
  timeAdjLabel->setPadding(0, 15);
  graphControlsLayout->setAnchor(
      timeAdjLabel, RelativeGridLayout::makeAnchor(
                        0, 3, 1, 1, Alignment::Minimum, Alignment::Minimum));

  Widget *timeLockPanel = graph_controls->add<Widget>();
  graphControlsLayout->setAnchor(
      timeLockPanel, RelativeGridLayout::makeAnchor(
                         0, 4, 1, 1, Alignment::Minimum, Alignment::Fill));
  RelativeGridLayout *tlLayout = new RelativeGridLayout();
  tlLayout->appendCol(
      RelativeGridLayout::Size(120.f, RelativeGridLayout::SizeType::Fixed));
  tlLayout->appendCol(1.f);
  tlLayout->appendRow(1.f);
  timeLockPanel->setLayout(tlLayout);
  CustomLabel *tmp = timeLockPanel->add<CustomLabel>("Lock view:", "sans-bold");
  tlLayout->setAnchor(tmp, RelativeGridLayout::makeAnchor(0, 0));
  tmp->setPadding(0, 25);
  tmp->setPadding(2, 10);
  timeLockedBox = timeLockPanel->add<SliderCheckBox>();
  tlLayout->setAnchor(timeLockedBox, RelativeGridLayout::makeAnchor(1, 0));
  timeLockedBox->setFontSize(16);
  timeLockedBox->setChecked(false);
  timeLockedBox->setCallback([this](bool value) {
    if (value) {
      this->viewingIntervalChanged(true);
    } else {
      timeAtLastChange = this->reactor->getCurrentTime();
    }
  });

  displayTimeSlider = new IntervalSlider(graph_controls);
  RelativeGridLayout::Anchor acr = RelativeGridLayout::makeAnchor(
      0, 5, 2, 1, Alignment::Fill, Alignment::Minimum);
  acr.padding = Vector4i(20, 10, 20, 0);
  graphControlsLayout->setAnchor(displayTimeSlider, acr);
  displayTimeSlider->setHighlightColor(coolBlue);
  displayTimeSlider->setSteps((unsigned int)DELETE_OLD_DATA_TIME_DEFAULT *
                              1000U);
  displayTimeSlider->setEnabled(true);
  displayTimeSlider->setFixedHeight(25);
  for (int i = 0; i < 2; i++)
    displayTimeSlider->setCallback(i, [this, i](float /*change*/) {
      this->viewingIntervalChanged(i == 0);
      if (i == 0 && !timeLockedBox->checked()) {
        timeAtLastChange = this->reactor->getCurrentTime();
      }
    });
  Button *displayResetBtn = new Button(graph_controls, "Reset view to newest");
  RelativeGridLayout::Anchor acr2 = RelativeGridLayout::makeAnchor(
      1, 3, 1, 1, Alignment::Maximum, Alignment::Middle);
  acr2.padding = Vector4i(0, 0, 10, 0);
  graphControlsLayout->setAnchor(displayResetBtn, acr2);
  displayResetBtn->setCallback([this]() {  // reset the view to default
    this->viewStart = -1.;
    timeLockedBox->setChecked(false);
    timeLockedBox->callback()(false);
  });
}
