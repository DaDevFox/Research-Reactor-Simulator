#include "../include/SimulatorGUI.h"

void SimulatorGUI::createPulseTab() {
  CustomWidget *pulse_tab = tabControl->createTab("Pulse");
  pulse_tab->setId("pulse tab");
  RelativeGridLayout *rel = new RelativeGridLayout();
  rel->appendCol(RelativeGridLayout::Size(
      15.f, RelativeGridLayout::SizeType::Fixed));  // 0 left border
  rel->appendCol(2.f);                              // 1 pulse graph
  rel->appendCol(1.f);                              // 2 info panel
  rel->appendCol(RelativeGridLayout::Size(
      15.f, RelativeGridLayout::SizeType::Fixed));  // 3 right border
  rel->appendRow(RelativeGridLayout::Size(
      15.f, RelativeGridLayout::SizeType::Fixed));  // 0 top border
  rel->appendRow(1.f);                              // 1 content
  pulse_tab->setLayout(rel);

  pulseGraph = pulse_tab->add<CustomGraph>(4, "Last pulse");
  rel->setAnchor(pulseGraph, RelativeGridLayout::makeAnchor(0, 0, 2, 2));
  initializePulseGraph();

  standInCover = pulse_tab->add<CustomLabel>(
      "Perform a pulse experiment to view data", "sans-bold", 35);
  rel->setAnchor(standInCover, RelativeGridLayout::makeAnchor(0, 0, 2, 2));
  standInCover->setTextAlignment(CustomLabel::TextAlign::HORIZONTAL_CENTER |
                                 CustomLabel::TextAlign::VERTICAL_CENTER);
  standInCover->setDrawBackground(true);
  standInCover->setBackgroundColor(Color(60, 255));
  standInCover->setColor(Color(255, 255));
  standInCover->setVisible(true);

  CustomWidget *dataSheet = pulse_tab->add<CustomWidget>();
  rel->setAnchor(dataSheet, RelativeGridLayout::makeAnchor(2, 1));

  RelativeGridLayout *rel2 = new RelativeGridLayout();
  rel2->appendCol(RelativeGridLayout::Size(
      15.f, RelativeGridLayout::SizeType::Fixed));  // 0 left border
  rel2->appendCol(RelativeGridLayout::Size(
      5.f, RelativeGridLayout::SizeType::Fixed));  // 1 color border
  rel2->appendCol(1.f);                            // 2 content
  rel2->appendCol(RelativeGridLayout::Size(
      15.f, RelativeGridLayout::SizeType::Fixed));  // 3 seperation border
  rel2->appendCol(RelativeGridLayout::Size(
      5.f, RelativeGridLayout::SizeType::Fixed));  // 4 color border
  rel2->appendCol(1.f);                            // 5 content
  rel2->appendRow(RelativeGridLayout::Size(
      100.f, RelativeGridLayout::SizeType::Fixed));  // 0 content
  rel2->appendRow(RelativeGridLayout::Size(
      15.f, RelativeGridLayout::SizeType::Fixed));  // 1 seperation border
  rel2->appendRow(RelativeGridLayout::Size(
      100.f, RelativeGridLayout::SizeType::Fixed));  // 2 content
  rel2->appendRow(RelativeGridLayout::Size(
      15.f, RelativeGridLayout::SizeType::Fixed));  // 3 bottom border
  rel2->appendRow(RelativeGridLayout::Size(
      20.f, RelativeGridLayout::SizeType::Fixed));  // 4 time interval
  rel2->appendRow(RelativeGridLayout::Size(
      20.f, RelativeGridLayout::SizeType::Fixed));  // 5 time labels
  rel2->appendRow(1.f);                             // 6 display label
  dataSheet->setLayout(rel2);

  Color colors[4] = {Color(255, 0, 0, 255), Color(0, 255),
                     Color(0, 255, 0, 255), Color(255, 0, 255, 255)};
  std::string text[4] = {"Peak power", "FWHM", "Peak fuel temp.",
                         "Pulse energy"};
  for (int i = 0; i < 4; i++) {
    CustomWidget *marker = dataSheet->add<CustomWidget>();
    rel2->setAnchor(
        marker, RelativeGridLayout::makeAnchor((i % 2) * 3 + 1, (i / 2) * 2));
    marker->setDrawBackground(true);
    marker->setBackgroundColor(colors[i]);

    CustomLabel *baseLabel = dataSheet->add<CustomLabel>(text[i], "sans-bold");
    rel2->setAnchor(baseLabel, RelativeGridLayout::makeAnchor((i % 2) * 3 + 2,
                                                              (i / 2) * 2));
    baseLabel->setPadding(0, 7.f);
    baseLabel->setPadding(1, 5.f);
    baseLabel->setBackgroundColor(Color(32, 255));
    baseLabel->setDrawBackground(true);
    baseLabel->setTextAlignment(CustomLabel::TextAlign::TOP |
                                CustomLabel::TextAlign::LEFT);
    baseLabel->setFontSize(25.f);
    baseLabel->setColor(Color(170, 255));

    RelativeGridLayout *rel3 = new RelativeGridLayout();
    rel3->appendCol(1.f);
    rel3->appendRow(1.f);
    baseLabel->setLayout(rel3);

    pulseLabels[i] = baseLabel->add<CustomLabel>("", "sans-bold");
    pulseLabels[i]->setColor(Color(255, 255));
    pulseLabels[i]->setPadding(0, 7.f);
    pulseLabels[i]->setFontSize(44.f);
    pulseLabels[i]->setTextAlignment(CustomLabel::TextAlign::BOTTOM |
                                     CustomLabel::TextAlign::LEFT);
    rel3->setAnchor(pulseLabels[i], RelativeGridLayout::makeAnchor(0, 0));
  }

  pulseTimer = dataSheet->add<IntervalSlider>();
  rel2->setAnchor(pulseTimer, RelativeGridLayout::makeAnchor(1, 4, 5, 1));
  pulseTimer->setEnabled(false);
  pulseTimer->setHighlightColor(coolBlue);
  pulseTimer->setSteps(50U);
  for (int i = 0; i < 2; i++)
    pulseTimer->setCallback(i,
                            [this](float /*change*/) { updatePulseTrack(); });
  CustomLabel *infoLbl;
  for (int i = 0; i < 2; i++) {
    infoLbl = dataSheet->add<CustomLabel>(i ? "5s later" : "pulse start",
                                          "sans-bold");
    infoLbl->setColor(Color(255, 255));
    infoLbl->setFontSize(17.f);
    infoLbl->setPadding(1, 4);
    rel2->setAnchor(infoLbl,
                    RelativeGridLayout::makeAnchor(
                        1, 5, 5, 1, i ? Alignment::Maximum : Alignment::Minimum,
                        Alignment::Minimum));
  }

  CustomWidget *displayPanel = dataSheet->add<CustomWidget>();
  rel2->setAnchor(displayPanel,
                  RelativeGridLayout::makeAnchor(1, 6, 5, 1, Alignment::Middle,
                                                 Alignment::Middle));
  displayPanel->setLayout(
      new BoxLayout(Orientation::Horizontal, Alignment::Fill));

  CustomLabel *temp;
  temp = displayPanel->add<CustomLabel>("From", "sans-bold", 25);
  temp->setPadding(2, 10);
  pulseDisplayLabels[0] = displayPanel->add<CustomLabel>("0", "sans", 30);
  pulseDisplayLabels[1] = displayPanel->add<CustomLabel>("00", "sans", 20);
  temp = displayPanel->add<CustomLabel>("to", "sans-bold", 25);
  temp->setPadding(0, 10);
  temp->setPadding(2, 10);
  pulseDisplayLabels[2] = displayPanel->add<CustomLabel>("5", "sans", 30);
  pulseDisplayLabels[3] = displayPanel->add<CustomLabel>("00", "sans", 20);

  for (int i = 0; i < 4; i++) {
    pulseDisplayLabels[i]->setColor(Color(255, 255));
    pulseDisplayLabels[i]->setTextAlignment(
        (i % 2) ? (CustomLabel::TextAlign::TOP | CustomLabel::TextAlign::LEFT)
                : (CustomLabel::TextAlign::VERTICAL_CENTER |
                   CustomLabel::TextAlign::HORIZONTAL_CENTER));
  }
}
