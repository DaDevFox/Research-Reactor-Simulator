#include "../include/SimulatorGUI.h"

const std::string SimulatorGUI::version()
{
		std::string ver = to_string(VERSION_MAJOR) + "." + to_string(VERSION_MINOR);
#if VERSION_REVISION
		ver += "." + to_string(VERSION_REVISION);
		return VERSION_BUILD ? (ver += "." + to_string(VERSION_BUILD)) : ver;
#else
		return VERSION_BUILD ? (ver + ".0." + to_string(VERSION_BUILD)) : ver;
#endif
	}

void SimulatorGUI::setSimulationTime(size_t time)
{
		selectedTime = time;
		reactor->setSpeedFactor(simulationTimes[time]);
		updateSimulationIcon();
	}

void SimulatorGUI::playPauseSimulation(bool play)
{
		if (play)
		{
			setSimulationTime(selectedTime);
			playPause->setIcon(ENTYPO_ICON_PLAY);
		}
		else
		{
			reactor->setSpeedFactor(0.);
			playPause->setIcon(ENTYPO_ICON_PAUS);
		}
		playPause->setPushed(!play);
		updateSimulationIcon();
	}

void SimulatorGUI::updateSimulationIcon()
{
		double sf = reactor->getSpeedFactor();
		if (sf == 0.)
		{
			simFactorLabel->setCaption("paused");
			simStatusLabel->setCaption(utf8(ENTYPO_ICON_PAUS).data());
		}
		else if (sf == 1.)
		{
			simFactorLabel->setCaption("real-time");
			simStatusLabel->setCaption(utf8(ENTYPO_ICON_PLAY).data());
		}
		else if (sf > 1.)
		{
			simFactorLabel->setCaption(formatDecimalsDoubleWithTrailing(sf, 2, false) + "x");
			simStatusLabel->setCaption(utf8(ENTYPO_ICON_FF).data());
		}
		else
		{
			simFactorLabel->setCaption(formatDecimalsDoubleWithTrailing(sf, 2, false) + "x");
			simStatusLabel->setCaption(utf8(ENTYPO_ICON_FB).data());
		}
	}

void SimulatorGUI::initializeSimulator()
{
		// Create the Simulator object, set initial properties
		reactor = new Simulator(properties);
		reactor->setDebugMode(debugMode);
		reactor->setScramCallback([this](int signal)
								  {
				if (signal > 0) LEDstatus += (uint16_t)1 << 13;
				std::string reason = "";
				if ((signal & Simulator::ScramSignals::Period) != 0)
				{
					reason = "Period too low | " + std::to_string(*reactor->getReactorPeriod()) + " s | asymptotic | " + std::to_string(*reactor->getReactorAsymPeriod()) + " s";
					periodScram->setGlow(true);
					periodScram->setBackgroundColor(uiStyleConfig.scramAlertColor);
					LEDstatus |= SCRAM_PER;
				}
				if ((signal & Simulator::ScramSignals::FuelTemperature) != 0)
				{
					reason = "Fuel temperature too high | " + std::to_string(reactor->getCurrentTemperature()) + " C";
					fuelTemperatureScram->setGlow(true);
					fuelTemperatureScram->setBackgroundColor(uiStyleConfig.scramAlertColor);
					LEDstatus |= SCRAM_FT;
				}
				if ((signal & Simulator::ScramSignals::WaterTemperature) != 0)
				{
					reason = "Water temperature too high | " + std::to_string(reactor->waterTemperature) + " C";
					waterTemperatureScram->setGlow(true);
					waterTemperatureScram->setBackgroundColor(uiStyleConfig.scramAlertColor);
					LEDstatus |= SCRAM_WT;
				}
				if ((signal & Simulator::ScramSignals::WaterLevel) != 0)
				{
					reason = "Water level too low | " + std::to_string(*reactor->getWaterLevel()) + " m";
					waterLevelScram->setGlow(true);
					waterLevelScram->setBackgroundColor(uiStyleConfig.scramAlertColor);
					// LEDstatus |= ALARM3;
					// NOT SUPPORTED BY THE BOX
				}
				if ((signal & Simulator::ScramSignals::Power) != 0)
				{
					reason = "Power too high | " + std::to_string(reactor->getCurrentPower()) + " W";
					powerScram->setGlow(true);
					powerScram->setBackgroundColor(uiStyleConfig.scramAlertColor);
					LEDstatus |= SCRAM_POW;
				}
				if ((signal & Simulator::ScramSignals::User) != 0)
				{
					reason = "Operator";
					userScram->setGlow(true);
					userScram->setBackgroundColor(uiStyleConfig.scramAlertColor);
					LEDstatus |= SCRAM_MAN;
				}
				cout << "The reactor has SCRAMed!" << endl;
				cout << "=======REASON=======" << endl;
				cout << reason << endl;
				cout << "====================" << endl; });
		reactor->setResetScramCallback([this]
									   {
				LEDstatus &= RESET_ALARM_KEY;
				SimulatorGuiUiHelpers::resetScramIndicators(
					uiStyleConfig,
					userScram,
					powerScram,
					periodScram,
					waterTemperatureScram,
					waterLevelScram,
					fuelTemperatureScram); });
		reactor->setPulseCallback([this](Simulator::PulseData data)
								  {
				// Format pulse graph
				pulsePerformed = true;
				pulseTimer->setEnabled(true);
				standInCover->setVisible(false);

				lastPulseData = data;
				updatePulseTrack(true); });
		reactor->setSevereErrorCallback([this](int reason)
										{
				toggleBaseWindow(false);
				std::string msgTxt;
				switch (reason)
				{
				case 0:
					msgTxt = "Power exceeded 10GW - an absurd limit. The reactor will SCRAM, since the simulator can't work with infinite numbers (assuming the power is still rising)"; break;
				case 1:
					msgTxt = "Since the Research reactor simulator can't simulate an explosion,\n the reactor will SCRAM. Information: Fuel temperature exceeded 950" + degCelsiusUnit + " (the uranium isotope melted)!"; break;
				default:
					msgTxt = "An unknown error has occured. An automatic SCRAM is mandatory."; break;
				}
				MessageDialog* msg = new MessageDialog(this, MessageDialog::Type::Warning, "Severe error", msgTxt);
				msg->setPosition(Vector2i((this->size().x() - msg->size().x()) / 2, (this->size().y() - msg->size().y()) / 2));
				msg->setCallback([this, msg](int /*choice*/)
					{
						toggleBaseWindow(true);
						msg->dispose();
					}); });
	}

void SimulatorGUI::viewingIntervalChanged(bool firstChanged)
{
		const double timeElapsed = reactor->getCurrentTime();
		const double range = std::min(timeElapsed, DELETE_OLD_DATA_TIME_DEFAULT);
		if (firstChanged)
		{
			viewStart = std::max(0., timeElapsed - DELETE_OLD_DATA_TIME_DEFAULT) + std::round(1000 * displayTimeSlider->value(0) * range) * 1e-3;
			timeAtLastChange = timeElapsed;
		}
		{ // update range
			float vals[2];
			for (int i = 0; i < 2; i++)
				vals[i] = displayTimeSlider->value(i);
			displayBox->setValue((float)(std::round(1000 * range * (vals[1] - vals[0])) * 1e-3));
		}
	}

