#include "../include/SimulatorGUI.h"

void SimulatorGUI::createMainTab()
{
		Widget *mainTabBase = tabControl->createTab("Main controls");
		mainTabBase->setId("main tab");
		RelativeGridLayout *rel3 = new RelativeGridLayout();
		rel3->appendCol(RelativeGridLayout::Size(250.f, RelativeGridLayout::SizeType::Fixed)); // 0 first column
		rel3->appendCol(RelativeGridLayout::Size(250.f, RelativeGridLayout::SizeType::Fixed)); // 1 second column
		rel3->appendCol(RelativeGridLayout::Size(80.f, RelativeGridLayout::SizeType::Fixed));  // 2 reactivitycolumn
		rel3->appendCol(RelativeGridLayout::Size(7.f, RelativeGridLayout::SizeType::Fixed));   // 3 padding
		rel3->appendCol(RelativeGridLayout::Size(190.f, RelativeGridLayout::SizeType::Fixed)); // 4 control rod display
		rel3->appendCol(RelativeGridLayout::Size(7.f, RelativeGridLayout::SizeType::Fixed));   // 5 padding
		rel3->appendCol(1.f);																   // 6 main content
		rel3->appendCol(RelativeGridLayout::Size(100.f, RelativeGridLayout::SizeType::Fixed)); // 7 alarms
		rel3->appendRow(1.f);
		mainTabBase->setLayout(rel3);

		// Create the laft hand side of the main screen
		createDataDisplays(mainTabBase, rel3);

		// Create a panel for the buttons of the main window
		Widget *main_right = mainTabBase->add<Widget>();
		rel3->setAnchor(main_right, RelativeGridLayout::makeAnchor(6, 0));
		main_right->setLayout(new BoxLayout(Orientation::Vertical, Alignment::Fill, 10, 10));

		// Create a panel for reactivity controls
		/*Widget* reactivityPanel = main_right->add<Widget>();
		reactivityPanel->setLayout(new BoxLayout(Orientation::Horizontal, Alignment::Middle, 0, 20));*/

		Widget *controlButtonsPanel = main_right->add<Widget>();
		controlButtonsPanel->setLayout(new BoxLayout(Orientation::Horizontal, Alignment::Middle, 0, 20));
		Button *scram;
		scram = controlButtonsPanel->add<Button>("SCRAM", ENTYPO_ICON_NEW);
		scram->setBackgroundColor(Color(255, 0, 0, 100));
		scram->setFixedSize(Vector2i(150, 45));
		scram->setTextColor(Color(255, 255));
		scram->setCallback([this]
						   { reactor->scram(Simulator::ScramSignals::User); });
		Button *fire;
		fire = controlButtonsPanel->add<Button>("FIRE");
		fire->setBackgroundColor(Color(255, 0, 0, 100));
		fire->setFixedSize(Vector2i(150, 45));
		fire->setTextColor(Color(255, 255));
		fire->setCallback([this]
						  { reactor->beginPulse(); });

		// Rod mode
		Widget *panel = main_right->add<Widget>();
		panel->setLayout(panelsLayout);
		rodMode = panel->add<ComboBox>(modes);
		rodMode->setFixedWidth(150);
		rodMode->setCallback([this](int change)
							 {
				switch (change)
				{
				case 0:
					reactor->regulatingRod()->setOperationMode(ControlRod::OperationModes::Manual);
					break;
					//case 1:
					//	reactor->regulatingRod()->setOperationMode(ControlRod::OperationModes::Simulation);
					//	reactor->regulatingRod()->setSimulationMode(SimulationModes::SquareWaveMode);
					//	break;
					//case 2:
					//	reactor->regulatingRod()->setOperationMode(ControlRod::OperationModes::Simulation);
					//	reactor->regulatingRod()->setSimulationMode(SimulationModes::SineMode);
					//	break;
					//case 3:
					//	reactor->regulatingRod()->setOperationMode(ControlRod::OperationModes::Simulation);
					//	reactor->regulatingRod()->setSimulationMode(SimulationModes::SawToothMode);
					//	break;
				case 4:
					reactor->regulatingRod()->setOperationMode(ControlRod::OperationModes::Automatic);
					reactor->setPowerHold(reactor->getCurrentPower());
					break;
					//case 5:
					//	reactor->regulatingRod()->setOperationMode(ControlRod::OperationModes::Pulse);
					//	break;
				} });
		rodMode->setSelectedIndex(0);

		Button *unscram;
		unscram = main_right->add<Button>("RESET SCRAM", ENTYPO_ICON_CCW);
		unscram->setTextColor(Color(255, 255));
		unscram->setFixedWidth(150);
		unscram->setCallback([this]
							 { reactor->scram(Simulator::ScramSignals::None); });

		// Create reactivity display
		periodDisplay = mainTabBase->add<PeriodDisplay>();
		rel3->setAnchor(periodDisplay, RelativeGridLayout::makeAnchor(2, 0));

		// Create rod display
		Widget *controlRodBase = mainTabBase->add<Widget>();
		rel3->setAnchor(controlRodBase, RelativeGridLayout::makeAnchor(4, 0));
		RelativeGridLayout *rodLayout = new RelativeGridLayout();
		rodLayout->appendRow(RelativeGridLayout::Size(30.f, RelativeGridLayout::SizeType::Fixed));
		rodLayout->appendRow(1.f);
		rodLayout->appendRow(RelativeGridLayout::Size(30.f, RelativeGridLayout::SizeType::Fixed));
		for (int i = 0; i < 3; i++)
			rodLayout->appendCol(1.f);
		controlRodBase->setLayout(rodLayout);

		CustomLabel *temp;
		CustomWidget *backg = controlRodBase->add<CustomWidget>();
		backg->setDrawBackground(true);
		backg->setBackgroundColor(Color(.15f, 1.f));
		rodLayout->setAnchor(backg, RelativeGridLayout::makeAnchor(0, 0, 3));
		rodDisplay = controlRodBase->add<ControlRodDisplay>();
		for (int i = 0; i < 3; i++)
		{
			temp = controlRodBase->add<CustomLabel>(i ? ((i == 1) ? "R" : "C") : "S", "sans-bold");
			temp->setFontSize(25.f);
			temp->setColor(Color(255, 255));
			if (i != 1)
				temp->setPadding(2 - i, ControlRodDisplay::getRodSpacing() * 2 / 3);
			temp->setTextAlignment(CustomLabel::TextAlign::HORIZONTAL_CENTER | CustomLabel::TextAlign::VERTICAL_CENTER);
			rodLayout->setAnchor(temp, RelativeGridLayout::makeAnchor(i, 0));
			rodDisplay->setRod(i, reactor->rods[i]->getRodSteps(), reactor->rods[i]->getActualPosition(), reactor->rods[i]->getExactPosition(), reactor->rods[i]->isEnabled());
		}
		rodLayout->setAnchor(rodDisplay, RelativeGridLayout::makeAnchor(0, 1, 3));

		// Create a panel for alarms
		CustomWidget *alarmPanel = mainTabBase->add<CustomWidget>();
		alarmPanel->setDrawBackground(true);
		alarmPanel->setBackgroundColor(Color(40, 255));
		rel3->setAnchor(alarmPanel, RelativeGridLayout::makeAnchor(7, 0));
		RelativeGridLayout *alarmLayout = new RelativeGridLayout();
		alarmLayout->appendCol(1.f);
		alarmLayout->appendRow(RelativeGridLayout::Size(35.f, RelativeGridLayout::SizeType::Fixed));
		alarmPanel->setLayout(alarmLayout);

		CustomLabel *alarmHeader = alarmPanel->add<CustomLabel>("SCRAMs");
		alarmHeader->setFontSize(24.f);
		alarmHeader->setTextAlignment(CustomLabel::TextAlign::HORIZONTAL_CENTER | CustomLabel::TextAlign::VERTICAL_CENTER);
		alarmLayout->setAnchor(alarmHeader, RelativeGridLayout::makeAnchor(0, 0));
		// Alarm labels
		CustomLabel *alarmLabels[6];
		std::string text[6] = {"MAN", "PER", "FTEMP", "WTEMP", "POW", "WLEVEL"};
		RelativeGridLayout::Anchor a;
		for (int i = 0; i < 6; i++)
		{
			if (i < 5)
			{
				alarmLayout->appendRow(1.f);
				alarmLabels[i] = alarmPanel->add<CustomLabel>(text[i], "sans-bold");
				a = RelativeGridLayout::makeAnchor(0, i + 1);
				a.padding = Vector4i(14, 7, 14, 7);
				alarmLayout->setAnchor(alarmLabels[i], a);
			}
			else
			{
				alarmLabels[i] = new CustomLabel(nullptr, text[i], "sans-bold");
			}
			alarmLabels[i]->setColor(Color(255, 255));
			alarmLabels[i]->setTextAlignment(CustomLabel::TextAlign::HORIZONTAL_CENTER | CustomLabel::TextAlign::VERTICAL_CENTER);
			alarmLabels[i]->setFixedSize(Vector2i(60, 40));
			alarmLabels[i]->setGlow(false);
			alarmLabels[i]->setBackgroundColor(Color(100, 255));
			alarmLabels[i]->setDrawBackground(true);
			alarmLabels[i]->setGlowAmount(20.f);
			alarmLabels[i]->setGlowColor(Color(255, 0, 0, 125));
		}
		userScram = alarmLabels[0];
		periodScram = alarmLabels[1];
		fuelTemperatureScram = alarmLabels[2];
		waterTemperatureScram = alarmLabels[3];
		powerScram = alarmLabels[4];
		waterLevelScram = alarmLabels[5];

		// FOR NOW
		waterLevelScram->setVisible(false);

		// Controls
		{
			for (int i = 0; i < NUMBER_OF_CONTROL_RODS; i++)
			{
				rodBox[i] = makeSettingLabel<IntBox<int>>(main_right, reactor->rods[i]->getRodName() + " rod magnet position: ", 200);
				rodBox[i]->setFixedSize(Vector2i(100, 20));
				rodBox[i]->setUnits("steps");
				rodBox[i]->setValueIncrement(1);
				rodBox[i]->setDefaultValue("0");
				rodBox[i]->setFontSize(16);
				rodBox[i]->setFormat("[0-9]+");
				rodBox[i]->setMinMaxValues(0, (int)*reactor->rods[i]->getRodSteps());
				rodBox[i]->setSpinnable(true);
				rodBox[i]->setCallback([this, i](const int change)
									   {
						try
						{
							this->reactor->rods[i]->commandMove((size_t)change);
						}
						catch (exception e)
						{
							return false;
						}
						return true; });
			}
		}

		// Checkboxes
		{
			Widget *checkBoxCooling = main_right->add<Widget>();
			checkBoxCooling->setLayout(panelsLayout);
			checkBoxCooling->add<CustomLabel>("Cooling :", "sans-bold");
			cooling = checkBoxCooling->add<SliderCheckBox>();
			cooling->setFontSize(16);
			cooling->setChecked(false);
			cooling->setCallback([this](bool value)
								 {
					properties->waterCooling = value;
					reactor->setWaterCooling(value);
					if (value)
					{
						//cooling->setCaption("enabled");
					}
					else
					{
						//cooling->setCaption("disabled");
					} });

			Widget *checkBoxPanelNeutronSource = main_right->add<Widget>();
			checkBoxPanelNeutronSource->setLayout(panelsLayout);
			checkBoxPanelNeutronSource->add<CustomLabel>("Neutron source :", "sans-bold");
			neutronSourceCB = checkBoxPanelNeutronSource->add<SliderCheckBox>();
			neutronSourceCB->setFontSize(16);
			neutronSourceCB->setChecked(reactor->getNeutronSourceInserted());
			neutronSourceCB->setCallback([this](bool value)
										 {
					reactor->setNeutronSourceInserted(value);
					properties->neutronSourceInserted = value; });
		}
	}