SimulatorGUI::SimulatorGUI(std::shared_ptr<IUiConfigProvider> configProvider = nullptr)
		: nanogui::Screen(Vector2i(WINDOW_DEFAULT_WIDTH, WINDOW_DEFAULT_HEIGHT), "Research reactor simulator"),
		  uiConfigProvider(configProvider ? std::move(configProvider) : std::make_shared<DefaultUiConfigProvider>()),
		  uiStyleConfig(uiConfigProvider->getStyleConfig()),
		  uiPanelLayoutConfig(uiConfigProvider->getPanelLayoutConfig()),
		  coolBlue(uiStyleConfig.accentColor)
{
		cout << "======= Research reactor simulator " << version() << " =======" << endl;

		// Load settings
		properties = new Settings();

		// Keyboard
		for (int i = 0; i < NUMBER_OF_CONTROL_RODS; i++)
			lastKeyPressed[i] = false;
		panelsLayout = new BoxLayout(Orientation::Horizontal, Alignment::Middle, uiPanelLayoutConfig.panelMargin, uiPanelLayoutConfig.panelSpacing);

		// Theme
		this->mTheme->mTabInnerMargin = uiStyleConfig.tabInnerMargin;
		this->mTheme->mStandardFontSize = uiStyleConfig.standardFontSize;
		this->mTheme->mTextColor = uiStyleConfig.themeTextColor;
		this->mTheme->mButtonCornerRadius = uiStyleConfig.buttonCornerRadius;
		this->mTheme->mTabMaxButtonWidth = uiStyleConfig.tabMaxButtonWidth;
		this->mTheme->mTabButtonVerticalPadding = uiStyleConfig.tabButtonVerticalPadding;
		this->mTheme->mBorderDark = coolBlue;
		this->mTheme->mBorderLight = Color(coolBlue.r(), coolBlue.g(), coolBlue.b(), 0.4f);
		// this->mTheme->mBorderWidth = 0.8f;
		this->mTheme->mTextBoxFontSize = uiStyleConfig.textBoxFontSize;
		// Set minimum size
		glfwSetWindowSizeLimits(mGLFWWindow, WINDOW_DEFAULT_WIDTH, WINDOW_DEFAULT_HEIGHT, GLFW_DONT_CARE, GLFW_DONT_CARE);

		// Icon
		GLFWimage icons[WINDOW_ICON_NUM];
		int idx = 0;
		for (int size = 24; size < 12 * (2 + WINDOW_ICON_NUM); size += 12)
		{
			icons[idx].width = size;
			icons[idx].height = size;
			icons[idx].pixels = createPixelData(size, size);
			idx++;
		}
		glfwSetWindowIcon(mGLFWWindow, WINDOW_ICON_NUM, icons);

		// Initialize the reactor simulator
		initializeSimulator();

#if defined(_WIN32)
		// Initialize THE BOX
		memset(btns, false, 11 * sizeof(bool));
		if (!reactor->scriptCommands.size())
			initializeSerial();
		if (boxConnected)
		{
			std::cout << "===========The Box Mk. III===========" << std::endl;
		}
#endif
		RelativeGridLayout *baseLayout = new RelativeGridLayout();
		baseLayout->appendCol(1.f);
		baseLayout->appendRow(1.f);
		this->setLayout(baseLayout);
		baseWindow = this->add<CustomWindow>("");
		baseLayout->setAnchor(baseWindow, RelativeGridLayout::makeAnchor(0, 0));

		// Create a layout for the window
		relativeLayout = new RelativeGridLayout();
		relativeLayout->appendCol(1.f);
		relativeLayout->appendRow(properties->graphSize);												// 0 graph
		relativeLayout->appendRow(1.f - properties->graphSize);											// 1 controls
		relativeLayout->appendRow(RelativeGridLayout::Size(2.f, RelativeGridLayout::SizeType::Fixed));	// 2 border
		relativeLayout->appendRow(RelativeGridLayout::Size(30.f, RelativeGridLayout::SizeType::Fixed)); // 3 bottom panel
		baseWindow->setLayout(relativeLayout);
		baseWindow->setBackgroundColor(uiStyleConfig.baseWindowBackgroundColor);
		baseWindow->setDrawBackground(true);

		CustomWidget *bottomBorder = baseWindow->add<CustomWidget>();
		bottomBorder->setBackgroundColor(Color(255, 255));
		bottomBorder->setDrawBackground(true);
		bottomBorder->setBackgroundColor(coolBlue);
		relativeLayout->setAnchor(bottomBorder, RelativeGridLayout::makeAnchor(0, 2));

		// Create the graph element
		initializeGraph();

		// Create the bottom panel
		createBottomPanel();

		// Create a place for all the other stuff
		tabControl = baseWindow->add<CustomTabWidget>();
		tabControl->header()->setStretch(true);
		tabControl->header()->setButtonAlignment(NVGalign::NVG_ALIGN_CENTER | NVGalign::NVG_ALIGN_MIDDLE);
		tabControl->header()->setFontSize(20.f);
		relativeLayout->setAnchor(tabControl, RelativeGridLayout::makeAnchor(0, 1));

		// Create main tab
		createMainTab();

		// Create graph screen tab
		createGraphSettingsTab();

		// Create physics settings tab
		createPhysicsSettingsTab();

		// Create rod settings tab
		createRodSettingsTab();

		// Create delayed groups tab
		createDelayedGroupsTab();

		// Create data in/out tab
		// Widget* data_tab = tabControl->createTab("Data tab settings");
		// data_tab->setLayout(new BoxLayout(Orientation::Vertical, Alignment::Minimum, 10, 10));

		// Create operation modes tab
		createOperationModesTab();

		// Create operational limits and conditions tab
		createOperationalLimitsTab();

		// Create pulse tab
		createPulseTab();

		// Create other tab
		createOtherTab();

		tabControl->setActiveTab(0);

		// Create layout
		performLayout();
	}

void SimulatorGUI::createBottomPanel()
{
		CustomWidget *bottomPanel = baseWindow->add<CustomWidget>();
		bottomPanel->setId("bottom panel");
		bottomPanel->setDrawBackground(true);
		bottomPanel->setBackgroundColor(uiStyleConfig.bottomPanelBackgroundColor);
		relativeLayout->setAnchor(bottomPanel, RelativeGridLayout::makeAnchor(0, 3));
		RelativeGridLayout *bottomLayout = new RelativeGridLayout();
		bottomLayout->appendRow(1.f);
		bottomLayout->appendCol(RelativeGridLayout::Size(120.f, RelativeGridLayout::SizeType::Fixed)); // 0 version label
		bottomLayout->appendCol(RelativeGridLayout::Size(1.f, RelativeGridLayout::SizeType::Fixed));   // 1 border
		bottomLayout->appendCol(RelativeGridLayout::Size(100.f, RelativeGridLayout::SizeType::Fixed)); // 2 fps label
		bottomLayout->appendCol(RelativeGridLayout::Size(1.f, RelativeGridLayout::SizeType::Fixed));   // 3 border
		bottomLayout->appendCol(1.f);																   // 4 speed label
		bottomLayout->appendCol(RelativeGridLayout::Size(1.f, RelativeGridLayout::SizeType::Fixed));   // 5 border
		bottomLayout->appendCol(RelativeGridLayout::Size(100.f, RelativeGridLayout::SizeType::Fixed)); // 6 buttons
		bottomLayout->appendCol(RelativeGridLayout::Size(1.f, RelativeGridLayout::SizeType::Fixed));   // 7 border
		bottomLayout->appendCol(RelativeGridLayout::Size(80.f, RelativeGridLayout::SizeType::Fixed));  // 8 simulation time factor
		bottomLayout->appendCol(RelativeGridLayout::Size(1.f, RelativeGridLayout::SizeType::Fixed));   // 9 border
		bottomLayout->appendCol(RelativeGridLayout::Size(150.f, RelativeGridLayout::SizeType::Fixed)); // 10 time label
		bottomPanel->setLayout(bottomLayout);

		for (int i = 1; i < 11; i += 2)
		{
			CustomWidget *border = bottomPanel->add<CustomWidget>();
			border->setDrawBackground(true);
			border->setBackgroundColor(coolBlue);
			bottomLayout->setAnchor(border, RelativeGridLayout::makeAnchor(i, 0));
		}

		CustomLabel *versionLabel = bottomPanel->add<CustomLabel>("v " + version());
		versionLabel->setTextAlignment(CustomLabel::TextAlign::LEFT | CustomLabel::TextAlign::VERTICAL_CENTER);
		versionLabel->setPadding(0, 5.f);
		versionLabel->setFontSize(20.f);
		versionLabel->setColor(Color(255, 255));
		bottomLayout->setAnchor(versionLabel, RelativeGridLayout::makeAnchor(0, 0));

		fpsLabel = bottomPanel->add<CustomLabel>("FPS: ");
		fpsLabel->setTextAlignment(CustomLabel::TextAlign::LEFT | CustomLabel::TextAlign::VERTICAL_CENTER);
		fpsLabel->setFontSize(20.f);
		fpsLabel->setColor(Color(255, 255));
		fpsLabel->setPadding(0, 5.f);
		bottomLayout->setAnchor(fpsLabel, RelativeGridLayout::makeAnchor(2, 0));

		CustomLabel *speedText = bottomPanel->add<CustomLabel>("Simulation speed:");
		speedText->setPadding(2, 5);
		speedText->setColor(Color(255, 255));
		speedText->setFontSize(20.f);
		speedText->setTextAlignment(CustomLabel::TextAlign::RIGHT | CustomLabel::TextAlign::VERTICAL_CENTER);
		bottomLayout->setAnchor(speedText, RelativeGridLayout::makeAnchor(4, 0, 1, 1, Alignment::Maximum));

		Widget *speedToolPanel = bottomPanel->add<Widget>();
		bottomLayout->setAnchor(speedToolPanel, RelativeGridLayout::makeAnchor(6, 0, 1, 1, Alignment::Middle));
		speedToolPanel->setLayout(new BoxLayout(Orientation::Horizontal, Alignment::Middle, 0, 5));
		slowDown = speedToolPanel->add<ToolButton>(ENTYPO_ICON_FB);
		slowDown->setFlags(Button::Flags::NormalButton);
		slowDown->setCallback([this]()
							  {
				if (!reactor->isPaused())
				{
					this->setSimulationTime(std::max((int)selectedTime - 1, 0));
				} });
		playPause = speedToolPanel->add<ToolButton>(ENTYPO_ICON_PAUS);
		playPause->setChangeCallback([this](bool value)
									 { playPauseSimulation(!value); });
		speedUp = speedToolPanel->add<ToolButton>(ENTYPO_ICON_FF);
		speedUp->setFlags(Button::Flags::NormalButton);
		speedUp->setCallback([this]()
							 {
				if (!this->reactor->isPaused())
				{
					this->setSimulationTime(std::min((int)selectedTime + 1, SIM_TIME_FACTOR_NUMBER - 1));
				} });

		simFactorLabel = bottomPanel->add<CustomLabel>("real-time");
		simFactorLabel->setColor(Color(255, 255));
		simFactorLabel->setFontSize(20.f);
		simFactorLabel->setTextAlignment(CustomLabel::TextAlign::HORIZONTAL_CENTER | CustomLabel::TextAlign::VERTICAL_CENTER);
		bottomLayout->setAnchor(simFactorLabel, RelativeGridLayout::makeAnchor(8, 0));

		timeLabel = bottomPanel->add<CustomLabel>("00:00:00");
		timeLabel->setColor(Color(255, 255));
		timeLabel->setFontSize(20.f);
		timeLabel->setTextAlignment(CustomLabel::TextAlign::HORIZONTAL_CENTER | CustomLabel::TextAlign::VERTICAL_CENTER);
		bottomLayout->setAnchor(timeLabel, RelativeGridLayout::makeAnchor(10, 0));

		simStatusLabel = bottomPanel->add<CustomLabel>(utf8(ENTYPO_ICON_PLAY).data(), "icons");
		simStatusLabel->setColor(Color(255, 255));
		simStatusLabel->setFontSize(40.f);
		simStatusLabel->setFixedWidth(30);
		simStatusLabel->setTextAlignment(CustomLabel::TextAlign::HORIZONTAL_CENTER | CustomLabel::TextAlign::VERTICAL_CENTER);
		bottomLayout->setAnchor(simStatusLabel, RelativeGridLayout::makeAnchor(10, 0, 1, 1, Alignment::Minimum, Alignment::Fill));
	}

void SimulatorGUI::resetSimToStart()
{
		reactor->resetSimulator();
		properties = new Settings();
		updateSettings(false);
		setSimulationTime(8); // 1x speed
		playPauseSimulation(true);
	}

void SimulatorGUI::handleDerivativeChange()
{
		for (int i = 0; i < NUMBER_OF_CONTROL_RODS; i++)
		{
			rodDerivatives[i]->setYdata(reactor->rods[i]->derivativeArray());
			double avg = reactor->rods[i]->getRodWorth() / *reactor->rods[i]->getRodSteps();
			double lim = std::round(std::max(std::ceil(reactor->rods[i]->getRodWorth() * reactor->rods[i]->derivativeArray()[reactor->rods[i]->maxDerivative()] / avg) * avg, 2 * avg));
			rodDerivatives[i]->setLimits(0., 1., 0., lim);
		}
	}

void SimulatorGUI::hardcoreMode(bool value)
{
		reactivityPlot->setEnabled(!value);
		rodReactivityPlot->setEnabled(properties->rodReactivityPlot && !value);

		reactivityShow->setVisible(!value);
		rodReactivityShow->setVisible(!value);

		canvas->setPadding(90.f, 25.f, value ? 120.f : 220.f, 50.f);

		performLayout();
	}

IntBox<int> *SimulatorGUI::makeSimulationSetting(CustomWidget *parent, int initialValue, std::string text)
{
		IntBox<int> *tempBox = makeSettingLabel<IntBox<int>>(parent, text, 100, initialValue);
		tempBox->setFixedWidth(100);
		tempBox->setFormat("[0-9]+");
		tempBox->setUnits("%");
		tempBox->setMinMaxValues(0, 100);
		tempBox->setValueIncrement(1);
		tempBox->setSpinnable(true);
		tempBox->setDefaultValue(to_string(initialValue));
		return tempBox;
	}

void SimulatorGUI::handleDebugChanged()
{
		reactor->setDebugMode(debugMode);
		if (debugMode)
		{
#if defined(_WIN32)
			ShowWindow(GetConsoleWindow(), SW_SHOW);
#endif
		}
		else
		{
#if defined(_WIN32)
			ShowWindow(GetConsoleWindow(), SW_HIDE);
#endif
		}
	}

SimulatorGUI::~SimulatorGUI()
{
		delete reactor;
		for (int i = 0; i < 2; i++)
		{
			for (int j = 0; j < 3; j++)
			{
				delete[] operationModesPlots[i][j];
			}
		}
#if defined(_WIN32)
		if (boxConnected)
			delete theBox;
#endif
	}