void SimulatorGUI::createDelayedGroupsTab()
{
		Widget *delayed_tab = tabControl->createTab("Delayed neutrons");
		delayed_tab->setId("delayed tab");
		RelativeGridLayout *delayedLayout = new RelativeGridLayout();
		delayedLayout->appendCol(1.f); // graph area
		delayedLayout->appendRow(1.f); // graph area
		delayed_tab->setLayout(delayedLayout);

		CustomWidget *topHost = delayed_tab->add<CustomWidget>();
		topHost->setBackgroundColor(Color(240, 255));
		topHost->setDrawBackground(true);
		delayedLayout->setAnchor(topHost, RelativeGridLayout::makeAnchor(0, 0));
		RelativeGridLayout *topLayout = new RelativeGridLayout();
		topLayout->appendCol(1.f);																	// graph
		topLayout->appendCol(RelativeGridLayout::Size(158.f, RelativeGridLayout::SizeType::Fixed)); // key
		topLayout->appendRow(1.f);
		topHost->setLayout(topLayout);

		Color dataColors[7] = {Color(255, 255), Color(34, 116, 165, 255), Color(247, 92, 3, 255), Color(241, 196, 15, 255), Color(0, 204, 102, 255), Color(240, 58, 71, 255), Color(153, 0, 153, 255)};

		CustomWidget *keyPanel = topHost->add<CustomWidget>();
		topLayout->setAnchor(keyPanel, RelativeGridLayout::makeAnchor(1, 0, 1, 1, Alignment::Minimum, Alignment::Middle));
		keyPanel->setDrawBackground(true);
		keyPanel->setBackgroundColor(Color(65, 255));
		keyPanel->setBorder(Border::BOTTOM | Border::LEFT | Border::TOP | Border::RIGHT);
		keyPanel->setBorderColor(coolBlue);
		keyPanel->setBorderWidth(1.f);
		RelativeGridLayout *rel = new RelativeGridLayout();
		rel->appendCol(RelativeGridLayout::Size(50.f, RelativeGridLayout::SizeType::Fixed)); // colors
		rel->appendCol(RelativeGridLayout::Size(1.f, RelativeGridLayout::SizeType::Fixed));	 // border
		rel->appendCol(RelativeGridLayout::Size(10.f, RelativeGridLayout::SizeType::Fixed)); // local padding
		rel->appendCol(RelativeGridLayout::Size(70.f, RelativeGridLayout::SizeType::Fixed)); // text
		for (int i = 0; i < 7; i++)
			rel->appendRow(RelativeGridLayout::Size(35.f, RelativeGridLayout::SizeType::Fixed)); // data rows
		rel->appendRow(RelativeGridLayout::Size(5.f, RelativeGridLayout::SizeType::Fixed));		 // padding
		keyPanel->setLayout(rel);
		Label *tempLabelKey = keyPanel->add<Label>("Color", "sans-bold");
		rel->setAnchor(tempLabelKey, RelativeGridLayout::makeAnchor(0, 0, 1, 1, Alignment::Middle, Alignment::Middle));
		tempLabelKey = keyPanel->add<Label>("Group No.", "sans-bold");
		rel->setAnchor(tempLabelKey, RelativeGridLayout::makeAnchor(3, 0, 1, 1, Alignment::Minimum, Alignment::Middle));
		CustomWidget *temp = keyPanel->add<CustomWidget>();
		rel->setAnchor(temp, RelativeGridLayout::makeAnchor(1, 0));
		temp->setDrawBackground(true);
		temp->setBackgroundColor(coolBlue);
		for (int i = 1; i < 7; i++)
		{
			temp = keyPanel->add<CustomWidget>();
			rel->setAnchor(temp, RelativeGridLayout::makeAnchor(0, i, 1, 1, Alignment::Middle, Alignment::Middle));
			temp->setDrawBackground(true);
			temp->setBackgroundColor(dataColors[i]);
			temp->setFixedSize(Vector2i(25, 25));
			Label *tempLabel = keyPanel->add<Label>(to_string(i), "sans-bold");
			rel->setAnchor(tempLabel, RelativeGridLayout::makeAnchor(3, i, 1, 1, Alignment::Minimum, Alignment::Middle));
		}

		delayedGroupsGraph = topHost->add<CustomGraph>(6, "Delayed neutron groups");
		delayedGroupsGraph->setPadding(90.f, 20.f, 60.f, 50.f);
		delayedGroupsGraph->setTextColor(Color(0, 255));
		topLayout->setAnchor(delayedGroupsGraph, RelativeGridLayout::makeAnchor(0, 0));
		// double betaLambda = reactor->prompt_lifetime;
		double betaLambda = 0;
		double sumBeta = 0, sumLambdaC = 0;
		/*
		The delayed group fractions for display are normalized to 100%, where 100% is a concentration
		of each group during stationary conditions at current power.
		*/
		for (int i = 0; i < 6; i++)
		{
			// betaLambda += reactor->beta_neutrons[i] / reactor->delayed_decay_time[i];
			sumBeta += reactor->beta_neutrons[i];
		}
		for (int i = 0; i < 6; i++)
		{
			delayedGroups[i] = delayedGroupsGraph->addPlot(reactor->getDataLength(), true);
			delayedGroups[i]->setXdata(reactor->time_);
			delayedGroups[i]->setYdata(reactor->state_vector_[i + 1]);
			delayedGroups[i]->setColor(dataColors[i + 1]);
			delayedGroups[i]->setDrawMode(DrawMode::Smart);
			delayedGroups[i]->setPointerColor(dataColors[i + 1]);
			delayedGroups[i]->setAxisPosition((i < 3) ? GraphElement::AxisLocation::Right : GraphElement::AxisLocation::Left);
			delayedGroups[i]->setPixelDrawRatio(0.3f); // skip 70% of pixels when drawing
			delayedGroups[i]->setValueComputing([this, sumBeta, sumLambdaC, i](double *val, const size_t index)
												{
					//*val = *val * sumLambdaC * reactor->delayed_decay_time[i] / (reactor->state_vector_[7][index] * reactor->beta_neutrons[i]);
					double sumLambdaC = 0;
					for (int j = 0; j < 6; j++)
						sumLambdaC += reactor->delayed_decay_time[j] * reactor->state_vector_[j + 1][index];
					*val = *val * sumBeta * reactor->delayed_decay_time[i] / (sumLambdaC * reactor->beta_neutrons[i]); });
			if (!i)
			{
				delayedGroups[i]->setAxisShown(true);
				delayedGroups[i]->setAxisPosition(GraphElement::AxisLocation::Left);
				delayedGroups[i]->setHorizontalAxisShown(true);
				delayedGroups[i]->setHorizontalMainLineShown(true);
				delayedGroups[i]->setTextShown(true);
				delayedGroups[i]->setName("Deviation");
				delayedGroups[i]->setUnits("%");
				delayedGroups[i]->setMajorTickNumber(2);
				delayedGroups[i]->setMinorTickNumber(3);
				delayedGroups[i]->setLimitOverride(1, "now");
				delayedGroups[i]->setLimitOverride(0, "30 seconds ago");
				delayedGroups[i]->setLimitMultiplier(100.);
				delayedGroups[i]->setHorizontalName("Time");
				delayedGroups[i]->setHorizontalUnits("s");
				delayedGroups[i]->setHorizontalTextOffset(20.f);
			}
		};
	}