bool SimulatorGUI::keyboardEvent(int key, int scancode, int action, int modifiers)
{
		if (Screen::keyboardEvent(key, scancode, action, modifiers))
			return true;
		if (!baseWindow->enabled())
			return false;

		if (action == GLFW_PRESS)
		{
			if (last10keys.size() == 10)
			{
				last10keys.pop_front();
			}
			last10keys.push_back(key);
			size_t keyN = last10keys.size();
			bool isGodMode = false;
			bool isDebug = false;
			bool isReset = false;
			if (keyN >= 7)
			{
				isGodMode = true;
				for (size_t i = keyN - 7; i < keyN; i++)
				{
					isGodMode = isGodMode && (last10keys[i] == cheat1[i - (keyN - 7)]);
				}
			}
			if (keyN >= 5)
			{
				isDebug = true;
				isReset = true;
				for (size_t i = keyN - 5; i < keyN; i++)
				{
					isDebug = isDebug && (last10keys[i] == cheat2[i - (keyN - 5)]);
					isReset = isReset && (last10keys[i] == cheat3[i - (keyN - 5)]);
				}
			}

			if (isGodMode)
			{
				reactor->godMode = !reactor->godMode;
				cout << "God mode: " << reactor->godMode << endl;
				return true;
			}
			if (isDebug)
			{
				debugMode = !debugMode;

				handleDebugChanged();
			}
			if (isReset)
			{
				resetSimToStart();
			}
		}

		// Safety rod
		if (key == safetyRodControl)
		{
			if (action == GLFW_RELEASE)
			{
				lastKeyPressed[0] = false;
				reactor->safetyRod()->clearCommands();
			}
			else
			{
				if (properties->allRodsAtOnce || !(lastKeyPressed[1] || lastKeyPressed[2]))
					lastKeyPressed[0] = true;
			}
		}
		else if (key == enableSafetyCommand && action == GLFW_PRESS)
		{
			reactor->safetyRod()->setEnabled(!reactor->safetyRod()->isEnabled());
		} // Regulation rod
		else if (key == regulatoryRodControl)
		{
			if (action == GLFW_RELEASE)
			{
				lastKeyPressed[1] = false;
				reactor->regulatingRod()->clearCommands();
			}
			else
			{
				if (properties->allRodsAtOnce || !(lastKeyPressed[0] || lastKeyPressed[2]))
					lastKeyPressed[1] = true;
			}
		}
		else if (key == enableRegCommand && action == GLFW_PRESS)
		{
			reactor->regulatingRod()->setEnabled(!reactor->regulatingRod()->isEnabled());
		} // Shim rod
		else if (key == shimRodControl)
		{
			if (action == GLFW_RELEASE)
			{
				lastKeyPressed[2] = false;
				reactor->shimRod()->clearCommands();
			}
			else
			{
				if (properties->allRodsAtOnce || !(lastKeyPressed[0] || lastKeyPressed[1]))
					lastKeyPressed[2] = true;
			}
		}
		else if (key == enableShimCommand && action == GLFW_PRESS)
		{
			reactor->shimRod()->setEnabled(!reactor->shimRod()->isEnabled());
		} // Move rod up
		else if (key == rodUpCommand && action != GLFW_REPEAT)
		{
			if (action == GLFW_RELEASE)
			{
				reactor->safetyRod()->clearCommands(ControlRod::CommandType::Top);
				reactor->regulatingRod()->clearCommands(ControlRod::CommandType::Top);
				reactor->shimRod()->clearCommands(ControlRod::CommandType::Top);
			}
			else
			{
				for (int i = 0; i < NUMBER_OF_CONTROL_RODS; i++)
				{
					if (lastKeyPressed[i])
						reactor->rods[i]->commandToTop();
				}
			}
		} // Move rod down
		else if (key == rodDownCommand && action != GLFW_REPEAT)
		{
			if (action == GLFW_RELEASE)
			{
				reactor->safetyRod()->clearCommands(ControlRod::CommandType::Bottom);
				reactor->regulatingRod()->clearCommands(ControlRod::CommandType::Bottom);
				reactor->shimRod()->clearCommands(ControlRod::CommandType::Bottom);
			}
			else
			{
				for (int i = 0; i < NUMBER_OF_CONTROL_RODS; i++)
				{
					if (lastKeyPressed[i])
						reactor->rods[i]->commandToBottom();
				}
			}
		} // SCRAM
		else if (key == scramCommand && action == GLFW_PRESS)
		{
			reactor->scram(Simulator::ScramSignals::User);
		} // reset scram
		else if (key == resetScramCommand && action == GLFW_PRESS)
		{
			reactor->scram(Simulator::ScramSignals::None);
		} // pause
		else if (key == pauseCommand && action == GLFW_PRESS)
		{
			playPauseSimulation(reactor->isPaused());
		} // fast forward
		else if (key == fasterCommand && action != GLFW_RELEASE)
		{
			if (!reactor->isPaused())
			{
				setSimulationTime(std::min((int)selectedTime + 1, SIM_TIME_FACTOR_NUMBER - 1));
			}
		} // slow down
		else if (key == slowerCommand && action != GLFW_RELEASE)
		{
			if (!reactor->isPaused())
			{
				setSimulationTime(std::max((int)selectedTime - 1, 0));
			}
		} // exit
		else if (key == GLFW_KEY_ESCAPE)
		{
			setVisible(false);
		} // change active tab
		else if (key == tabChangeCommand && action == GLFW_PRESS)
		{
			if (modifiers & GLFW_MOD_SHIFT)
			{
				if (tabControl->activeTab() == 0)
				{
					tabControl->setActiveTab(tabControl->tabCount() - 1);
				}
				else
				{
					tabControl->setActiveTab((tabControl->activeTab() - 1) % tabControl->tabCount());
				}
			}
			else
			{
				tabControl->setActiveTab((tabControl->activeTab() + 1) % tabControl->tabCount());
			}
		} // fire
		else if (key == firePulseCommand && action == GLFW_PRESS)
		{
			reactor->beginPulse();
		} // toogle neutron source
		else if (key == sourceToggleCommand && action == GLFW_PRESS)
		{
			reactor->setNeutronSourceInserted(!reactor->getNeutronSourceInserted());
			neutronSourceCB->setChecked(reactor->getNeutronSourceInserted());
		}
		else if (action == GLFW_PRESS && key == demoModeCommand && modifiers & GLFW_MOD_CONTROL)
		{
			reactor->setDemoMode();
			reactor->regulatingRod()->setOperationMode(ControlRod::OperationModes::Manual);
			rodMode->setSelectedIndex(0);
		}
		else if (action == GLFW_PRESS && key == demoModeHighPowerCommand && modifiers & GLFW_MOD_CONTROL)
		{
			reactor->setHighPowerDemoMode();
			reactor->regulatingRod()->setOperationMode(ControlRod::OperationModes::Manual);
			rodMode->setSelectedIndex(0);
		}
		else
		{
			return false;
		}
		// If code execution got to here, it means that one of the if's must have been executed, so return true(handled)
		return true;
	}

	virtual bool resizeEvent(const Eigen::Vector2i &size)
	{
		if (Screen::resizeEvent(size))
			return true;
		if (layoutStart)
		{
			performLayout();
		}
		else
		{
			layoutStart = true;
		}
		return true;
	}

	// DEBUG: Override to trace mouse events
	virtual bool mouseButtonEvent(const nanogui::Vector2i &p, int button, bool down, int modifiers) override
	{
		static std::ofstream debugLog("debug_clicks.txt", std::ios::app);
		debugLog << "[DEBUG] mouseButtonEvent at (" << p.x() << ", " << p.y() << ") button=" << button << " down=" << down << std::endl;
		Widget *target = findWidget(p);
		if (target)
		{
			debugLog << "[DEBUG] findWidget found: " << typeid(*target).name() << std::endl;
		}
		else
		{
			debugLog << "[DEBUG] findWidget found: nullptr" << std::endl;
		}
		bool result = Screen::mouseButtonEvent(p, button, down, modifiers);
		debugLog << "[DEBUG] Screen::mouseButtonEvent returned: " << result << std::endl;
		debugLog.flush();
		return result;
	}

private:
	bool lastKeyPressed[NUMBER_OF_CONTROL_RODS];
	int safetyRodControl = GLFW_KEY_S;
	int regulatoryRodControl = GLFW_KEY_R;
	int shimRodControl = GLFW_KEY_C;
	int rodUpCommand = GLFW_KEY_UP;
	int rodDownCommand = GLFW_KEY_DOWN;
	int scramCommand = GLFW_KEY_X;
	int resetScramCommand = GLFW_KEY_0;
	int enableSafetyCommand = GLFW_KEY_1;
	int enableRegCommand = GLFW_KEY_2;
	int enableShimCommand = GLFW_KEY_3;
	int pauseCommand = GLFW_KEY_SPACE;
	int fasterCommand = GLFW_KEY_RIGHT;
	int slowerCommand = GLFW_KEY_LEFT;
	int tabChangeCommand = GLFW_KEY_TAB;
	int firePulseCommand = GLFW_KEY_P;
	int sourceToggleCommand = GLFW_KEY_N;
	int demoModeCommand = GLFW_KEY_D;
	int demoModeHighPowerCommand = GLFW_KEY_F;
	int cheat1[7] = {GLFW_KEY_G, GLFW_KEY_O, GLFW_KEY_D, GLFW_KEY_M, GLFW_KEY_O, GLFW_KEY_D, GLFW_KEY_E};
	int cheat2[5] = {GLFW_KEY_D, GLFW_KEY_E, GLFW_KEY_B, GLFW_KEY_U, GLFW_KEY_G};
	int cheat3[5] = {GLFW_KEY_R, GLFW_KEY_E, GLFW_KEY_S, GLFW_KEY_E, GLFW_KEY_T};
	bool debugMode = false;
	deque<int> last10keys = deque<int>();
	size_t displayInterval[2] = {0, 0};
	bool btns[11];
	int lastModeState = 0;

	void updateAlphaGraph()
	{
		// Point 1
		alphaX[0] = 0.;
		alphaY[0] = properties->alpha0;

		// Point 2
		alphaX[1] = (double)properties->alphaT1;
		alphaY[1] = properties->alphaAtT1;

		// Point 3
		alphaX[2] = 1000.;
		alphaY[2] = properties->alphaAtT1 + (float)(properties->alphaK * (1000.f - properties->alphaT1));

		// Autoscale
		alphaPlot->setLimits(0., 1000., (std::ceil(std::min(std::min(properties->alpha0, properties->alphaAtT1), alphaY[2]) / 5.) - 1.) * 5., (std::floor(std::max(std::max(properties->alpha0, properties->alphaAtT1), alphaY[2]) / 5) + 1.) * 5.);
		alphaPlot->setMajorTickNumber((size_t)std::max((alphaPlot->limits()[3] - alphaPlot->limits()[2] - 5.) / 5., 0.));
	}

	std::string getTimeSinceStart()
	{
		size_t time[3];
		double t = reactor->getCurrentTime();
		time[2] = (size_t)floor(fmod(t, 60.));
		time[1] = (size_t)floor(fmod(t, 3600.) / 60.);
		time[0] = (size_t)floor(t / 3600.);
		std::string ret[3];
		for (int i = 0; i < 3; i++)
		{
			ret[i] = ((time[i] < 10) ? ("0" + to_string(time[i])) : (to_string(time[i])));
		}
		return ret[0] + ":" + ret[1] + ":" + ret[2];
	}

public:
	double lastTime = get_seconds_since_epoch();

	virtual void draw(NVGcontext *ctx)
	{
		double reactorElapsed = reactor->getCurrentTime();
		if (startScript.size())
		{
			loadScriptFromFile(startScript);
			startScript = "";
		}

		// Run new calculation
		reactor->runLoop();

		// Get from which index to which index the data will be drawn and update view slider
		const double sliderRange = std::min(DELETE_OLD_DATA_TIME_DEFAULT, reactorElapsed);
		double sliderStart = displayTimeSlider->value(0) * sliderRange;
		if (viewStart >= 0.)
		{
			if (!timeLockedBox->checked())
			{
				double diff = reactorElapsed - timeAtLastChange;
				sliderStart = viewStart + diff - max(0., reactorElapsed - DELETE_OLD_DATA_TIME_DEFAULT);
				reculculateDisplayInterval(max(viewStart + diff, 0.), viewStart + diff + properties->displayTime);
			}
			else
			{
				sliderStart = viewStart - max(0., reactorElapsed - DELETE_OLD_DATA_TIME_DEFAULT);
				reculculateDisplayInterval(max(viewStart, 0.), viewStart + properties->displayTime);
			}
		}
		else
		{
			sliderStart = max(reactorElapsed - properties->displayTime, 0.);
			reculculateDisplayInterval(sliderStart, reactorElapsed);
		}
		displayTimeSlider->setValue(0, (float)(sliderStart / sliderRange));
		displayTimeSlider->setValue(1, (float)min(1., (sliderStart + properties->displayTime) / sliderRange));

		// Link plots to display interval
		reactivityPlot->setPlotRange(displayInterval[0], displayInterval[1]);
		rodReactivityPlot->setPlotRange(displayInterval[0], displayInterval[1]);
		temperaturePlot->setPlotRange(displayInterval[0], displayInterval[1]);
		powerPlot->setPlotRange(displayInterval[0], displayInterval[1]);
		for (size_t i = 0; i < 6; i++)
		{
			delayedGroups[i]->setPlotRange(displayInterval[0], displayInterval[1]);
		}

		try
		{
			// Save times for better performance
			double timeStart = reactor->time_[displayInterval[0]];
			double timeEnd = reactor->time_[displayInterval[1]];
			// Set reactivity scaling
			reactivityPlot->setLimits(timeStart, timeEnd, properties->reactivityGraphLimits[0], properties->reactivityGraphLimits[1]);
			rodReactivityPlot->setLimits(timeStart, timeEnd, properties->reactivityGraphLimits[0], properties->reactivityGraphLimits[1]);
			// Set power plot scaling
			pair<int, int> newExtremes = recalculatePowerExtremes();
			if (isZero.first || isZero.second)
			{
				if (isZero.first && isZero.second)
				{
					powerPlot->setLimits(timeStart, timeEnd,
										 0., 1.);
				}
				else
				{
					powerPlot->setLimits(timeStart, timeEnd,
										 0., pow(10., std::max(newExtremes.first, newExtremes.second)));
				}
			}
			else
			{
				powerPlot->setLimits(timeStart, timeEnd,
									 /*(newExtremes.first < -3) ? 0. : pow(10., newExtremes.first)*/ powerPlot->getYlog() ? pow(10., newExtremes.first) : 0., pow(10., newExtremes.second));
			}
			// Set temperature scaling
			temperaturePlot->setLimits(timeStart, timeEnd, properties->temperatureGraphLimits[0], properties->temperatureGraphLimits[1]);

			// Set stacked graph scaling
			if (tabControl->activeTab() == 4)
			{
				for (int i = 0; i < 6; i++)
				{
					delayedGroups[i]->setLimits(timeStart, timeEnd, 0., 3.);
				}
			}
		}
		catch (exception e)
		{
			cerr << "Index out of bounds: SimulatorGUI.draw" << "\n"
				 << e.what() << endl;
		}

		// re-draw control rods
		if (tabControl->activeTab() == 3)
		{
			float pointPos;
			for (int i = 0; i < NUMBER_OF_CONTROL_RODS; i++)
			{
				rodCurves[i]->setPointerPosition(reactor->rods[i]->getCurrentPCM() / reactor->rods[i]->getRodWorth());
				rodCurves[i]->setRodPosition(*reactor->rods[i]->getExactPosition() / *reactor->rods[i]->getRodSteps());
				rodCurves[i]->setHorizontalPointerPosition(*reactor->rods[i]->getExactPosition() / *reactor->rods[i]->getRodSteps());

				pointPos = *reactor->rods[i]->getExactPosition();
				pointPos = (float)(reactor->rods[i]->derivativeArray()[(int)std::floor(pointPos)] * (std::ceil(pointPos) - pointPos) + (pointPos - std::floor(pointPos)) * reactor->rods[i]->derivativeArray()[(int)std::ceil(pointPos)]);
				rodDerivatives[i]->setPointerPosition((float)(reactor->rods[i]->getRodWorth() * pointPos / rodDerivatives[i]->limits()[3]));
				rodDerivatives[i]->setHorizontalPointerPosition(*reactor->rods[i]->getExactPosition() / *reactor->rods[i]->getRodSteps());
			}
		}

		// Show data
		powerShow->setData(reactor->getCurrentPower());
		size_t curIndx = reactor->getCurrentIndex();
		reactivityShow->setData(reactor->reactivity_[curIndx]);
		rodReactivityShow->setData(reactor->rodReactivity_[curIndx]);
		temperatureShow->setData(reactor->temperature_[curIndx]);
		waterTemperatureShow->setData(*reactor->getWaterTemperature());
		// waterLevelShow->setData(*reactor->getWaterLevel() * 100.);
		periodShow->setData(*reactor->getReactorPeriod());

		// Data for graphical reactor period display
		periodDisplay->setPeriod(*reactor->getReactorPeriod());

		double newTime = get_seconds_since_epoch();
		float thisFps = powf((float)(newTime - lastTime), -1.f);
		fpsSum += thisFps;
		if (fpsCount == 0 || fpsCount == 20)
		{
			fpsLabel->setCaption("test");
			auto test1 = fpsLabel->caption();
			fpsLabel->setCaption("FPS: " + to_string((int)roundf(fpsCount ? (fpsSum / fpsCount) : thisFps)));
			fpsSum = thisFps;
			fpsCount %= 20;
		}
		fpsCount++;
		lastTime = newTime;

		// Update alpha plot
		float tempNow = reactor->getCurrentTemperature();
		alphaPlot->setHorizontalPointerPosition(tempNow / 1000.f);
		alphaPlot->setPointerPosition((float)((reactor->getReactivityCoefficient(tempNow) - alphaPlot->limits()[2]) / (alphaPlot->limits()[3] - alphaPlot->limits()[2])));

		// Update the text
		for (int i = 0; i < NUMBER_OF_CONTROL_RODS; i++)
			rodBox[i]->setValue((int)std::ceil(*reactor->rods[i]->getExactPosition()));

		// Update time
		timeLabel->setCaption(getTimeSinceStart());

		if (!reactor->getScramStatus())
		{
			SimulatorGuiUiHelpers::updateScramWarningIndicators(
				uiStyleConfig,
				reactor,
				properties,
				tempNow,
				periodScram,
				fuelTemperatureScram,
				waterTemperatureScram,
				powerScram);
		}

		if (shouldUpdateNeutronSource)
		{
			sourceSettings->performLayout(ctx);
			shouldUpdateNeutronSource = false;
		}

		/* Draw the user interface */
		Screen::draw(ctx);

		// Send dickbut PNG bits over serial
#if defined(_WIN32)
		if (boxConnected)
		{
			if (theBox->IsConnected())
				handleBox();
		}
		else
		{
			updateCOMports();
		}
#endif
	}

	double lastData = 0.;