void SimulatorGUI::updateNeutronSourceTab()
{
		int v = (int)reactor->getNeutronSourceMode() - 1;
		bool tempB;
		for (int i = 0; i < 3; i++)
		{
			tempB = (v == i);
			neutronSourcePeriodBoxes[i]->parent()->setVisible(tempB);
			neutronSourcePeriodBoxes[i]->setEditable(tempB);
			neutronSourceAmplitudeBoxes[i]->parent()->setVisible(tempB);
			neutronSourceAmplitudeBoxes[i]->setEditable(tempB);
		}
		tempB = (v == 0);
		for (int i = 0; i < 4; i++)
		{
			neutronSourceSQWBoxes[i]->parent()->setVisible(tempB);
			neutronSourceSQWBoxes[i]->setEditable(tempB);
		}
		tempB = (v == 1);
		neutronSourceSINEModeBox->parent()->setVisible(tempB);
		neutronSourceSINEModeBox->setEnabled(tempB);
		tempB = (v == 2);
		for (int i = 0; i < 6; i++)
		{
			neutronSourceSAWBoxes[i]->parent()->setVisible(tempB);
			neutronSourceSAWBoxes[i]->setEditable(tempB);
		}
		for (int i = (int)sourceGraph->actualGraphNumber() - 1; i >= 0; i--)
		{
			sourceGraph->removeGraphElement(i);
		}
		PeriodicalMode *ns_mode = reactor->getSourceModeClass(reactor->getNeutronSourceMode());
		size_t dataP = ns_mode->num_points();
		neutronSourcePlot = sourceGraph->addPlot(dataP);
		neutronSourcePlot->setPlotRange(0, dataP - 1);
		neutronSourcePlot->setLimits(0., ns_mode->getPeriod(), std::min(-1.5 * ns_mode->getAmplitude(), -1.), std::max(1.5 * ns_mode->getAmplitude(), 1.));
		neutronSourcePlot->setMainTickFontSize(18.f);
		neutronSourcePlot->setMajorTickFontSize(16.f);
		neutronSourcePlot->setNameFontSize(24.f);
		neutronSourcePlot->setPointerShown(false);
		neutronSourcePlot->setColor(coolBlue);
		neutronSourcePlot->setDrawMode(DrawMode::Default);
		neutronSourcePlot->setTextColor(Color(250, 255));
		neutronSourcePlot->setAxisColor(Color(250, 255));
		neutronSourcePlot->setTextShown(true);
		neutronSourcePlot->setAxisShown(true);
		neutronSourcePlot->setUnits("n/s");
		neutronSourcePlot->setName("Delta source activity");
		neutronSourcePlot->setTextOffset(40.f);
		neutronSourcePlot->setMainLineShown(true);
		neutronSourcePlot->setMajorTickNumber(3);
		neutronSourcePlot->setMinorTickNumber(1);
		neutronSourcePlot->setHorizontalAxisShown(true);
		neutronSourcePlot->setHorizontalUnits("s");
		neutronSourcePlot->setHorizontalName("Time");
		neutronSourcePlot->setHorizontalMainLineShown(true);
		neutronSourcePlot->setHorizontalMajorTickNumber(3);
		neutronSourcePlot->setHorizontalMinorTickNumber(1);
		if (v >= 0)
		{
			neutronSourceTracker = sourceGraph->addPlot(2);
			neutronSourceTracker->setPlotRange(0, 1);
			neutronSourceTracker->setDrawMode(DrawMode::Default);
			neutronSourceTracker->setAxisShown(false);
			neutronSourceTracker->setColor(Color(1.f, 0.f, 0.f, 1.f));
			neutronSourceTracker->setHorizontalAxisShown(false);
			neutronSourceTracker->setTextShown(false);
			neutronSourceTracker->setPointerShown(false);
			neutronSourceTracker->setXdata(ns_mode->getTrackerArray());
			neutronSourceTracker->setYdata(trackerY);
			neutronSourceTracker->setLimits(0., ns_mode->getPeriod(), 0., 1.);
		}
		double *xAxis = new double[dataP];
		double *yAxis = new double[dataP];
		ns_mode->fillXYaxis(xAxis, yAxis);

		neutronSourcePlot->setXdata(xAxis);
		neutronSourcePlot->setYdata(yAxis);
		shouldUpdateNeutronSource = true;
	}