#if defined(_WIN32)
	void handleBox()
	{
		LEDstatus = (uint16_t)0;
		// Write LED status
		int scramS = reactor->getScramStatus();
		if (Simulator::ScramSignals::Period & scramS)
			LEDstatus |= SCRAM_PER;
		if (Simulator::ScramSignals::FuelTemperature & scramS)
			LEDstatus |= SCRAM_FT;
		if (Simulator::ScramSignals::WaterTemperature & scramS)
			LEDstatus |= SCRAM_WT;
		if (Simulator::ScramSignals::Power & scramS)
			LEDstatus |= SCRAM_POW;
		if (Simulator::ScramSignals::User & scramS)
			LEDstatus |= SCRAM_MAN;

		if (reactor->safetyRod()->isEnabled())
		{
			LEDstatus |= ROD_SAFETY_ENBL;
		}
		if (reactor->regulatingRod()->isEnabled())
		{
			LEDstatus |= ROD_REG_ENBL;
		}
		if (reactor->shimRod()->isEnabled())
		{
			LEDstatus |= ROD_SHIM_ENBL;
		}

		if (reactor->safetyRod()->getCommandType() == ControlRod::CommandType::Top || *reactor->safetyRod()->getExactPosition() == (float)*reactor->safetyRod()->getRodSteps())
		{
			LEDstatus |= ROD_SAFETY_UP;
		}
		if (reactor->regulatingRod()->getCommandType() == ControlRod::CommandType::Top || *reactor->regulatingRod()->getExactPosition() == (float)*reactor->regulatingRod()->getRodSteps())
		{
			LEDstatus |= ROD_REG_UP;
		}
		if (reactor->shimRod()->getCommandType() == ControlRod::CommandType::Top || *reactor->shimRod()->getExactPosition() == (float)*reactor->shimRod()->getRodSteps())
		{
			LEDstatus |= ROD_SHIM_UP;
		}
		if (reactor->safetyRod()->getCommandType() == ControlRod::CommandType::Bottom || *reactor->safetyRod()->getExactPosition() == 0.f)
		{
			LEDstatus |= ROD_SAFETY_DOWN;
		}
		if (reactor->regulatingRod()->getCommandType() == ControlRod::CommandType::Bottom || *reactor->regulatingRod()->getExactPosition() == 0.f)
		{
			LEDstatus |= ROD_REG_DOWN;
		}
		if (reactor->shimRod()->getCommandType() == ControlRod::CommandType::Bottom || *reactor->shimRod()->getExactPosition() == 0.f)
		{
			LEDstatus |= ROD_SHIM_DOWN;
		}
		if (reactor->regulatingRod()->getOperationMode() == ControlRod::OperationModes::Pulse && reactor->getScramStatus() == 0)
		{
			LEDstatus |= FIRE_LED_B;
		}

		// Convert LED status to two bytes
		char sendByte[3];
		sendByte[0] = 77;
		sendByte[1] = LEDstatus >> 8;
		sendByte[2] = LEDstatus & 0x00ff;

		// Write LED data

		theBox->WriteData(sendByte, 3);

		// Reset sounds
		// LEDstatus &= (1 << 13) - 1;

		// Read data
		uint16_t box_data = 0;
		bool gotData = false;
		char buffer[2];
		double time_now = get_seconds_since_epoch();
		while (theBox->availableBytes() >= 2)
		{
			theBox->ReadData(buffer, 2);
			box_data = (unsigned char)buffer[1];
			box_data += ((uint16_t)buffer[0]) << 8;
			gotData = true;
			handleBoxData(box_data, time_now);
		}
		if (!gotData)
		{ // Disconnect box if no data is recieved in 1 second
			if (lastData == 0.)
			{
				lastData = time_now;
			}
			else if (time_now > lastData + 1.)
			{
				boxConnected = false;
				theBox->~Serial();
				std::cout << "Box disconnected! (timeout)" << std::endl;
				lastData = 0.;
			}
			return;
		}
	}
#endif
	bool shouldUpdateNeutronSource = false;
	void updateNeutronSourceTab()
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

	void handleBoxData(uint16_t box_data, double now)
	{
		lastData = now;
		bool rodsMoving[NUMBER_OF_CONTROL_RODS];
		for (int i = 0; i < NUMBER_OF_CONTROL_RODS; i++)
			rodsMoving[i] = (reactor->rods[i]->getCommandType() == ControlRod::CommandType::None);
		if (box_data & SCRAM_BTN)
		{
			if (!btns[0])
				reactor->scram(Simulator::ScramSignals::User);
		}
		if (box_data & FIRE_BTN)
		{
			if (!btns[1])
			{
				if (reactor->getScramStatus() == 0)
					reactor->beginPulse();
			}
		}
		if (box_data & ENABLE_SAFETY_BTN)
		{
			if (!btns[2])
			{
				if (reactor->getScramStatus() == 0)
					reactor->safetyRod()->setEnabled(!reactor->safetyRod()->isEnabled());
			}
		}
		if (box_data & UP_SAFETY_BTN)
		{
			if (!btns[3] && ((rodsMoving[1] && rodsMoving[2]) || properties->allRodsAtOnce))
				reactor->safetyRod()->commandToTop();
		}
		else
		{
			if (btns[3])
				reactor->safetyRod()->clearCommands(ControlRod::CommandType::Top);
		}
		if (box_data & DOWN_SAFETY_BTN)
		{
			if (!btns[4] && ((rodsMoving[1] && rodsMoving[2]) || properties->allRodsAtOnce))
				reactor->safetyRod()->commandToBottom();
		}
		else
		{
			if (btns[4])
				reactor->safetyRod()->clearCommands(ControlRod::CommandType::Bottom);
		}
		if (box_data & ENABLE_REG_BTN)
		{
			if (!btns[5])
			{
				if (reactor->getScramStatus() == 0)
					reactor->regulatingRod()->setEnabled(!reactor->regulatingRod()->isEnabled());
			}
		}
		if (box_data & UP_REG_BTN)
		{
			if (!btns[6] && ((rodsMoving[0] && rodsMoving[2]) || properties->allRodsAtOnce))
				reactor->regulatingRod()->commandToTop();
		}
		else
		{
			if (btns[6])
				reactor->regulatingRod()->clearCommands(ControlRod::CommandType::Top);
		}
		if (box_data & DOWN_REG_BTN)
		{
			if (!btns[7] && ((rodsMoving[0] && rodsMoving[2]) || properties->allRodsAtOnce))
				reactor->regulatingRod()->commandToBottom();
		}
		else
		{
			if (btns[7])
				reactor->regulatingRod()->clearCommands(ControlRod::CommandType::Bottom);
		}
		if (box_data & ENABLE_SHIM_BTN)
		{
			if (!btns[8])
			{
				if (reactor->getScramStatus() == 0)
					reactor->shimRod()->setEnabled(!reactor->shimRod()->isEnabled());
			}
		}
		if (box_data & UP_SHIM_BTN)
		{
			if (!btns[9] && ((rodsMoving[0] && rodsMoving[1]) || properties->allRodsAtOnce))
				reactor->shimRod()->commandToTop();
		}
		else
		{
			if (btns[9])
				reactor->shimRod()->clearCommands(ControlRod::CommandType::Top);
		}
		if (box_data & DOWN_SHIM_BTN)
		{
			if (!btns[10] && ((rodsMoving[0] && rodsMoving[1]) || properties->allRodsAtOnce))
				reactor->shimRod()->commandToBottom();
		}
		else
		{
			if (btns[10])
				reactor->shimRod()->clearCommands(ControlRod::CommandType::Bottom);
		}
		btns[0] = (box_data & SCRAM_BTN) != 0;
		btns[1] = (box_data & FIRE_BTN) != 0;
		btns[2] = (box_data & ENABLE_SAFETY_BTN) != 0;
		btns[3] = (box_data & UP_SAFETY_BTN) != 0;
		btns[4] = (box_data & DOWN_SAFETY_BTN) != 0;
		btns[5] = (box_data & ENABLE_REG_BTN) != 0;
		btns[6] = (box_data & UP_REG_BTN) != 0;
		btns[7] = (box_data & DOWN_REG_BTN) != 0;
		btns[8] = (box_data & ENABLE_SHIM_BTN) != 0;
		btns[9] = (box_data & UP_SHIM_BTN) != 0;
		btns[10] = (box_data & DOWN_SHIM_BTN) != 0;

		int mode = box_data & 7;
		if (mode != lastModeState)
		{
			rodMode->setSelectedIndex(mode);
			lastModeState = mode;
		}
	}

	// static string formatDecimals(const double x, const int decDigits, bool removeTrailingZeros = true)
	//{
	//	stringstream ss;
	//	ss << fixed;
	//	ss.precision(decDigits);
	//	ss << x;
	//	std::string res = ss.str();
	//	if (removeTrailingZeros)
	//	{
	//		while (res.back() == '0') res = res.substr(0, res.length() - 1);
	//		if (res.back() == '.') res = res.substr(0, res.length() - 1);
	//	}
	//	return res;
	// }

	void reculculateDisplayInterval(double fromTime, double toTime)
	{
		fromTime = std::max(fromTime, 0.);
		toTime = std::min(toTime, reactor->getCurrentTime());
		displayInterval[0] = reactor->getIndexFromTime(fromTime);
		displayInterval[1] = reactor->getIndexFromTime(toTime);
	}

	// Method for calculating autoscale factors
	pair<int, int> recalculatePowerExtremes(double fromTime = 0., double toTime = 0.)
	{
		int err = 0;
		if (fromTime + toTime == 0.)
		{
			fromTime = reactor->time_[displayInterval[0]];
			toTime = reactor->time_[displayInterval[1]];
		}
		try
		{
			// Find the last change since the graph begin time
			size_t startIndex = 0;
			for (; startIndex < reactor->getOrderChanges(); startIndex++)
			{
				if (reactor->getExtremeAt(startIndex).when >= fromTime)
					break;
			}
			Simulator::PowerExtreme *firstExtreme;
			if (startIndex)
			{
				startIndex--;
				firstExtreme = &reactor->getExtremeAt(startIndex);
			}
			else
			{
				firstExtreme = &reactor->trailingExtreme;
			}
			err = 1;
			// Find the last change before the graph end time
			size_t endIndex = startIndex;
			for (; endIndex < reactor->getOrderChanges(); endIndex++)
			{
				if (reactor->getExtremeAt(endIndex).when > toTime)
					break;
			}
			if (endIndex != 0)
				endIndex--;
			err = 2;
			// If there is only one order change, return the order
			if (startIndex == endIndex)
			{
				isZero.first = firstExtreme->isZero;
				isZero.second = firstExtreme->isZero;
				return pair<int, int>(firstExtreme->order, firstExtreme->order + 1);
			}
			else
			{
				// Iterate through the order changes and find the smallest and largest order
				int minOrder = firstExtreme->order;
				int maxOrder = minOrder;
				isZero.first = firstExtreme->isZero;
				isZero.second = firstExtreme->isZero;
				for (size_t i = startIndex + 1; i <= endIndex; i++)
				{
					Simulator::PowerExtreme current = reactor->getExtremeAt(i);
					if (current.isZero)
					{
						isZero.first = true;
					}
					else
					{
						if (isZero.second)
						{
							maxOrder = current.order;
							isZero.second = false;
						}
						else
						{
							maxOrder = max(maxOrder, current.order);
						}
						minOrder = min(minOrder, current.order);
					}
				}
				isZero.first = isZero.first || minOrder < -7;
				return pair<int, int>(minOrder, ++maxOrder);
			}
		}
		catch (exception e)
		{
			cerr << "recalculatePowerExtremes(): error code " << err << endl;
			isZero.first = true;
			return pair<int, int>(0, 10);
		}
	}

	std::vector<string> getCOMports()
	{
#ifdef _WIN32
		TCHAR *ptr = new TCHAR[65535];
		TCHAR *temp_ptr;
		unsigned long dwChars = QueryDosDevice(NULL, ptr, 65535);
		std::vector<string> comPorts_ = std::vector<string>();
		while (dwChars)
		{
			int port;
			if (sscanf(ptr, "COM%d", &port) == 1)
			{
				comPorts_.push_back("COM" + std::to_string(port));
			}
			temp_ptr = strchr(ptr, 0);
			dwChars -= (DWORD)((temp_ptr - ptr) / sizeof(TCHAR) + 1);
			ptr = temp_ptr + 1;
		}
		return comPorts_;
#else
		return std::vector<string>();
#endif
	}

	void saveArchive(std::string path)
	{
		properties->saveArchive(path);
		toggleBaseWindow(true);
	}

	void loadArchive(std::string path)
	{
		properties->restoreArchive(path);
		toggleBaseWindow(true);
	}

	void loadScriptFromFile(std::string path)
	{
		double time0 = reactor->getCurrentTime();
		std::ifstream ifs;
		if (path.length())
		{
			ifs.open(path);

			if (!ifs)
			{
				std::cerr << "Error opening input file: " << path << std::endl;
				return;
			}
			std::istream &is = static_cast<std::istream &>(ifs);

			Command cmd;

			while (is >> cmd)
			{
				cmd.timed += time0;
				cout << cmd;
				reactor->scriptCommands.push_back(cmd);
			}

			MessageDialog *msg = new MessageDialog(this, MessageDialog::Type::Warning, "Load script", "Loaded.");
			msg->setPosition(Vector2i((this->size().x() - msg->size().x()) / 2, (this->size().y() - msg->size().y()) / 2));
			msg->setCallback([this](int /*choice*/)
							 { toggleBaseWindow(true); });
		}
		else
		{
			MessageDialog *msg = new MessageDialog(this, MessageDialog::Type::Warning, "Load script", "Bad file name.");
			msg->setPosition(Vector2i((this->size().x() - msg->size().x()) / 2, (this->size().y() - msg->size().y()) / 2));
			msg->setCallback([this](int /*choice*/)
							 { toggleBaseWindow(true); });
		}
	}

	void updateSettings(bool updateReactor = true)
	{
		curveFillBox->setChecked(properties->curveFill);
		curveFillBox->callback()(properties->curveFill);
		avoidPeriodScramBox->setChecked(properties->avoidPeriodScram);
		avoidPeriodScramBox->callback()(properties->avoidPeriodScram);
		for (int i = 0; i < 6; i++)
		{
			delayedGroupBoxes[i]->setValue(properties->betas[i]);
			delayedGroupBoxes[6 + i]->setValue(properties->lambdas[i]);
			delayedGroupsEnabledBoxes[i]->setChecked(properties->groupsEnabled[i]);
			delayedGroupsEnabledBoxes[i]->callback()(properties->groupsEnabled[i]);
		}
		coreVolumeBox->setValue(properties->coreVolume * 1e3); // Convert from m3 to L
		alpha0Box->setValue(properties->alpha0);
		alphaPeakBox->setValue(properties->alphaAtT1);
		alphaSlopeBox->setValue((float)properties->alphaK);
		tempPeakBox->setValue(properties->alphaT1);
		displayBox->setValue(properties->displayTime);
		excessReactivityBox->setValue(properties->excessReactivity);
		fissionProductsBox->setChecked(properties->fissionPoisons);
		fissionProductsBox->callback()(properties->fissionPoisons);
		graphSizeBox->setValue((int)(properties->graphSize * 100));
		sourceActivityBox->setValue(properties->neutronSourceActivity);
		neutronSourceCB->setChecked(properties->neutronSourceInserted);
		neutronSourceCB->callback()(properties->neutronSourceInserted);
		neutronSourceModeBox->setSelectedIndex(properties->ns_mode);
		neutronSourcePeriodBoxes[0]->setValue(properties->ns_squareWave.period);
		neutronSourcePeriodBoxes[1]->setValue(properties->ns_sineMode.period);
		neutronSourcePeriodBoxes[2]->setValue(properties->ns_sawToothMode.period);
		neutronSourceAmplitudeBoxes[0]->setValue(properties->ns_squareWave.amplitude);
		neutronSourceAmplitudeBoxes[1]->setValue(properties->ns_sineMode.amplitude);
		neutronSourceAmplitudeBoxes[2]->setValue(properties->ns_sawToothMode.amplitude);
		for (int i = 0; i < 4; i++)
			neutronSourceSQWBoxes[i]->setValue((int)(properties->ns_squareWave.xIndex[i] * 100));
		neutronSourceSINEModeBox->setSelectedIndex((int)properties->ns_sineMode.mode);
		for (int i = 0; i < 6; i++)
			neutronSourceSAWBoxes[i]->setValue((int)(properties->ns_sawToothMode.xIndex[i] * 100));
		periodLimBox->setValue((float)properties->periodLimit);
		powerLimBox->setValue(properties->powerLimit * 1e-03);
		fuel_tempLimBox->setValue(properties->tempLimit);
		water_tempLimBox->setValue(properties->waterTempLimit);
		// water_levelLimBox->setValue(properties->waterLevelLimit);
		for (int i = 0; i < 4; i++)
		{
			bool value = false;
			switch (i)
			{
			case 0:
				value = properties->periodScram;
				break;
			case 1:
				value = properties->powerScram;
				break;
			case 2:
				value = properties->tempScram;
				break;
			case 3:
				value = properties->waterTempScram;
				break;
			case 4:
				value = properties->waterLevelScram;
				break;
			}
			scramEnabledBoxes[i]->setChecked(value);
			scramEnabledBoxes[i]->callback()(value);
		}

		promptNeutronLifetimeBox->setValue(properties->promptNeutronLifetime);
		for (int i = 0; i < 2; i++)
		{
			reactivityLimitBox[i]->setValue(properties->reactivityGraphLimits[i]);
			temperatureLimitBox[i]->setValue(properties->temperatureGraphLimits[i]);
		}
		rodReactivityBox->setChecked(properties->rodReactivityPlot);
		rodReactivityBox->callback()(properties->rodReactivityPlot);
		for (int i = 0; i < 3; i++)
		{
			rodStepsBox[i]->setValue((int)properties->rodSettings[i].rodSteps);
			rodWorthBox[i]->setValue(properties->rodSettings[i].rodWorth);
			rodSpeedBox[i]->setValue(properties->rodSettings[i].rodSpeed);
			for (int j = 0; j < 2; j++)
			{
				rodCurveSliders[i * 2 + j]->setValue(properties->rodSettings[i].rodCurve[j]);
				rodCurveSliders[i * 2 + j]->finalCallback()(properties->rodSettings[i].rodCurve[j]);
				rodCurves[i]->setParameter(j * 2, properties->rodSettings[i].rodCurve[j]);
			}
		}
		periodBoxes[0]->setValue(properties->squareWave.period);
		amplitudeBoxes[0]->setValue(properties->squareWave.amplitude);
		for (int i = 0; i < 4; i++)
			squareWaveBoxes[i]->setValue((int)(properties->squareWave.xIndex[i] * 100));
		periodBoxes[1]->setValue(properties->sineMode.period);
		amplitudeBoxes[1]->setValue(properties->sineMode.amplitude);
		sineModeBox->setSelectedIndex(properties->sineMode.mode);
		periodBoxes[2]->setValue(properties->sawToothMode.period);
		amplitudeBoxes[2]->setValue(properties->sawToothMode.amplitude);
		for (int i = 0; i < 6; i++)
			sawToothBoxes[i]->setValue((int)(properties->sawToothMode.xIndex[i] * 100));
		keepCurrentPowerBox->setChecked(properties->steadyCurrentPower);
		keepCurrentPowerBox->callback()(properties->steadyCurrentPower);
		steadyPowerBox->setValue(properties->steadyGoalPower);
		automaticMarginBox->setValue(properties->steadyMargin * 100);
		tempEffectsBox->setChecked(properties->temperatureEffects);
		tempEffectsBox->callback()(properties->temperatureEffects);
		cooling->setChecked(properties->waterCooling);
		cooling->callback()(properties->waterCooling);
		coolingPowerBox->setValue(properties->waterCoolingPower);
		waterVolumeInput->setValue(properties->waterVolume);
		allRodsBox->setChecked(properties->allRodsAtOnce);
		allRodsBox->callback()(properties->allRodsAtOnce);
		logScaleBox->setChecked(properties->yAxisLog);
		logScaleBox->callback()(properties->yAxisLog);
		autoScramBox->setChecked(properties->automaticPulseScram);
		autoScramBox->callback()(properties->automaticPulseScram);
		hardcoreBox->setChecked(properties->reactivityHardcore);
		hardcoreBox->callback()(properties->reactivityHardcore);
		squareWaveSpeedBox->setChecked(properties->squareWaveUsesRodSpeed);
		squareWaveSpeedBox->callback()(properties->squareWaveUsesRodSpeed);

		if (updateReactor)
			reactor->setProperties(properties);
	}

	bool prevToggle;
	void toggleBaseWindow(bool value)
	{
		if (!value && baseWindow->enabled())
			prevToggle = (reactor->getSpeedFactor() != 0.);
		baseWindow->setEnabled(value);
		baseWindow->setFocused(value);
		playPauseSimulation(value ? prevToggle : value);
	}



int runSimulatorGuiApp(int argc, char **argv)
{
	try
	{
#if defined(_WIN32)
		ShowWindow(GetConsoleWindow(), SW_HIDE);
#endif
		nanogui::init();
		{
			nanogui::ref<SimulatorGUI> app = new SimulatorGUI();
			app->handleDebugChanged();
			app->drawAll();
			app->setVisible(true);
			if (argc == 2)
				app->startScript = argv[1];
			nanogui::mainloop(10);
		}

		nanogui::shutdown();
	}
	catch (const std::runtime_error &e)
	{
		std::string error_msg = std::string("Caught a fatal error: ") + std::string(e.what());
#if defined(_WIN32)
		MessageBoxA(nullptr, error_msg.c_str(), NULL, MB_ICONERROR | MB_OK);
#else
		std::cerr << error_msg << endl;
#endif
		return -1;
	}

	return 0;
}
