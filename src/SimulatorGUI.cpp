/*
	SimulatorGUI.cpp draws the main
	GUI screen
*/
#define NOMINMAX
#define SCROLL_BAR_THICKNESS 12
#define SCROLL_BAR_ROUND .8f

#include <nanogui/common.h>
#include "../include/CustomGraph.h"
#include "../include/CustomWindow.h"
#include "CustomLabel.h"
#include "CustomWidget.h"
#include <nanogui/screen.h>
#include <nanogui/window.h>
#include <nanogui/layout.h>
#include <nanogui/label.h>
#include <nanogui/checkbox.h>
#include <nanogui/button.h>
#include <nanogui/toolbutton.h>
#include <nanogui/popupbutton.h>
#include <nanogui/combobox.h>
#include <nanogui/progressbar.h>
#include <nanogui/entypo.h>
#include <nanogui/messagedialog.h>
#include <nanogui/textbox.h>
#include <nanogui/slider.h>
#include <nanogui/imagepanel.h>
#include <nanogui/imageview.h>
#include <nanogui/vscrollpanel.h>
#include <nanogui/colorwheel.h>
#include <nanogui/graph.h>
#include <nanogui/tabwidget.h>
#include <Simulator.h>
#include "../include/DataDisplay.h"
#include "../include/pieChart.h"
#include "../include/controlRodDisplay.h"
#include "../include/ReactivityDisplay.h"
#include "../include/SliderCheckBox.h"
#include "../include/IntervalSlider.h"
#if defined(_WIN32)
#include <windows.h>
#endif
#include <nanogui/glutil.h>
#include <string>
#include <nanogui/nanogui.h>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <math.h>
#include "../include/Settings.h"
#include "../include/SerialClass.h"
#include "../include/fileDialog.h"
#include "../include/RelativeGridLayout.h"
#include "../include/Plot.h"
#include <Icon.h>
#include "../include/util.h"
#include <CustomTabWidget.h>
#include "../include/UiConfigProvider.h"
#include "../include/SimulatorGuiUiHelpers.h"
#include "../include/SimulatorGuiApp.h"
#include <memory>

/* Resolution formats supported:
 *	HD 720	(1280 x 720)
 *	WXGA	(1280 x 768)
 *	laptop	(1366 x 768)
 *	SXGA	(1280 x 1024)
 *	WXGA	(1280 x 800)
 *	better	(1200+ x 700+)
 */
#define WINDOW_DEFAULT_WIDTH 1280 // HD 720p width
#define WINDOW_DEFAULT_HEIGHT 720 // HD 720p height

#define WINDOW_ICON_NUM 7 // Number of icon formats

// SIMULATOR VERSION
// major.minor.revision.build
#define VERSION_MAJOR 1
#define VERSION_MINOR 4
#define VERSION_REVISION 1
#define VERSION_BUILD 0 // Setting this to 0 doesn't display the build number

// RECIEVING
#define BOX_ID "IJS_F8_BOX3"
#define ENABLE_SAFETY_BTN 8
#define UP_SAFETY_BTN 16
#define DOWN_SAFETY_BTN 32
#define ENABLE_SHIM_BTN 64
#define UP_SHIM_BTN 128
#define DOWN_SHIM_BTN 256
#define ENABLE_REG_BTN 512
#define UP_REG_BTN 1024
#define DOWN_REG_BTN 2048
#define FIRE_BTN 4096
#define SCRAM_BTN 8192

// SENDING
#define SCRAM_PER 2
#define SCRAM_FT 4
#define SCRAM_WT 8
#define SCRAM_POW 16
#define SCRAM_MAN 32
#define FIRE_LED_B 64
#define ROD_SAFETY_ENBL 128
#define ROD_SAFETY_UP 256
#define ROD_SAFETY_DOWN 512
#define ROD_REG_ENBL 1024
#define ROD_REG_UP 2048
#define ROD_REG_DOWN 4096
#define ROD_SHIM_ENBL 8192
#define ROD_SHIM_UP 16384
#define ROD_SHIM_DOWN 32768

#define RESET_ALARM_KEY 8128

#define SIM_TIME_FACTOR_NUMBER 21

#define SCI_NUMBER_FORMAT "[0-9]*\\.?[0-9]+([eE][-+]?[0-9]+)?"
#define SCI_NUMBER_FORMAT_NEG "[-]?[0-9]*\\.?[0-9]+([eE][-+]?[0-9]+)?"

using namespace nanogui;
using std::cerr;
using std::cout;
using std::endl;
using std::max;
using std::string;
using std::to_string;

class SimulatorGUI : public nanogui::Screen
{
private:
	const string box_auth = BOX_ID;
	pair<bool, bool> isZero = pair<bool, bool>(false, false);
	double simulationTimes[SIM_TIME_FACTOR_NUMBER] = {0.01, 0.05, 0.1, 0.2, 0.3, 0.4, 0.5, 0.75, 1., 1.5, 2., 4., 5., 7.5, 10., 15., 20., 50., 100., 250., 500.};
	const std::vector<string> modes = {
		"Manual",
		//"Square wave","Sine wave","Saw tooth",
		"Automatic"
		//, "Pulse"
	};
	const std::vector<string> ns_modes = {"Constant", "Square wave", "Sine wave", "Saw tooth"};
	const std::vector<string> sineModes = {"Normal", "Quadratic"};
	string sqw_settingNames[4] = {"Wave up start: ", "Wave up end: ", "Wave down start: ", "Wave down end: "};
	string saw_settingNames[6] = {"Tooth up start: ", "Tooth up peak: ", "Tooth up end: ", "Tooth down start: ", "Tooth down peak: ", "Tooth down end: "};
	double lastBoxCheck;
	bool layoutStart = false;
	float fpsSum = 0.f;
	char fpsCount = 0;
	const std::string degCelsiusUnit = std::string(utf8(0xBA).data()) + "C";
	const std::string alpha = std::string(utf8(0x3B1).data());
	double alphaX[3] = {0., 0., 0.};
	float alphaY[3] = {0.f, 0.f, 0.f};

public:
	Simulator::PulseData lastPulseData;
	bool pulsePerformed = false;
	double viewStart = -1.;
	double timeAtLastChange = 0.;
	string startScript = "";

	Settings *properties;

#if defined(_WIN32)
	Serial *theBox;
#endif
	Simulator *reactor;
	CustomGraph *canvas;
	CustomGraph *delayedGroupsGraph;
	CustomGraph *pulseGraph;
	CustomGraph *sourceGraph;
	BoxLayout *layout;
	CustomWindow *baseWindow;
	RelativeGridLayout *relativeLayout; // layout for the main window
	CustomLabel *fpsLabel;
	Plot *reactivityPlot;
	Plot *rodReactivityPlot;
	Plot *powerPlot;
	Plot *temperaturePlot;
	Plot *delayedGroups[6];
	Plot *pulsePlots[4];
	ToolButton *slowDown;
	ToolButton *playPause;
	ToolButton *speedUp;
	size_t selectedTime = 8;
	BezierCurve *rodCurves[NUMBER_OF_CONTROL_RODS];
	Plot *rodDerivatives[NUMBER_OF_CONTROL_RODS];
	CustomTabWidget *tabControl;

	CustomWidget *sourceSettings;
	Plot *neutronSourcePlot;
	Plot *neutronSourceTracker;
	ComboBox *neutronSourceModeBox;
	ComboBox *neutronSourceSINEModeBox;
	FloatBox<float> *neutronSourcePeriodBoxes[3];
	FloatBox<float> *neutronSourceAmplitudeBoxes[3];
	IntBox<int> *neutronSourceSQWBoxes[4];
	IntBox<int> *neutronSourceSAWBoxes[6];

	CustomWidget *displayPanel1;
	CustomWidget *displayPanel2;
	ControlRodDisplay *rodDisplay;
	PeriodDisplay *periodDisplay;
	ComboBox *rodMode;
	IntBox<int> *rodBox[NUMBER_OF_CONTROL_RODS];
	SliderCheckBox *neutronSourceCB;
	SliderCheckBox *cooling;

	IntBox<int> *graphSizeBox;
	SliderCheckBox *curveFillBox;
	SliderCheckBox *rodReactivityBox;
	FloatBox<float> *reactivityLimitBox[2];
	FloatBox<float> *temperatureLimitBox[2];
	FloatBox<float> *displayBox;
	SliderCheckBox *logScaleBox;
	SliderCheckBox *hardcoreBox;
	IntervalSlider *displayTimeSlider;
	SliderCheckBox *timeLockedBox;

	FloatBox<double> *delayedGroupBoxes[12];
	SliderCheckBox *delayedGroupsEnabledBoxes[6];
	FloatBox<double> *coreVolumeBox;
	FloatBox<double> *waterVolumeInput;
	SliderCheckBox *tempEffectsBox;
	SliderCheckBox *fissionProductsBox;
	FloatBox<float> *excessReactivityBox;
	FloatBox<double> *sourceActivityBox;
	FloatBox<double> *coolingPowerBox;
	FloatBox<double> *promptNeutronLifetimeBox;
	Plot *alphaPlot;
	const size_t sigmaPoints = 3;
	FloatBox<float> *alpha0Box;
	FloatBox<float> *alphaPeakBox;
	FloatBox<float> *tempPeakBox;
	FloatBox<float> *alphaSlopeBox;

	IntBox<int> *rodStepsBox[NUMBER_OF_CONTROL_RODS];
	FloatBox<float> *rodWorthBox[NUMBER_OF_CONTROL_RODS];
	FloatBox<float> *rodSpeedBox[NUMBER_OF_CONTROL_RODS];
	Slider *rodCurveSliders[NUMBER_OF_CONTROL_RODS * 2];

	// Rod modes
	FloatBox<float> *periodBoxes[3];
	FloatBox<float> *amplitudeBoxes[3];
	IntBox<int> *squareWaveBoxes[4];
	SliderCheckBox *squareWaveSpeedBox;
	ComboBox *sineModeBox;
	IntBox<int> *sawToothBoxes[6];
	SliderCheckBox *keepCurrentPowerBox;
	FloatBox<double> *steadyPowerBox;
	SliderCheckBox *avoidPeriodScramBox;
	FloatBox<float> *automaticMarginBox;

	// Neutron source simulation
	FloatBox<float> *ns_periodBoxes[3];
	FloatBox<float> *ns_amplitudeBoxes[3];
	IntBox<int> *ns_squareWaveBoxes[4];
	ComboBox *ns_sineModeBox;
	IntBox<int> *ns_sawToothBoxes[6];

	SliderCheckBox *scramEnabledBoxes[5];
	IntBox<float> *periodLimBox;
	FloatBox<double> *powerLimBox;
	FloatBox<float> *fuel_tempLimBox;
	FloatBox<float> *water_tempLimBox;
	FloatBox<float> *water_levelLimBox;
	SliderCheckBox *allRodsBox;
	SliderCheckBox *autoScramBox;

	IntervalSlider *pulseTimer;

	CustomLabel *userScram;
	CustomLabel *powerScram;
	CustomLabel *fuelTemperatureScram;
	CustomLabel *waterTemperatureScram;
	CustomLabel *waterLevelScram;
	CustomLabel *periodScram;

	CustomLabel *timeLabel;
	CustomLabel *simStatusLabel;
	CustomLabel *simFactorLabel;

	CustomLabel *pulseLabels[4];
	CustomLabel *pulseDisplayLabels[4];
	CustomLabel *standInCover;

	DataDisplay<double> *powerShow;
	DataDisplay<float> *reactivityShow;
	DataDisplay<float> *rodReactivityShow;
	DataDisplay<float> *temperatureShow;
	DataDisplay<double> *periodShow;
	DataDisplay<double> *waterTemperatureShow;
	DataDisplay<double> *waterLevelShow;

	Plot *operationModes[3];
	Plot *operationModesTrackers[3];

	std::shared_ptr<IUiConfigProvider> uiConfigProvider;
	UiStyleConfig uiStyleConfig;
	UiPanelLayoutConfig uiPanelLayoutConfig;
	Color coolBlue;

	BoxLayout *panelsLayout = nullptr;

	uint16_t LEDstatus = 0;
	bool boxConnected = false;


	// For cleaner code



	// MORE

	// Anti spaghetti machine
#if defined(_WIN32)
	vector<string> comPorts;
	vector<string> lastCOMports;


#endif



	void createMainTab()
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

	void createGraphSettingsTab()
	{
		Widget *graph_controls = tabControl->createTab("Graph controls");
		graph_controls->setId("graph controls");
		RelativeGridLayout *graphControlsLayout = new RelativeGridLayout();
		graphControlsLayout->appendRow(RelativeGridLayout::Size(140.f, RelativeGridLayout::SizeType::Fixed));
		graphControlsLayout->appendRow(RelativeGridLayout::Size(10.f, RelativeGridLayout::SizeType::Fixed));
		graphControlsLayout->appendRow(RelativeGridLayout::Size(2.f, RelativeGridLayout::SizeType::Fixed));
		graphControlsLayout->appendRow(RelativeGridLayout::Size(30.f, RelativeGridLayout::SizeType::Fixed));
		graphControlsLayout->appendRow(RelativeGridLayout::Size(20.f, RelativeGridLayout::SizeType::Fixed));
		graphControlsLayout->appendRow(1.f);
		graphControlsLayout->appendCol(1.f);
		graphControlsLayout->appendCol(1.f);
		graph_controls->setLayout(graphControlsLayout);
		CustomWidget *border = graph_controls->add<CustomWidget>();
		border->setBackgroundColor(coolBlue);
		border->setDrawBackground(true);
		graphControlsLayout->setAnchor(border, RelativeGridLayout::makeAnchor(0, 2, 2, 1));

		// Create a panel for graph size
		Widget *generalLeftPanel = new Widget(graph_controls);
		graphControlsLayout->setAnchor(generalLeftPanel, RelativeGridLayout::makeAnchor(0, 0));
		generalLeftPanel->setLayout(new BoxLayout(Orientation::Vertical, Alignment::Minimum, 10, 10));
		Widget *graphSizePanel = generalLeftPanel->add<Widget>();
		graphSizePanel->setLayout(panelsLayout);

		graphSizePanel->add<Label>("Graph size :", "sans-bold");
		graphSizeBox = graphSizePanel->add<IntBox<int>>((int)(100 * properties->graphSize));
		graphSizeBox->setUnits("%");
		graphSizeBox->setDefaultValue(to_string((int)std::roundf(properties->graphSize * 100)));
		graphSizeBox->setFontSize(16);
		graphSizeBox->setFormat("[0-9]+");
		graphSizeBox->setSpinnable(true);
		graphSizeBox->setMinValue(30);
		graphSizeBox->setMaxValue(70);
		graphSizeBox->setValueIncrement(1);
		graphSizeBox->setCallback([this](int a)
								  {
				if (a > 70) a = 70;
				if (a < 30) a = 30;
				relativeLayout->setRowSize(0, a / 100.f);
				relativeLayout->setRowSize(1, 1.f - a / 100.f);
				performLayout();
				properties->graphSize = a / 100.f; });

		// Create a panel for display time
		Widget *timePanel = generalLeftPanel->add<Widget>();
		timePanel->setLayout(panelsLayout);

		// Create a panel for graph limits
		Widget *reactivityLimitsPanel = generalLeftPanel->add<Widget>();
		reactivityLimitsPanel->setLayout(panelsLayout);

		// Create another panel for graph limits
		Widget *temperatureLimitsPanel = generalLeftPanel->add<Widget>();
		temperatureLimitsPanel->setLayout(panelsLayout);

		// Create a panel for curve fill, rod reactivity line visibility, log scale power and hardcore mode
		Widget *sliderPanel = graph_controls->add<Widget>();
		graphControlsLayout->setAnchor(sliderPanel, RelativeGridLayout::makeAnchor(1, 0, 1, 1, Alignment::Maximum, Alignment::Fill));

		RelativeGridLayout *sliderLayout = new RelativeGridLayout();
		for (int i = 0; i < 4; i++)
			sliderLayout->appendCol((i % 2) ? RelativeGridLayout::Size(10.f, RelativeGridLayout::SizeType::Fixed) : 1.f);
		for (int i = 0; i < 4; i++)
			sliderLayout->appendRow(1.f);
		sliderPanel->setLayout(sliderLayout);

		sliderLayout->setAnchor(sliderPanel->add<Label>("Rod reactivity plot:", "sans-bold"), RelativeGridLayout::makeAnchor(0, 0, 1, 1, Alignment::Minimum, Alignment::Middle));
		rodReactivityBox = sliderPanel->add<SliderCheckBox>();
		sliderLayout->setAnchor(rodReactivityBox, RelativeGridLayout::makeAnchor(2, 0, 1, 1, Alignment::Maximum, Alignment::Middle));
		rodReactivityBox->setFontSize(16);
		rodReactivityBox->setChecked(properties->rodReactivityPlot);
		rodReactivityBox->setCallback([this](bool value)
									  {
				properties->rodReactivityPlot = value;
				rodReactivityPlot->setEnabled(value && !properties->reactivityHardcore); });

		sliderLayout->setAnchor(sliderPanel->add<Label>("Curve fill:", "sans-bold"), RelativeGridLayout::makeAnchor(0, 1, 1, 1, Alignment::Minimum, Alignment::Middle));
		curveFillBox = sliderPanel->add<SliderCheckBox>();
		sliderLayout->setAnchor(curveFillBox, RelativeGridLayout::makeAnchor(2, 1, 1, 1, Alignment::Maximum, Alignment::Middle));
		curveFillBox->setFontSize(16);
		curveFillBox->setChecked(properties->curveFill);
		curveFillBox->setCallback([this](bool value)
								  {
				rodReactivityPlot->setFill(value);
				reactivityPlot->setFill(value);
				powerPlot->setFill(value);
				temperaturePlot->setFill(value);
				properties->curveFill = value; });

		timePanel->add<Label>("Display time :", "sans-bold");
		displayBox = timePanel->add<FloatBox<float>>(properties->displayTime);
		displayBox->setFixedSize(Vector2i(100, 20));
		displayBox->setUnits("s");
		displayBox->setDefaultValue(formatDecimalsDouble((double)properties->displayTime, 1));
		displayBox->setFontSize(16);
		displayBox->setFormat("[0-9]*[.]?[0-9]?");
		displayBox->setSpinnable(true);
		displayBox->setMinMaxValues(0.5f, (float)reactor->getDeleteOldValues());
		displayBox->setValueIncrement(1.f);
		displayBox->setCallback([this](float a)
								{
				properties->displayTime = std::min((float)reactor->getDeleteOldValues(), std::max(a, 0.5f));
				std::string limit = formatDecimalsDouble((double)properties->displayTime, 1) + " seconds ago";
				powerPlot->setLimitOverride(0, limit);
				delayedGroups[0]->setLimitOverride(0, limit); });

		{
			Label *temp = reactivityLimitsPanel->add<Label>("Reactivity graph:  from ", "sans-bold");
			temp->setFixedWidth(160);
			reactivityLimitBox[0] = reactivityLimitsPanel->add<FloatBox<float>>(properties->reactivityGraphLimits[0]);
			reactivityLimitBox[0]->setFixedSize(Vector2i(100, 20));
			reactivityLimitBox[0]->setUnits("pcm");
			reactivityLimitBox[0]->setDefaultValue(to_string(properties->reactivityGraphLimits[0]));
			reactivityLimitBox[0]->setFontSize(16);
			reactivityLimitBox[0]->setFormat("[-]?[0-9]*[.]?[0-9]?");
			reactivityLimitBox[0]->setSpinnable(true);
			reactivityLimitBox[0]->setValueIncrement(1.f);
			reactivityLimitBox[0]->setCallback([this](float a)
											   { properties->reactivityGraphLimits[0] = a; });

			reactivityLimitsPanel->add<Label>(" to ", "sans-bold");
			reactivityLimitBox[1] = reactivityLimitsPanel->add<FloatBox<float>>(properties->reactivityGraphLimits[1]);
			reactivityLimitBox[1]->setFixedSize(Vector2i(100, 20));
			reactivityLimitBox[1]->setUnits("pcm");
			reactivityLimitBox[1]->setDefaultValue(to_string(properties->reactivityGraphLimits[1]));
			reactivityLimitBox[1]->setFontSize(16);
			reactivityLimitBox[1]->setFormat("[-]?[0-9]*[.]?[0-9]?");
			reactivityLimitBox[1]->setSpinnable(true);
			reactivityLimitBox[1]->setValueIncrement(1.f);
			reactivityLimitBox[1]->setCallback([this](float a)
											   { properties->reactivityGraphLimits[1] = a; });
			Button *btn = reactivityLimitsPanel->add<Button>("Reset");
			btn->setCallback([this]()
							 {
					reactivityLimitBox[0]->setValue(reactor->getExcessReactivity() - reactor->getTotalRodWorth());
					reactivityLimitBox[1]->setValue(static_cast<int>(reactor->getExcessReactivity())); });
		}

		{
			// Temperature stuff
			Label *temp = temperatureLimitsPanel->add<Label>("Temperature graph:  from ", "sans-bold");
			temp->setFixedWidth(160);
			temperatureLimitBox[0] = temperatureLimitsPanel->add<FloatBox<float>>(properties->temperatureGraphLimits[0]);
			temperatureLimitBox[0]->setFixedSize(Vector2i(100, 20));
			temperatureLimitBox[0]->setUnits(degCelsiusUnit);
			temperatureLimitBox[0]->setDefaultValue(to_string(properties->temperatureGraphLimits[0]));
			temperatureLimitBox[0]->setFontSize(16);
			temperatureLimitBox[0]->setFormat("[-]?[0-9]*[.]?[0-9]?");
			temperatureLimitBox[0]->setMinValue(0.f);
			temperatureLimitBox[0]->setSpinnable(true);
			temperatureLimitBox[0]->setValueIncrement(1.f);
			temperatureLimitBox[0]->setCallback([this](float a)
												{ properties->temperatureGraphLimits[0] = a; });

			temperatureLimitsPanel->add<Label>(" to ", "sans-bold");
			temperatureLimitBox[1] = temperatureLimitsPanel->add<FloatBox<float>>(properties->temperatureGraphLimits[1]);
			temperatureLimitBox[1]->setFixedSize(Vector2i(100, 20));
			temperatureLimitBox[1]->setUnits(degCelsiusUnit);
			temperatureLimitBox[1]->setDefaultValue(to_string(properties->temperatureGraphLimits[1]));
			temperatureLimitBox[1]->setFontSize(16);
			temperatureLimitBox[1]->setFormat("[-]?[0-9]*[.]?[0-9]?");
			temperatureLimitBox[1]->setMinValue(0.f);
			temperatureLimitBox[1]->setSpinnable(true);
			temperatureLimitBox[1]->setValueIncrement(1.f);
			temperatureLimitBox[1]->setCallback([this](float a)
												{ properties->temperatureGraphLimits[1] = a; });

			Button *btn = temperatureLimitsPanel->add<Button>("Reset");
			btn->setCallback([this]()
							 {
					temperatureLimitBox[0]->setValue(TEMPERATURE_GRAPH_FROM_DEFAULT);
					temperatureLimitBox[1]->setValue(TEMPERATURE_GRAPH_TO_DEFAULT); });
		}

		sliderLayout->setAnchor(sliderPanel->add<Label>("Power log scale:", "sans-bold"), RelativeGridLayout::makeAnchor(0, 2, 1, 1, Alignment::Minimum, Alignment::Middle));
		logScaleBox = sliderPanel->add<SliderCheckBox>();
		sliderLayout->setAnchor(logScaleBox, RelativeGridLayout::makeAnchor(2, 2, 1, 1, Alignment::Maximum, Alignment::Middle));
		logScaleBox->setFontSize(16);
		logScaleBox->setChecked(properties->yAxisLog);
		logScaleBox->setCallback([this](bool value)
								 {
				properties->yAxisLog = value;
				powerPlot->setYlog(value); });

		sliderLayout->setAnchor(sliderPanel->add<Label>("Hide reactivity:", "sans-bold"), RelativeGridLayout::makeAnchor(0, 3, 1, 1, Alignment::Minimum, Alignment::Middle));
		hardcoreBox = sliderPanel->add<SliderCheckBox>();
		sliderLayout->setAnchor(hardcoreBox, RelativeGridLayout::makeAnchor(2, 3, 1, 1, Alignment::Maximum, Alignment::Middle));
		hardcoreBox->setFontSize(16);
		hardcoreBox->setChecked(properties->yAxisLog);
		hardcoreBox->setCallback([this](bool value)
								 {
				properties->reactivityHardcore = value;
				hardcoreMode(value); });

		CustomLabel *timeAdjLabel = graph_controls->add<CustomLabel>("Edit display range", "sans-bold");
		timeAdjLabel->setFontSize(25);
		timeAdjLabel->setPadding(0, 15);
		graphControlsLayout->setAnchor(timeAdjLabel, RelativeGridLayout::makeAnchor(0, 3, 1, 1, Alignment::Minimum, Alignment::Minimum));

		Widget *timeLockPanel = graph_controls->add<Widget>();
		graphControlsLayout->setAnchor(timeLockPanel, RelativeGridLayout::makeAnchor(0, 4, 1, 1, Alignment::Minimum, Alignment::Fill));
		RelativeGridLayout *tlLayout = new RelativeGridLayout();
		tlLayout->appendCol(RelativeGridLayout::Size(120.f, RelativeGridLayout::SizeType::Fixed));
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
		timeLockedBox->setCallback([this](bool value)
								   {
				if (value)
				{
					this->viewingIntervalChanged(true);
				}
				else
				{
					timeAtLastChange = this->reactor->getCurrentTime();
				} });

		displayTimeSlider = new IntervalSlider(graph_controls);
		RelativeGridLayout::Anchor acr = RelativeGridLayout::makeAnchor(0, 5, 2, 1, Alignment::Fill, Alignment::Minimum);
		acr.padding = Vector4i(20, 10, 20, 0);
		graphControlsLayout->setAnchor(displayTimeSlider, acr);
		displayTimeSlider->setHighlightColor(coolBlue);
		displayTimeSlider->setSteps((unsigned int)DELETE_OLD_DATA_TIME_DEFAULT * 1000U);
		displayTimeSlider->setEnabled(true);
		displayTimeSlider->setFixedHeight(25);
		for (int i = 0; i < 2; i++)
			displayTimeSlider->setCallback(i, [this, i](float /*change*/)
										   {
				this->viewingIntervalChanged(i == 0);
				if (i == 0 && !timeLockedBox->checked())
				{
					timeAtLastChange = this->reactor->getCurrentTime();
				} });
		Button *displayResetBtn = new Button(graph_controls, "Reset view to newest");
		RelativeGridLayout::Anchor acr2 = RelativeGridLayout::makeAnchor(1, 3, 1, 1, Alignment::Maximum, Alignment::Middle);
		acr2.padding = Vector4i(0, 0, 10, 0);
		graphControlsLayout->setAnchor(displayResetBtn, acr2);
		displayResetBtn->setCallback([this]() { // reset the view to default
			this->viewStart = -1.;
			timeLockedBox->setChecked(false);
			timeLockedBox->callback()(false);
		});
	}

	void createPhysicsSettingsTab()
	{
		Widget *physics_settings_base = tabControl->createTab("Physics settings", false);
		CustomTabWidget *modeTabs = physics_settings_base->add<CustomTabWidget>();
		modeTabs->header()->setStretch(true);
		modeTabs->header()->setButtonAlignment(NVGalign::NVG_ALIGN_MIDDLE | NVGalign::NVG_ALIGN_CENTER);
		RelativeGridLayout *grid = new RelativeGridLayout();
		grid->appendCol(1.f);
		grid->appendRow(1.f);
		physics_settings_base->setLayout(grid);
		grid->setAnchor(modeTabs, RelativeGridLayout::makeAnchor(0, 0));

		// Create titles, periods and amplitudes
		std::string titles[2] = {"Physics", "Neutron source"};
		// RelativeGridLayout* layouts[2];
		// Widget* tabs[2];

		Widget *physics_settings = modeTabs->createTab("Physics");
		physics_settings->setId("Physics tab");
		RelativeGridLayout *physicsLayout = new RelativeGridLayout();
		physicsLayout->appendRow(RelativeGridLayout::Size(15.f, RelativeGridLayout::SizeType::Fixed));
		physicsLayout->appendRow(RelativeGridLayout::Size(4 * 46.f, RelativeGridLayout::SizeType::Fixed));
		physicsLayout->appendRow(RelativeGridLayout::Size(15.f, RelativeGridLayout::SizeType::Fixed));
		physicsLayout->appendRow(1.f);
		physicsLayout->appendRow(RelativeGridLayout::Size(15.f, RelativeGridLayout::SizeType::Fixed));
		physicsLayout->appendCol(RelativeGridLayout::Size(10.f, RelativeGridLayout::SizeType::Fixed));
		physicsLayout->appendCol(1.f);
		physicsLayout->appendCol(1.f);
		physicsLayout->appendCol(RelativeGridLayout::Size(10.f + SCROLL_BAR_THICKNESS, RelativeGridLayout::SizeType::Fixed));
		physics_settings->setLayout(physicsLayout);
		CustomWidget *delayedPanel = physics_settings->add<CustomWidget>();
		physicsLayout->setAnchor(delayedPanel, RelativeGridLayout::makeAnchor(1, 1, 2, 1));
		delayedPanel->setDrawBackground(true);
		delayedPanel->setBackgroundColor(Color(40, 40, 40, 255));
		RelativeGridLayout *relPhysics = new RelativeGridLayout();
		for (int i = 1; i < 10; i++)
			relPhysics->appendRow(RelativeGridLayout::Size((i % 2) ? 1.f : 45.f, RelativeGridLayout::SizeType::Fixed));
		relPhysics->appendCol(RelativeGridLayout::Size(1.f, RelativeGridLayout::SizeType::Fixed));
		relPhysics->appendCol(RelativeGridLayout::Size(100.f, RelativeGridLayout::SizeType::Fixed));
		for (int i = 0; i < 13; i++)
			relPhysics->appendCol((i % 2) ? 1.f : RelativeGridLayout::Size(1.f, RelativeGridLayout::SizeType::Fixed));
		delayedPanel->setLayout(relPhysics);

		// Source settings tab
		sourceSettings = modeTabs->createTab("Neutron source");
		sourceSettings->setId("Neutron source tab");
		RelativeGridLayout *sourceLayout = new RelativeGridLayout();
		sourceLayout->appendRow(RelativeGridLayout::Size(15.f, RelativeGridLayout::SizeType::Fixed));
		for (int j = 0; j < 11; j++)
		{
			sourceLayout->appendRow(RelativeGridLayout::Size((j == 2) ? 1.f : 30.f, RelativeGridLayout::SizeType::Fixed));
		}
		sourceLayout->appendRow(1.f);
		sourceLayout->appendRow(RelativeGridLayout::Size(15.f, RelativeGridLayout::SizeType::Fixed));
		sourceLayout->appendCol(1.f);
		sourceLayout->appendCol(RelativeGridLayout::Size(15.f, RelativeGridLayout::SizeType::Fixed));
		sourceLayout->appendCol(RelativeGridLayout::Size(350.f, RelativeGridLayout::SizeType::Fixed));
		sourceLayout->appendCol(RelativeGridLayout::Size(SCROLL_BAR_THICKNESS, RelativeGridLayout::SizeType::Fixed));
		sourceSettings->setLayout(sourceLayout);
		// source seperator
		CustomWidget *ns_border = sourceSettings->add<CustomWidget>();
		ns_border->setDrawBackground(true);
		ns_border->setBackgroundColor(coolBlue);
		sourceLayout->setAnchor(ns_border, RelativeGridLayout::makeAnchor(1, 3, 2));
		// Source base
		sourceActivityBox = makeSettingLabel<FloatBox<double>>(sourceSettings, "Base intensity: ", 100, properties->neutronSourceActivity);
		sourceLayout->setAnchor(sourceActivityBox->parent(), RelativeGridLayout::makeAnchor(2, 1, 1, 1, Alignment::Minimum, Alignment::Middle));
		sourceActivityBox->setAlignment(TextBox::Alignment::Left);
		sourceActivityBox->setFixedSize(Vector2i(125, 30));
		sourceActivityBox->setSpinnable(true);
		sourceActivityBox->setValueIncrement(5e4);
		sourceActivityBox->setDefaultValue(std::to_string(sourceActivityBox->value()));
		sourceActivityBox->setMinValue(0.);
		sourceActivityBox->setFormat(SCI_NUMBER_FORMAT);
		sourceActivityBox->setUnits("n/s");
		sourceActivityBox->setCallback([this](double change)
									   {
				properties->neutronSourceActivity = change;
				reactor->setNeutronSourceActivity(change); });
		neutronSourceModeBox = makeSettingLabel<ComboBox>(sourceSettings, "Source type:", 100, ns_modes);
		sourceLayout->setAnchor(neutronSourceModeBox->parent(), RelativeGridLayout::makeAnchor(2, 2, 1, 1, Alignment::Minimum, Alignment::Middle));
		neutronSourceModeBox->setFixedWidth(125);
		neutronSourceModeBox->setCallback([this](int change)
										  {
				properties->ns_mode = (char)change;
				reactor->setNeutronSourceMode((SimulationModes)change);
				updateNeutronSourceTab(); });
		neutronSourceModeBox->setSelectedIndex(0);
		// Source graph
		sourceGraph = sourceSettings->add<CustomGraph>(2, "Neutron source simulation");
		sourceGraph->setDrawBackground(true);
		sourceGraph->setBackgroundColor(Color(32, 255));
		sourceLayout->setAnchor(sourceGraph, RelativeGridLayout::makeAnchor(0, 0, 1, 14, Alignment::Fill, Alignment::Fill));
		sourceGraph->setPadding(90.f, 20.f, 10.f, 70.f);
		sourceGraph->setPlotBackgroundColor(Color(60, 255));
		sourceGraph->setPlotGridColor(Color(177, 255));
		sourceGraph->setPlotBorderColor(Color(200, 255));
		PeriodicalMode *per;
		// source general
		for (int i = 0; i < 3; i++)
		{
			per = reactor->getSourceModeClass((SimulationModes)(i + 1));
			neutronSourcePeriodBoxes[i] = makeSettingLabel<FloatBox<float>>(sourceSettings, "Period: ", 100, per->getPeriod());
			sourceLayout->setAnchor(neutronSourcePeriodBoxes[i]->parent(), RelativeGridLayout::makeAnchor(2, 4, 1, 1, Alignment::Minimum, Alignment::Middle));
			neutronSourcePeriodBoxes[i]->setAlignment(TextBox::Alignment::Left);
			neutronSourcePeriodBoxes[i]->setFixedWidth(100);
			neutronSourcePeriodBoxes[i]->setFormat(SCI_NUMBER_FORMAT);
			neutronSourcePeriodBoxes[i]->setUnits("s");
			neutronSourcePeriodBoxes[i]->setMinValue(0.1f);
			neutronSourcePeriodBoxes[i]->setValueIncrement(0.1f);
			neutronSourcePeriodBoxes[i]->setSpinnable(true);
			neutronSourcePeriodBoxes[i]->setDefaultValue(formatDecimalsDouble(per->getPeriod(), 1));
			neutronSourcePeriodBoxes[i]->setCallback([this, i, per](float change)
													 {
					switch (i)
					{
					case 0: properties->squareWave.period = change; break;
					case 1: properties->sineMode.period = change; break;
					case 2: properties->sawToothMode.period = change; break;
					}
					per->setPeriod(change);
					updateNeutronSourceTab(); });
			neutronSourceAmplitudeBoxes[i] = makeSettingLabel<FloatBox<float>>(sourceSettings, "Intensity: ", 100, per->getAmplitude());
			sourceLayout->setAnchor(neutronSourceAmplitudeBoxes[i]->parent(), RelativeGridLayout::makeAnchor(2, 5));
			neutronSourceAmplitudeBoxes[i]->setAlignment(TextBox::Alignment::Left);
			neutronSourceAmplitudeBoxes[i]->setFixedWidth(100);
			neutronSourceAmplitudeBoxes[i]->setFormat(SCI_NUMBER_FORMAT);
			neutronSourceAmplitudeBoxes[i]->setUnits("n/s");
			neutronSourceAmplitudeBoxes[i]->setMinValue(0.f);
			neutronSourceAmplitudeBoxes[i]->setValueIncrement(1e4);
			neutronSourceAmplitudeBoxes[i]->setSpinnable(true);
			neutronSourceAmplitudeBoxes[i]->setDefaultValue(to_string(per->getAmplitude()));
			neutronSourceAmplitudeBoxes[i]->setCallback([per, i, this](float change)
														{
					switch (i)
					{
					case 0: properties->squareWave.amplitude = change; break;
					case 1: properties->sineMode.amplitude = change; break;
					case 2: properties->sawToothMode.amplitude = change; break;
					}
					per->setAmplitude(change);
					updateNeutronSourceTab(); });
		}
		// source SQW
		float sqPeriod = properties->ns_squareWave.period;
		for (int sqw = 0; sqw < 4; sqw++)
		{
			neutronSourceSQWBoxes[sqw] = makeSimulationSetting(sourceSettings, (int)roundf(properties->ns_squareWave.xIndex[sqw] * 100), sqw_settingNames[sqw]);
			sourceLayout->setAnchor(neutronSourceSQWBoxes[sqw]->parent(), RelativeGridLayout::makeAnchor(2, 6 + sqw));
			neutronSourceSQWBoxes[sqw]->setCallback([this, sqw, sqPeriod](int change)
													{
					float val = change / 100.f;
					reactor->source_sqw->xIndex[sqw] = val;
					properties->squareWave.xIndex[sqw] = val;
					if (sqw != 3)
					{
						neutronSourceSQWBoxes[sqw + 1]->setMinValue(change);
						if (neutronSourceSQWBoxes[sqw + 1]->value() < change) { squareWaveBoxes[sqw + 1]->setValue(change); }
						else { updateNeutronSourceTab(); }
					}
					else { updateNeutronSourceTab(); } });
		}
		// source sine
		neutronSourceSINEModeBox = makeSettingLabel<ComboBox>(sourceSettings, "Sine type: ", 100, sineModes);
		sourceLayout->setAnchor(neutronSourceSINEModeBox->parent(), RelativeGridLayout::makeAnchor(2, 6));
		neutronSourceSINEModeBox->setFixedWidth(125);
		neutronSourceSINEModeBox->setCallback([this](int change)
											  {
				properties->ns_sineMode.mode = (Settings::SineSettings::SineMode)change;
				reactor->source_sinMode->mode = (Sine::SineMode)change;
				updateNeutronSourceTab(); });
		// source saw tooth
		float stPeriod = properties->ns_sawToothMode.period;
		for (int saw = 0; saw < 6; saw++)
		{
			neutronSourceSAWBoxes[saw] = makeSimulationSetting(sourceSettings, (int)roundf(properties->sawToothMode.xIndex[saw] * 100), saw_settingNames[saw]);
			sourceLayout->setAnchor(neutronSourceSAWBoxes[saw]->parent(), RelativeGridLayout::makeAnchor(2, 6 + saw));
			neutronSourceSAWBoxes[saw]->setCallback([this, stPeriod, saw](int change)
													{
					float val = change / 100.f;
					properties->ns_sawToothMode.xIndex[saw] = val;
					reactor->source_saw->xIndex[saw] = val;
					if (saw < 5)
					{
						neutronSourceSAWBoxes[saw + 1]->setMinValue(change);
						if (neutronSourceSAWBoxes[saw + 1]->value() < change) { neutronSourceSAWBoxes[saw + 1]->setValue(change); }
						else { updateNeutronSourceTab(); }
					}
					else { updateNeutronSourceTab(); } });
		}
		updateNeutronSourceTab();

		/* PHYSICS */
		// Texts and borders
		for (int i = 0; i < 6; i++)
		{
			Label *textLabel = delayedPanel->add<Label>("Group " + std::to_string(i + 1) + ((i == 5 || i == 0) ? (i == 5 ? " (fastest)" : " (slowest)") : ""), "sans-bold");
			relPhysics->setAnchor(textLabel, RelativeGridLayout::makeAnchor((i + 1) * 2 + 1, 1, 1, 1, Alignment::Middle, Alignment::Middle));
		}
		CustomWidget *border;
		for (int i = 0; i < 8; i++)
		{
			border = delayedPanel->add<CustomWidget>();
			border->setDrawBackground(true);
			border->setBackgroundColor(coolBlue);
			relPhysics->setAnchor(border, RelativeGridLayout::makeAnchor(i * 2, 0, 1, 9));
		}
		CustomLabel *textLabel;
		std::string rowText[3] = {"Beta(i):", "Lambda(i):", "Enabled:"};
		for (int i = 0; i < 3; i++)
		{
			textLabel = delayedPanel->add<CustomLabel>(rowText[i], "sans-bold");
			textLabel->setPadding(0, 7.f);
			relPhysics->setAnchor(textLabel, RelativeGridLayout::makeAnchor(0, 2 * (i + 1) + 1, 1, 1, Alignment::Minimum, Alignment::Middle));
		}

		// Data inputs
		for (int row = 0; row < 4; row++)
		{
			border = delayedPanel->add<CustomWidget>();
			border->setDrawBackground(true);
			border->setBackgroundColor(coolBlue);
			relPhysics->setAnchor(border, RelativeGridLayout::makeAnchor(0, 2 * row, 15));
			switch (row)
			{
			case 2:
			{
				for (int i = 0; i < 6; i++)
				{
					delayedGroupsEnabledBoxes[i] = delayedPanel->add<SliderCheckBox>();
					delayedGroupsEnabledBoxes[i]->setChecked(properties->groupsEnabled[i]);
					relPhysics->setAnchor(delayedGroupsEnabledBoxes[i], RelativeGridLayout::makeAnchor(2 * (i + 1) + 1, 7, 1, 1, Alignment::Middle, Alignment::Middle));
					delayedGroupsEnabledBoxes[i]->setCallback([this, i](bool change)
															  {
							properties->groupsEnabled[i] = change;
							reactor->setDelayedGroupEnabled(i, change); });
				}
				break;
			}
			case 3:
			{
				border = delayedPanel->add<CustomWidget>();
				border->setDrawBackground(true);
				border->setBackgroundColor(coolBlue);
				relPhysics->setAnchor(border, RelativeGridLayout::makeAnchor(0, 2 * (row + 1), 15));
				continue;
				break;
			}
			default:
			{
				for (int i = 0; i < 6; i++)
				{
					size_t index = 6 * row + i;
					delayedGroupBoxes[index] = delayedPanel->add<FloatBox<double>>(row ? properties->lambdas[i] : properties->betas[i]);
					relPhysics->setAnchor(delayedGroupBoxes[index], RelativeGridLayout::makeAnchor(2 * (i + 1) + 1, 2 * (row + 1) + 1, 1, 1, Alignment::Middle, Alignment::Middle));
					delayedGroupBoxes[index]->setFixedSize(Vector2i(130, 30));
					delayedGroupBoxes[index]->setSpinnable(true);
					delayedGroupBoxes[index]->setValueIncrement(0.00001f);
					delayedGroupBoxes[index]->setDefaultValue(std::to_string(delayedGroupBoxes[index]->value()));
					delayedGroupBoxes[index]->setMinValue(0.0000001f);
					delayedGroupBoxes[index]->setFormat(SCI_NUMBER_FORMAT);
					delayedGroupBoxes[index]->setCallback([this, row, i](double change)
														  {
							if (row)
							{
								properties->lambdas[i] = change;
								reactor->setDelayedGroupDecay(i, change);
							}
							else
							{
								properties->betas[i] = change;
								reactor->setDelayedGroupFraction(i, change);
							} });
				}
			}
			}
		}

		// Create panel for lower left settings
		Widget *settingsVert = physics_settings->add<Widget>();
		physicsLayout->setAnchor(settingsVert, RelativeGridLayout::makeAnchor(1, 3, 1, 1));
		settingsVert->setLayout(new BoxLayout(Orientation::Vertical, Alignment::Fill, 0, 15));

		// Core volume
		Widget *corePanel = settingsVert->add<Widget>();

		corePanel->setLayout(panelsLayout);
		corePanel->add<Label>("Core volume: ", "sans-bold");
		coreVolumeBox = corePanel->add<FloatBox<double>>(properties->coreVolume * 1e3);
		coreVolumeBox->setAlignment(TextBox::Alignment::Left);
		coreVolumeBox->setFixedSize(Vector2i(180, 30));
		coreVolumeBox->setSpinnable(true);
		coreVolumeBox->setValueIncrement(1.);
		coreVolumeBox->setDefaultValue(std::to_string(coreVolumeBox->value()));
		coreVolumeBox->setMinValue(1.);
		coreVolumeBox->setFormat(SCI_NUMBER_FORMAT);
		coreVolumeBox->setUnits("L");
		coreVolumeBox->setCallback([this](double change)
								   {
				reactor->setReactorCoreVolume(change * 1e-03);
				properties->coreVolume = change * 1e-03; });

		Widget *waterPanel = settingsVert->add<Widget>();
		waterPanel->setLayout(panelsLayout);
		waterPanel->add<Label>("Water volume: ", "sans-bold");
		waterVolumeInput = waterPanel->add<FloatBox<double>>(properties->waterVolume);
		waterVolumeInput->setFixedSize(Vector2i(100, 30));
		waterVolumeInput->setAlignment(TextBox::Alignment::Left);
		waterVolumeInput->setSpinnable(true);
		waterVolumeInput->setValueIncrement(1.);
		waterVolumeInput->setDefaultValue(std::to_string(waterVolumeInput->value()));
		waterVolumeInput->setMinValue(1.);
		waterVolumeInput->setFormat(SCI_NUMBER_FORMAT);
		waterVolumeInput->setUnits("m" + string(utf8(0xB3).data()));
		waterVolumeInput->setCallback([this](double change)
									  {
				reactor->setWaterVolume(change);
				properties->waterVolume = change; });

		// Create a panel for temperature effects
		Widget *checkBoxPanelTemperatureEffects = settingsVert->add<Widget>();
		checkBoxPanelTemperatureEffects->setLayout(panelsLayout);
		// Create a panel for Fission poisoning
		Widget *checkBoxPanelFissionPoisoning = settingsVert->add<Widget>();
		checkBoxPanelFissionPoisoning->setLayout(panelsLayout);
		// Create a panel for excess reactivity
		Widget *excessPanel = settingsVert->add<Widget>();
		excessPanel->setLayout(panelsLayout);
		// Create a panel for cooling power
		Widget *waterCoolingPowerPanel = settingsVert->add<Widget>();
		waterCoolingPowerPanel->setLayout(panelsLayout);
		// Create a panel for prompt neutron lifetime
		Widget *promptPanel = settingsVert->add<Widget>();
		promptPanel->setLayout(panelsLayout);

		{
			checkBoxPanelTemperatureEffects->add<Label>("Temperature effects: ", "sans-bold");
			tempEffectsBox = checkBoxPanelTemperatureEffects->add<SliderCheckBox>();
			tempEffectsBox->setFontSize(16);
			tempEffectsBox->setChecked(properties->temperatureEffects);
			tempEffectsBox->setCallback([this](bool value)
										{
					reactor->setTemperatureEffectsEnabled(value);
					properties->temperatureEffects = value; });
		}
		{
			checkBoxPanelFissionPoisoning->add<Label>("Xe poisoning: ", "sans-bold");
			fissionProductsBox = checkBoxPanelFissionPoisoning->add<SliderCheckBox>();
			fissionProductsBox->setFontSize(16);
			fissionProductsBox->setChecked(properties->fissionPoisons);
			fissionProductsBox->setCallback([this](bool value)
											{
					reactor->setFissionPoisoningEffectsEnabled(value);
					properties->fissionPoisons = value; });
		}

		excessPanel->add<Label>("Excess reactivity: ", "sans-bold");
		excessReactivityBox = excessPanel->add<FloatBox<float>>(properties->excessReactivity);
		excessReactivityBox->setFixedSize(Vector2i(125, 30));
		excessReactivityBox->setUnits("pcm");
		excessReactivityBox->setMinValue(0.f);
		excessReactivityBox->setFormat(SCI_NUMBER_FORMAT);
		excessReactivityBox->setSpinnable(true);
		excessReactivityBox->setAlignment(TextBox::Alignment::Left);
		excessReactivityBox->setDefaultValue(std::to_string(excessReactivityBox->value()));
		excessReactivityBox->setValueIncrement(10.);
		excessReactivityBox->setCallback([this](float change)
										 {
				properties->excessReactivity = change;
				reactor->setExcessReactivity(change); });

		// Water cooling power
		waterCoolingPowerPanel->add<Label>("Water cooling power: ", "sans-bold");
		coolingPowerBox = waterCoolingPowerPanel->add<FloatBox<double>>(properties->waterCoolingPower);
		coolingPowerBox->setAlignment(TextBox::Alignment::Left);
		coolingPowerBox->setFixedSize(Vector2i(150, 30));
		coolingPowerBox->setSpinnable(true);
		coolingPowerBox->setValueIncrement(1.);
		coolingPowerBox->setDefaultValue(std::to_string(coolingPowerBox->value()));
		coolingPowerBox->setMinValue(1.);
		coolingPowerBox->setFormat(SCI_NUMBER_FORMAT);
		coolingPowerBox->setUnits("W");
		coolingPowerBox->setCallback([this](double change)
									 {
				properties->waterCoolingPower = change;
				reactor->setCoolingPower(change); });

		// Prompt neutron lifetime
		promptPanel->add<Label>("Prompt neutron lifetime: ", "sans-bold");
		promptNeutronLifetimeBox = promptPanel->add<FloatBox<double>>(properties->promptNeutronLifetime);
		promptNeutronLifetimeBox->setAlignment(TextBox::Alignment::Left);
		promptNeutronLifetimeBox->setFixedSize(Vector2i(150, 30));
		promptNeutronLifetimeBox->setSpinnable(true);
		promptNeutronLifetimeBox->setValueIncrement(1e-6);
		promptNeutronLifetimeBox->setDefaultValue(std::to_string(promptNeutronLifetimeBox->value()));
		promptNeutronLifetimeBox->setMinMaxValues(1e-7, 1.);
		promptNeutronLifetimeBox->setFormat(SCI_NUMBER_FORMAT);
		promptNeutronLifetimeBox->setUnits("s");
		promptNeutronLifetimeBox->setCallback([this](double change)
											  {
				properties->promptNeutronLifetime = change;
				reactor->setPromptNeutronLifetime(change); });

		// Alpha panel
		Widget *alphaPanel = physics_settings->add<Widget>();
		physicsLayout->setAnchor(alphaPanel, RelativeGridLayout::makeAnchor(2, 3, 1, 1));
		alphaPanel->setLayout(new BoxLayout(Orientation::Vertical, Alignment::Fill, 0, 10));
		// Temperature reactivity coef.
		CustomGraph *alphaGraph = alphaPanel->add<CustomGraph>(1, "Temp. reactivity coef.");
		alphaGraph->setBackgroundColor(Color(60, 255));
		alphaGraph->setDrawBackground(true);
		alphaGraph->setFixedHeight(250);
		alphaGraph->setPadding(80.f, 25.f, 30.f, 65.f);
		alphaGraph->setTextColor(Color(0, 255));

		alphaPlot = alphaGraph->addPlot(sigmaPoints, false);
		alphaPlot->setName("-" + alpha);
		alphaPlot->setHorizontalName("Temperature");
		alphaPlot->setTextColor(Color(250, 255));
		alphaPlot->setTextOffset(30.f);
		alphaPlot->setColor(Color(0, 120, 255, 255));
		alphaPlot->setPointerColor(Color(0, 120, 255, 255));
		alphaPlot->setPointerOverride(true);
		alphaPlot->setFill(true);
		alphaPlot->setFillColor(Color(0, 120, 255, 150));
		alphaPlot->setAxisShown(true);
		alphaPlot->setMainLineShown(true);
		alphaPlot->setAxisColor(Color(250, 255));
		alphaPlot->setMajorTickNumber(2);
		alphaPlot->setMinorTickNumber(4);
		alphaPlot->setTextShown(true);
		alphaPlot->setMainTickFontSize(22.f);
		alphaPlot->setMajorTickFontSize(20.f);
		alphaPlot->setUnits("pcm/" + degCelsiusUnit);
		alphaPlot->setHorizontalPointerColor(Color(120, 0, 255, 255));
		alphaPlot->setHorizontalAxisShown(true);
		alphaPlot->setHorizontalMainLineShown(true);
		alphaPlot->setHorizontalPointerShown(true);
		alphaPlot->setHorizontalMajorTickNumber(3);
		alphaPlot->setHorizontalMinorTickNumber(4);
		alphaPlot->setHorizontalUnits(degCelsiusUnit);
		alphaPlot->setDrawMode(DrawMode::Default);
		alphaPlot->setPlotRange(0, 2);

		alphaPlot->setLimits(0., 1000., 0., 15.);
		updateAlphaGraph();
		alphaPlot->setXdata(alphaX);
		alphaPlot->setYdata(alphaY);

		// Alpha settings
		string alphaText[4] = {"Starting " + alpha + ":", "Peak temp.:", alpha + " at peak temp.:", "Slope after peak temp.:"};
		float vals[4] = {properties->alpha0, properties->alphaT1, properties->alphaAtT1, (float)properties->alphaK};
		float increments[4] = {.1f, 5.f, .1f, .001f};
		float maxVals[3] = {20.f, 1000.f, 120.f};
		string units[4] = {"pcm/" + degCelsiusUnit, degCelsiusUnit, "pcm/" + degCelsiusUnit, "pcm/" + degCelsiusUnit + string(utf8(0xB2).data())};
		for (int i = 0; i < 4; i++)
		{
			// Create a panel for prompt neutron lifetime
			Widget *alphaSettingPanel = alphaPanel->add<Widget>();
			alphaSettingPanel->setLayout(panelsLayout);
			alphaSettingPanel->add<Label>(alphaText[i] + " ", "sans-bold");
			FloatBox<float> *temp = alphaSettingPanel->add<FloatBox<float>>(vals[i]);
			temp->setAlignment(TextBox::Alignment::Left);
			temp->setFixedSize(Vector2i(150, 30));
			temp->setSpinnable(true);
			temp->setValueIncrement(increments[i]);
			temp->setDefaultValue(std::to_string(temp->value()));
			if (i < 3)
			{
				temp->setMinMaxValues((i == 1) ? 0.f : -maxVals[i], maxVals[i]);
			}
			else
			{
				temp->setMinMaxValues(-4.f, 4.f);
			}
			temp->setFormat((i == 1) ? SCI_NUMBER_FORMAT : SCI_NUMBER_FORMAT_NEG);
			temp->setUnits(units[i]);
			switch (i)
			{
			case 0:
				alpha0Box = temp;
				break;
			case 1:
				tempPeakBox = temp;
				break;
			case 2:
				alphaPeakBox = temp;
				break;
			case 3:
				alphaSlopeBox = temp;
				break;
			}
		}

		alpha0Box->setCallback([this](float change)
							   {
				properties->alpha0 = change;
				reactor->setAlpha0(change);
				updateAlphaGraph(); });
		tempPeakBox->setCallback([this](float change)
								 {
				properties->alphaT1 = change;
				reactor->setAlphaTempPeak(change);
				updateAlphaGraph(); });
		alphaPeakBox->setCallback([this](float change)
								  {
				properties->alphaAtT1 = change;
				reactor->setAlphaPeak(change);
				updateAlphaGraph(); });
		alphaSlopeBox->setCallback([this](float change)
								   {
				properties->alphaK = (double)change;
				reactor->setAlphaSlope((double)change);
				updateAlphaGraph(); });

		modeTabs->setActiveTab(0);
	}

	void createRodSettingsTab()
	{
		Widget *rod_settings = tabControl->createTab("Control rods");
		rod_settings->setId("Rod settings tab");
		RelativeGridLayout *rod_settings_layout = new RelativeGridLayout(); /* COLUMNS */
		for (int i = 0; i < 9; i++)
		{
			if (i == 3 || i == 6)
				rod_settings_layout->appendCol(RelativeGridLayout::Size(2.f, RelativeGridLayout::SizeType::Fixed));
			rod_settings_layout->appendCol(1.f);
		}
		/* ROWS */
		rod_settings_layout->appendRow(RelativeGridLayout::Size(35.f, RelativeGridLayout::SizeType::Fixed));  // 0 title
		rod_settings_layout->appendRow(RelativeGridLayout::Size(15.f, RelativeGridLayout::SizeType::Fixed));  // 1 empty space
		rod_settings_layout->appendRow(RelativeGridLayout::Size(20.f, RelativeGridLayout::SizeType::Fixed));  // 2 setting name
		rod_settings_layout->appendRow(RelativeGridLayout::Size(20.f, RelativeGridLayout::SizeType::Fixed));  // 3 setting value
		rod_settings_layout->appendRow(RelativeGridLayout::Size(30.f, RelativeGridLayout::SizeType::Fixed));  // 4 top slider
		rod_settings_layout->appendRow(RelativeGridLayout::Size(320.f, RelativeGridLayout::SizeType::Fixed)); // 5 graph
		rod_settings_layout->appendRow(RelativeGridLayout::Size(30.f, RelativeGridLayout::SizeType::Fixed));  // 6 bottom slider
		rod_settings_layout->appendRow(RelativeGridLayout::Size(15.f, RelativeGridLayout::SizeType::Fixed));  // 7 empty space
		rod_settings_layout->appendRow(RelativeGridLayout::Size(1.f, RelativeGridLayout::SizeType::Fixed));	  // 8 border
		rod_settings_layout->appendRow(RelativeGridLayout::Size(15.f, RelativeGridLayout::SizeType::Fixed));  // 9 empty space
		rod_settings_layout->appendRow(RelativeGridLayout::Size(20.f, RelativeGridLayout::SizeType::Fixed));  // 10 label
		rod_settings_layout->appendRow(RelativeGridLayout::Size(320.f, RelativeGridLayout::SizeType::Fixed)); // 11 derivative graph
		rod_settings->setLayout(rod_settings_layout);

		// Create borders
		for (int i = 0; i < 2; i++)
		{
			CustomWidget *border = rod_settings->add<CustomWidget>();
			rod_settings_layout->setAnchor(border, RelativeGridLayout::makeAnchor(i ? 3 : 7, 0, 1, 13));
			border->setBackgroundColor(coolBlue);
			border->setDrawBackground(true);
		}
		CustomWidget *border = rod_settings->add<CustomWidget>();
		rod_settings_layout->setAnchor(border, RelativeGridLayout::makeAnchor(0, 8, 11, 1));
		border->setBackgroundColor(coolBlue);
		border->setDrawBackground(true);

		Label *tempLabel;
		ControlRod *useRod;
		// for each rod
		for (int i = 0; i < NUMBER_OF_CONTROL_RODS; i++)
		{
			useRod = reactor->rods[i];

			// Titles
			tempLabel = rod_settings->add<Label>(useRod->getRodName() + " rod", "sans-bold");
			tempLabel->setFontSize(30);
			rod_settings_layout->setAnchor(tempLabel, RelativeGridLayout::makeAnchor(i * 4, 0, 3, 1, Alignment::Middle, Alignment::Maximum));

			// Rod steps setting
			tempLabel = rod_settings->add<Label>("Rod steps:", "sans-bold");
			rod_settings_layout->setAnchor(tempLabel, RelativeGridLayout::makeAnchor(i * 4, 2, 1, 1, Alignment::Middle, Alignment::Middle));
			rodStepsBox[i] = rod_settings->add<IntBox<int>>((int)properties->rodSettings[i].rodSteps);
			rod_settings_layout->setAnchor(rodStepsBox[i], RelativeGridLayout::makeAnchor(i * 4, 3, 1, 1, Alignment::Middle));
			rodStepsBox[i]->setFixedSize(Vector2i(100, 20));
			rodStepsBox[i]->setAlignment(TextBox::Alignment::Left);
			rodStepsBox[i]->setUnits("steps");
			rodStepsBox[i]->setDefaultValue(to_string(properties->rodSettings[i].rodSteps));
			rodStepsBox[i]->setMinMaxValues(1, 100000);
			rodStepsBox[i]->setValueIncrement(10);
			rodStepsBox[i]->setFormat("[0-9]+");
			rodStepsBox[i]->setCallback([useRod, i, this](int change)
										{
					properties->rodSettings[i].rodSteps = change;
					useRod->setRodSteps((size_t)change);
					rodCurves[i]->setLimitHorizontalMultiplier((double)*useRod->getRodSteps()); });

			// Rod worth setting
			tempLabel = rod_settings->add<Label>("Rod worth:", "sans-bold");
			rod_settings_layout->setAnchor(tempLabel, RelativeGridLayout::makeAnchor(i * 4 + 1, 2, 1, 1, Alignment::Middle, Alignment::Middle));
			rodWorthBox[i] = rod_settings->add<FloatBox<float>>(properties->rodSettings[i].rodWorth);
			rod_settings_layout->setAnchor(rodWorthBox[i], RelativeGridLayout::makeAnchor(i * 4 + 1, 3, 1, 1, Alignment::Middle));
			rodWorthBox[i]->setFixedSize(Vector2i(100, 20));
			rodWorthBox[i]->setAlignment(TextBox::Alignment::Left);
			rodWorthBox[i]->setUnits("pcm");
			rodWorthBox[i]->setDefaultValue(to_string(properties->rodSettings[i].rodWorth));
			rodWorthBox[i]->setMinValue(1.f);
			rodWorthBox[i]->setValueIncrement(10.f);
			rodWorthBox[i]->setFormat("[0-9]*\\.?[0-9]+");
			rodWorthBox[i]->setCallback([useRod, i, this](float change)
										{
					properties->rodSettings[i].rodWorth = change;
					useRod->setRodWorth(change); });

			// Rod speed setting
			tempLabel = rod_settings->add<Label>("Rod speed:", "sans-bold");
			rod_settings_layout->setAnchor(tempLabel, RelativeGridLayout::makeAnchor(i * 4 + 2, 2, 1, 1, Alignment::Middle, Alignment::Middle));
			rodSpeedBox[i] = rod_settings->add<FloatBox<float>>(properties->rodSettings[i].rodSpeed);
			rod_settings_layout->setAnchor(rodSpeedBox[i], RelativeGridLayout::makeAnchor(i * 4 + 2, 3, 1, 1, Alignment::Middle));
			rodSpeedBox[i]->setFixedSize(Vector2i(100, 20));
			rodSpeedBox[i]->setAlignment(TextBox::Alignment::Left);
			rodSpeedBox[i]->setUnits("steps/s");
			rodSpeedBox[i]->setDefaultValue(to_string(properties->rodSettings[i].rodSpeed));
			rodSpeedBox[i]->setMinValue(0.f);
			rodSpeedBox[i]->setValueIncrement(1.f);
			rodSpeedBox[i]->setFormat("[0-9]*\\.?[0-9]+");
			rodSpeedBox[i]->setCallback([useRod, i, this](float change)
										{
											properties->rodSettings[i].rodSpeed = change;
											useRod->setRodSpeed(change);
											if (i == 1)
												reactor->regulatingRod()->sine()->fillXYaxis(operationModesPlots[0][1], operationModesPlots[1][1]); // Update SQW graph
										});

			// Display tool
			CustomGraph *rodControl = rod_settings->add<CustomGraph>(1);
			rod_settings_layout->setAnchor(rodControl, RelativeGridLayout::makeAnchor(i * 4, 5, 3, 1, Alignment::Middle));
			rodControl->setFixedWidth(320);
			rodControl->setBackgroundColor(Color(60, 255));
			rodControl->setDrawBackground(true);
			rodControl->setPadding(80.f, 25.f, 30.f, 70.f);
			rodControl->setTextColor(Color(250, 255));
			rodCurves[i] = rodControl->addBezierCurve();
			rodCurves[i]->setName("Reactivity");
			rodCurves[i]->setHorizontalName("Position");
			rodCurves[i]->setTextColor(Color(250, 255));
			rodCurves[i]->setTextOffset(30.f);
			rodCurves[i]->setColor(Color(0, 120, 255, 255));
			rodCurves[i]->setPointerColor(Color(0, 120, 255, 255));
			rodCurves[i]->setPointerOverride(true);
			rodCurves[i]->setFill(true);
			rodCurves[i]->setFillColor(Color(0, 120, 255, 150));
			rodCurves[i]->setAxisShown(true);
			rodCurves[i]->setMainLineShown(true);
			rodCurves[i]->setAxisColor(Color(250, 255));
			rodCurves[i]->setMajorTickNumber(3);
			rodCurves[i]->setMinorTickNumber(1);
			rodCurves[i]->setTextShown(true);
			rodCurves[i]->setMainTickFontSize(22.f);
			rodCurves[i]->setMajorTickFontSize(20.f);
			rodCurves[i]->setLimitMultiplier(100.);
			rodCurves[i]->setUnits("%");
			rodCurves[i]->setHorizontalPointerColor(Color(120, 0, 255, 255));
			rodCurves[i]->setHorizontalAxisShown(true);
			rodCurves[i]->setHorizontalMainLineShown(true);
			rodCurves[i]->setHorizontalPointerShown(true);
			rodCurves[i]->setHorizontalMajorTickNumber(1);
			rodCurves[i]->setHorizontalMinorTickNumber(1);
			rodCurves[i]->setLimitHorizontalMultiplier((double)*useRod->getRodSteps());
			rodCurves[i]->setHorizontalUnits("steps");

			// Sliders
			for (int j = 0; j < 2; j++)
			{
				rodCurves[i]->setParameter(j * 2, properties->rodSettings[i].rodCurve[j]);
				size_t sliderIndex = i * 2 + j;
				rodCurveSliders[sliderIndex] = rod_settings->add<Slider>();
				rod_settings_layout->setAnchor(rodCurveSliders[sliderIndex], RelativeGridLayout::makeAnchor(i * 4, 4 + j * 2, 3, 1, Alignment::Middle, Alignment::Middle));
				rodCurveSliders[sliderIndex]->setFixedSize(Vector2i(340, 15));
				rodCurveSliders[sliderIndex]->setValue(properties->rodSettings[i].rodCurve[j]);
				rodCurveSliders[sliderIndex]->setCallback([this, i, j](float change)
														  { rodCurves[i]->setParameter(j * 2, change); });
				rodCurveSliders[sliderIndex]->setFinalCallback([useRod, j, i, this](float stop)
															   {
						rodCurves[i]->setParameter(j * 2, stop);
						properties->rodSettings[i].rodCurve[j] = stop;
						useRod->setParameter(j, stop);
						handleDerivativeChange(); });
			}

			// Derivative graphs
			tempLabel = rod_settings->add<Label>("Differential\nrod worth curve", "sans-bold", 25);
			rod_settings_layout->setAnchor(tempLabel, RelativeGridLayout::makeAnchor(i * 4, 10, 3, 1, Alignment::Middle, Alignment::Maximum));

			CustomGraph *rodDerDisplay = rod_settings->add<CustomGraph>(1);
			rod_settings_layout->setAnchor(rodDerDisplay, RelativeGridLayout::makeAnchor(i * 4, 11, 3, 1, Alignment::Middle, Alignment::Minimum));
			rodDerDisplay->setFixedSize(Vector2i(320, 320));
			rodDerDisplay->setBackgroundColor(Color(60, 255));
			rodDerDisplay->setDrawBackground(true);
			rodDerDisplay->setPadding(80.f, 25.f, 30.f, 70.f);
			rodDerDisplay->setTextColor(Color(250, 255));
			rodDerivatives[i] = rodDerDisplay->addPlot(ControlRod::dataPoints, false);
			rodDerivatives[i]->setName("Reactivity change");
			rodDerivatives[i]->setHorizontalName("Position");
			rodDerivatives[i]->setTextColor(Color(250, 255));
			rodDerivatives[i]->setTextOffset(30.f);
			rodDerivatives[i]->setColor(Color(0, 120, 255, 255));
			rodDerivatives[i]->setPointerColor(Color(0, 120, 255, 255));
			rodDerivatives[i]->setPointerOverride(true);
			rodDerivatives[i]->setPointerShown(true);
			rodDerivatives[i]->setHorizontalPointerShown(true);
			rodDerivatives[i]->setFill(true);
			rodDerivatives[i]->setFillColor(Color(0, 120, 255, 150));
			rodDerivatives[i]->setAxisShown(true);
			rodDerivatives[i]->setMainLineShown(true);
			rodDerivatives[i]->setAxisColor(Color(250, 255));
			rodDerivatives[i]->setMajorTickNumber(3);
			rodDerivatives[i]->setMinorTickNumber(1);
			rodDerivatives[i]->setTextShown(true);
			rodDerivatives[i]->setMainTickFontSize(22.f);
			rodDerivatives[i]->setMajorTickFontSize(20.f);
			rodDerivatives[i]->setUnits("pcm/step");
			rodDerivatives[i]->setHorizontalPointerColor(Color(120, 0, 255, 255));
			rodDerivatives[i]->setHorizontalAxisShown(true);
			rodDerivatives[i]->setHorizontalMainLineShown(true);
			rodDerivatives[i]->setHorizontalPointerShown(true);
			rodDerivatives[i]->setHorizontalMajorTickNumber(1);
			rodDerivatives[i]->setHorizontalMinorTickNumber(1);
			rodDerivatives[i]->setLimitHorizontalMultiplier((double)*useRod->getRodSteps());
			rodDerivatives[i]->setHorizontalUnits("steps");
			rodDerivatives[i]->setXdataLin(0L);
			rodDerivatives[i]->setPlotRange(0, *useRod->getRodSteps());
			rodDerivatives[i]->setValueComputing([this, i](double *v, size_t /*in*/)
												 { *v *= reactor->rods[i]->getRodWorth(); });
		}
		handleDerivativeChange();
	}

	void createDelayedGroupsTab()
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

	double *operationModesPlots[2][3];
	const int simModeFields[3] = {7, 3, 8};
	void createOperationModesTab()
	{
		Widget *modes_base = tabControl->createTab("Operation modes", false);
		CustomTabWidget *modeTabs = modes_base->add<CustomTabWidget>();
		modeTabs->header()->setStretch(true);
		modeTabs->header()->setButtonAlignment(NVGalign::NVG_ALIGN_MIDDLE | NVGalign::NVG_ALIGN_CENTER);
		RelativeGridLayout *grid = new RelativeGridLayout();
		grid->appendCol(1.f);
		grid->appendRow(1.f);
		modes_base->setLayout(grid);
		grid->setAnchor(modeTabs, RelativeGridLayout::makeAnchor(0, 0));

		// Create titles, periods and amplitudes
		std::string titles[3] = {"Square wave", "Sine wave", "Saw tooth"};
		RelativeGridLayout *layouts[3];
		CustomWidget *tabs[3];
		for (int i = 0; i < 3; i++)
		{
			tabs[i] = modeTabs->createTab(titles[i]);
			tabs[i]->setId(titles[i] + " tab");
			tabs[i]->setDrawBackground(true);
			tabs[i]->setBackgroundColor(Color(100, 255));

			layouts[i] = new RelativeGridLayout();
			layouts[i]->appendCol(RelativeGridLayout::Size(15.f, RelativeGridLayout::SizeType::Fixed));	 // left border
			layouts[i]->appendCol(RelativeGridLayout::Size(250.f, RelativeGridLayout::SizeType::Fixed)); // settings
			layouts[i]->appendCol(1.f);																	 // graph
			layouts[i]->appendCol(RelativeGridLayout::Size(15.f, RelativeGridLayout::SizeType::Fixed));	 // right border
			layouts[i]->appendRow(RelativeGridLayout::Size(15.f, RelativeGridLayout::SizeType::Fixed));	 // top border
			for (int j = 0; j < simModeFields[i]; j++)
			{
				layouts[i]->appendRow(RelativeGridLayout::Size(30.f, RelativeGridLayout::SizeType::Fixed));
			}
			layouts[i]->appendRow(1.f);																	// empty space
			layouts[i]->appendRow(RelativeGridLayout::Size(20.f, RelativeGridLayout::SizeType::Fixed)); // bottom border
			tabs[i]->setLayout(layouts[i]);

			PeriodicalMode *use = reactor->regulatingRod()->getMode((SimulationModes)(i + 1));
			size_t arraySize = use->num_points();

			// create graph first just for event binding
			CustomGraph *graph = tabs[i]->add<CustomGraph>(2, titles[i]);
			graph->setDrawBackground(true);
			graph->setBackgroundColor(Color(32, 255));
			layouts[i]->setAnchor(graph, RelativeGridLayout::makeAnchor(2, 0, 2, simModeFields[i] + 3, Alignment::Fill, Alignment::Fill));
			graph->setPadding(90.f, 20.f, 10.f, 70.f);
			graph->setPlotBackgroundColor(Color(60, 255));
			graph->setPlotGridColor(Color(177, 255));
			graph->setPlotBorderColor(Color(200, 255));
			operationModes[i] = graph->addPlot(arraySize);
			operationModes[i]->setPlotRange(0, arraySize - 1);
			operationModes[i]->setLimits(0., use->getPeriod(), -1.5 * use->getAmplitude(), 1.5 * use->getAmplitude());
			operationModesTrackers[i] = graph->addPlot(2);
			operationModesTrackers[i]->setPlotRange(0, 1);
			operationModesTrackers[i]->setDrawMode(DrawMode::Default);
			operationModesTrackers[i]->setAxisShown(false);
			operationModesTrackers[i]->setColor(Color(1.f, 0.f, 0.f, 1.f));
			operationModesTrackers[i]->setHorizontalAxisShown(false);
			operationModesTrackers[i]->setTextShown(false);
			operationModesTrackers[i]->setPointerShown(false);
			operationModesTrackers[i]->setXdata(use->getTrackerArray());
			operationModesTrackers[i]->setYdata(trackerY);
			operationModesTrackers[i]->setLimits(0., use->getPeriod(), 0., 1.);

			// Make arrays
			for (size_t p = 0; p < 2; p++)
				operationModesPlots[p][i] = new double[arraySize];
			// Fill arrays
			use->fillXYaxis(operationModesPlots[0][i], operationModesPlots[1][i]);

			// period
			periodBoxes[i] = makeSettingLabel<FloatBox<float>>(tabs[i], "Period: ", 100, use->getPeriod());
			layouts[i]->setAnchor(periodBoxes[i]->parent(), RelativeGridLayout::makeAnchor(1, 1, 1, 1, Alignment::Minimum, Alignment::Middle));
			periodBoxes[i]->setFixedWidth(100);
			periodBoxes[i]->setAlignment(TextBox::Alignment::Left);
			periodBoxes[i]->setFormat(SCI_NUMBER_FORMAT);
			periodBoxes[i]->setUnits("s");
			periodBoxes[i]->setMinValue(0.1f);
			periodBoxes[i]->setValueIncrement(0.1f);
			periodBoxes[i]->setSpinnable(true);
			periodBoxes[i]->setDefaultValue(formatDecimalsDouble(use->getPeriod(), 1));
			periodBoxes[i]->setCallback([use, i, this](float change)
										{
					switch (i)
					{
					case 0: properties->squareWave.period = change; break;
					case 1: properties->sineMode.period = change; break;
					case 2: properties->sawToothMode.period = change; break;
					}
					use->setPeriod(change);
					use->fillXYaxis(operationModesPlots[0][i], operationModesPlots[1][i]);
					operationModes[i]->setLimits(0., change, -1.5 * use->getAmplitude(), 1.5 * use->getAmplitude());
					operationModesTrackers[i]->setLimits(0., use->getPeriod(), 0., 1.); });

			// amplitude
			amplitudeBoxes[i] = makeSettingLabel<FloatBox<float>>(tabs[i], "Amplitude: ", 100, use->getAmplitude());
			layouts[i]->setAnchor(amplitudeBoxes[i]->parent(), RelativeGridLayout::makeAnchor(1, 2, 1, 1, Alignment::Minimum, Alignment::Middle));
			amplitudeBoxes[i]->setFixedWidth(100);
			amplitudeBoxes[i]->setAlignment(TextBox::Alignment::Left);
			amplitudeBoxes[i]->setFormat(SCI_NUMBER_FORMAT);
			amplitudeBoxes[i]->setUnits("steps");
			amplitudeBoxes[i]->setMinValue(0.f);
			amplitudeBoxes[i]->setValueIncrement(1);
			amplitudeBoxes[i]->setSpinnable(true);
			amplitudeBoxes[i]->setDefaultValue(to_string(use->getAmplitude()));
			amplitudeBoxes[i]->setCallback([use, i, this](float change)
										   {
					switch (i)
					{
					case 0: properties->squareWave.amplitude = change; break;
					case 1: properties->sineMode.amplitude = change; break;
					case 2: properties->sawToothMode.amplitude = change; break;
					}
					use->setAmplitude(change);
					use->fillXYaxis(operationModesPlots[0][i], operationModesPlots[1][i]);
					operationModes[i]->setLimits(0., use->getPeriod(), -1.5 * change, 1.5 * change); });

			operationModes[i]->setMainTickFontSize(18.f);
			operationModes[i]->setMajorTickFontSize(16.f);
			operationModes[i]->setNameFontSize(24.f);
			operationModes[i]->setPointerShown(false);
			operationModes[i]->setXdata(operationModesPlots[0][i]);
			operationModes[i]->setYdata(operationModesPlots[1][i]);
			operationModes[i]->setColor(coolBlue);
			operationModes[i]->setLimits(0., use->getPeriod(), -1.5 * use->getAmplitude(), 1.5 * use->getAmplitude());
			operationModes[i]->setDrawMode(DrawMode::Default);
			operationModes[i]->setTextColor(Color(250, 255));
			operationModes[i]->setAxisColor(Color(250, 255));
			operationModes[i]->setTextShown(true);
			operationModes[i]->setAxisShown(true);
			operationModes[i]->setUnits("steps");
			operationModes[i]->setName("Delta rod position");
			operationModes[i]->setTextOffset(40.f);
			operationModes[i]->setMainLineShown(true);
			operationModes[i]->setMajorTickNumber(3);
			operationModes[i]->setMinorTickNumber(1);
			operationModes[i]->setHorizontalAxisShown(true);
			operationModes[i]->setHorizontalUnits("s");
			operationModes[i]->setHorizontalName("Time");
			operationModes[i]->setHorizontalMainLineShown(true);
			operationModes[i]->setHorizontalMajorTickNumber(3);
			operationModes[i]->setHorizontalMinorTickNumber(1);
		}

		/* SQUARE WAVE */
		for (int sqw = 0; sqw < 4; sqw++)
		{
			squareWaveBoxes[sqw] = makeSimulationSetting(tabs[0], (int)roundf(properties->squareWave.xIndex[sqw] * 100), sqw_settingNames[sqw]);
			layouts[0]->setAnchor(squareWaveBoxes[sqw]->parent(), RelativeGridLayout::makeAnchor(1, 3 + sqw, 1, 1, Alignment::Minimum, Alignment::Middle));
			squareWaveBoxes[sqw]->setCallback([this, sqw](int change)
											  {
					float val = change / 100.f;
					reactor->regulatingRod()->squareWave()->xIndex[sqw] = val;
					properties->squareWave.xIndex[sqw] = val;
					if (sqw != 3)
					{
						squareWaveBoxes[sqw + 1]->setMinValue(change);
						if (squareWaveBoxes[sqw + 1]->value() < change) squareWaveBoxes[sqw + 1]->setValue(change);
					}
					reactor->regulatingRod()->squareWave()->fillXYaxis(operationModesPlots[0][0], operationModesPlots[1][0]); });
		}
		squareWaveSpeedBox = makeSettingLabel<SliderCheckBox>(tabs[0], "Use finite rod speed: ");
		layouts[0]->setAnchor(squareWaveSpeedBox->parent(), RelativeGridLayout::makeAnchor(1, 7, 1, 1, Alignment::Minimum, Alignment::Middle));
		squareWaveSpeedBox->setCallback([this](bool change)
										{
				properties->squareWaveUsesRodSpeed = change;
				if (change)
				{
					reactor->regulatingRod()->squareWave()->rodSpeed = reactor->regulatingRod()->getRodSpeed();
				}
				else
				{
					reactor->regulatingRod()->squareWave()->rodSpeed = 0.f;
				}
				reactor->regulatingRod()->squareWave()->fillXYaxis(operationModesPlots[0][0], operationModesPlots[1][0]); });

		/* SINE MODE */
		sineModeBox = makeSettingLabel<ComboBox>(tabs[1], "Sine type: ", 0, sineModes);
		layouts[1]->setAnchor(sineModeBox->parent(), RelativeGridLayout::makeAnchor(1, 3, 1, 1, Alignment::Minimum, Alignment::Middle));
		sineModeBox->setFixedWidth(150);
		sineModeBox->setCallback([this](int change)
								 {
				properties->sineMode.mode = (Settings::SineSettings::SineMode)change;
				reactor->regulatingRod()->sine()->mode = (Sine::SineMode)change;
				reactor->regulatingRod()->sine()->fillXYaxis(operationModesPlots[0][1], operationModesPlots[1][1]); });

		/* SAW TOOTH */
		for (int saw = 0; saw < 6; saw++)
		{
			sawToothBoxes[saw] = makeSimulationSetting(tabs[2], (int)roundf(properties->sawToothMode.xIndex[saw] * 100), saw_settingNames[saw]);
			layouts[2]->setAnchor(sawToothBoxes[saw]->parent(), RelativeGridLayout::makeAnchor(1, 3 + saw, 1, 1, Alignment::Minimum, Alignment::Middle));
			sawToothBoxes[saw]->setCallback([this, saw](int change)
											{
					float val = change / 100.f;
					reactor->regulatingRod()->sawTooth()->xIndex[saw] = val;
					properties->sawToothMode.xIndex[saw] = val;
					if (saw < 5)
					{
						sawToothBoxes[saw + 1]->setMinValue(change);
						if (sawToothBoxes[saw + 1]->value() < change) sawToothBoxes[saw + 1]->setValue(change);
					}
					reactor->regulatingRod()->sawTooth()->fillXYaxis(operationModesPlots[0][2], operationModesPlots[1][2]); });
		}

		// AUTOMATIC
		CustomWidget *autoTab = modeTabs->createTab("Automatic");
		RelativeGridLayout *autoLayout = new RelativeGridLayout();
		autoLayout->appendCol(RelativeGridLayout::Size(15.f, RelativeGridLayout::SizeType::Fixed)); // left border
		autoLayout->appendCol(1.f);																	// everything else
		autoLayout->appendRow(RelativeGridLayout::Size(15.f, RelativeGridLayout::SizeType::Fixed)); // top border
		for (int i = 0; i < 4; i++)
			autoLayout->appendRow(RelativeGridLayout::Size(30.f, RelativeGridLayout::SizeType::Fixed)); // 4 rows for 4 settings
		autoTab->setLayout(autoLayout);

		keepCurrentPowerBox = makeSettingLabel<SliderCheckBox>(autoTab, "Current power stability: ", 0);
		keepCurrentPowerBox->setChecked(properties->steadyCurrentPower);
		autoLayout->setAnchor(keepCurrentPowerBox->parent(), RelativeGridLayout::makeAnchor(1, 1, 1, 1, Alignment::Minimum, Alignment::Middle));

		steadyPowerBox = makeSettingLabel<FloatBox<double>>(autoTab, "Steady power: ", 100, properties->steadyGoalPower);
		steadyPowerBox->setEnabled(!properties->steadyCurrentPower);
		steadyPowerBox->setUnits("W");
		steadyPowerBox->setFormat(SCI_NUMBER_FORMAT);
		steadyPowerBox->setMinValue(0.);
		steadyPowerBox->setAlignment(TextBox::Alignment::Right);
		steadyPowerBox->setValueIncrement(1e3);
		steadyPowerBox->setSpinnable(true);
		autoLayout->setAnchor(steadyPowerBox->parent(), RelativeGridLayout::makeAnchor(1, 2, 1, 1, Alignment::Minimum, Alignment::Middle));

		keepCurrentPowerBox->setCallback([this](bool checked)
										 {
				steadyPowerBox->setEnabled(!checked);
				properties->steadyCurrentPower = checked;
				reactor->setKeepCurrentPower(checked); });

		steadyPowerBox->setCallback([this](double newValue)
									{
				if (newValue > 0.)
				{
					properties->steadyGoalPower = newValue;
					reactor->setAutomaticSteadyPower(newValue);
				} });

		avoidPeriodScramBox = makeSettingLabel<SliderCheckBox>(autoTab, "Avoid period SCRAM: ", 0);
		avoidPeriodScramBox->setChecked(reactor->getKeepCurrentPower());
		autoLayout->setAnchor(avoidPeriodScramBox->parent(), RelativeGridLayout::makeAnchor(1, 3, 1, 1, Alignment::Minimum, Alignment::Middle));

		avoidPeriodScramBox->setCallback([this](bool checked)
										 {
				properties->avoidPeriodScram = checked;
				reactor->setAutomaticAvoidPeriodScram(checked); });

		automaticMarginBox = makeSettingLabel<FloatBox<float>>(autoTab, "Maximum power deviation: ", 0, properties->steadyMargin * 100);
		automaticMarginBox->setUnits("%");
		automaticMarginBox->setFormat(SCI_NUMBER_FORMAT);
		automaticMarginBox->setMinMaxValues(0.1f, 10.f);
		automaticMarginBox->setAlignment(TextBox::Alignment::Right);
		automaticMarginBox->setValueIncrement(0.5f);
		automaticMarginBox->setSpinnable(true);
		autoLayout->setAnchor(automaticMarginBox->parent(), RelativeGridLayout::makeAnchor(1, 4, 1, 1, Alignment::Minimum, Alignment::Middle));

		automaticMarginBox->setCallback([this](float change)
										{
				properties->steadyMargin = change / 100;
				reactor->setAutomaticDeviation(change / 100); });

		modeTabs->setActiveTab(0);
	}

	const std::string labels[5] = {"Period", "Power", "Fuel temperature", "Water temperature", "Water level"};
	const Simulator::ScramSignals reasons[5] = {Simulator::ScramSignals::Period, Simulator::ScramSignals::Power, Simulator::ScramSignals::FuelTemperature, Simulator::ScramSignals::WaterTemperature, Simulator::ScramSignals::WaterLevel};
	void createOperationalLimitsTab()
	{
		CustomWidget *limits_tab = tabControl->createTab("Operational limits");
		limits_tab->setId("op. limits tab");
		RelativeGridLayout *rel = new RelativeGridLayout();
		rel->appendCol(RelativeGridLayout::Size(15.f, RelativeGridLayout::SizeType::Fixed));  // left border
		rel->appendCol(RelativeGridLayout::Size(200.f, RelativeGridLayout::SizeType::Fixed)); // text
		rel->appendCol(RelativeGridLayout::Size(100.f, RelativeGridLayout::SizeType::Fixed)); // data
		rel->appendCol(1.f);																  // check boxes
		rel->appendRow(RelativeGridLayout::Size(15.f, RelativeGridLayout::SizeType::Fixed));  // top border
		limits_tab->setLayout(rel);

		// Skips water level
		for (int i = 0; i < 4; i++)
		{
			rel->appendRow(RelativeGridLayout::Size(27.5f, RelativeGridLayout::SizeType::Fixed));
			Label *temp = limits_tab->add<Label>(labels[i] + " SCRAM :", "sans-bold");
			rel->setAnchor(temp, RelativeGridLayout::makeAnchor(1, 1 + i, 1, 1, Alignment::Minimum, Alignment::Middle));
			scramEnabledBoxes[i] = limits_tab->add<SliderCheckBox>();
			RelativeGridLayout::Anchor a = RelativeGridLayout::makeAnchor(3, 1 + i, 1, 1, Alignment::Minimum, Alignment::Middle);
			a.padding[0] = 8;
			rel->setAnchor(scramEnabledBoxes[i], a);
			scramEnabledBoxes[i]->setChecked(reactor->getScramEnabled(reasons[i]));
			scramEnabledBoxes[i]->setCallback([this, i](bool checked)
											  {
					reactor->setScramEnabled(reasons[i], checked);
					switch (i)
					{
					case 0: properties->periodScram = checked; break;
					case 1: properties->powerScram = checked; break;
					case 2: properties->tempScram = checked; break;
					case 3: properties->waterTempScram = checked; break;
					case 4: properties->waterLevelScram = checked; break;
					} });
		}

		// Create the period limit
		periodLimBox = limits_tab->add<IntBox<float>>((float)properties->periodLimit);
		rel->setAnchor(periodLimBox, RelativeGridLayout::makeAnchor(2, 1, 1, 1, Alignment::Fill, Alignment::Middle));
		periodLimBox->setFixedSize(Vector2i(100, 20));
		periodLimBox->setUnits("s");
		periodLimBox->setDefaultValue(formatDecimalsDouble((float)properties->periodLimit, 1));
		periodLimBox->setFontSize(16);
		periodLimBox->setFormat(SCI_NUMBER_FORMAT);
		periodLimBox->setMinMaxValues(0.f, 3600.f);
		periodLimBox->setValueIncrement(0.1f);
		periodLimBox->setCallback([this](float a)
								  {
				properties->periodLimit = a;
				reactor->setPeriodLimit(a); });

		// Create the power limit
		powerLimBox = limits_tab->add<FloatBox<double>>(properties->powerLimit / 1000.);
		rel->setAnchor(powerLimBox, RelativeGridLayout::makeAnchor(2, 2, 1, 1, Alignment::Fill, Alignment::Middle));
		powerLimBox->setFixedSize(Vector2i(100, 20));
		powerLimBox->setUnits("kW");
		powerLimBox->setDefaultValue(to_string(powerLimBox->value()));
		powerLimBox->setFontSize(16);
		powerLimBox->setFormat(SCI_NUMBER_FORMAT);
		powerLimBox->setMinMaxValues(0., 1e12);
		powerLimBox->setValueIncrement(1e2);
		powerLimBox->setCallback([this](double a)
								 {
				properties->powerLimit = a * 1e3;
				reactor->setPowerLimit(a * 1e3); });

		// Create the fuel temperature limit
		fuel_tempLimBox = limits_tab->add<FloatBox<float>>(properties->tempLimit);
		rel->setAnchor(fuel_tempLimBox, RelativeGridLayout::makeAnchor(2, 3, 1, 1, Alignment::Fill, Alignment::Middle));
		fuel_tempLimBox->setFixedSize(Vector2i(100, 20));
		fuel_tempLimBox->setUnits(degCelsiusUnit);
		fuel_tempLimBox->setDefaultValue(to_string(fuel_tempLimBox->value()));
		fuel_tempLimBox->setFontSize(16);
		fuel_tempLimBox->setFormat(SCI_NUMBER_FORMAT);
		fuel_tempLimBox->setMinValue((int)ENVIRONMENT_TEMPERATURE_DEFAULT);
		fuel_tempLimBox->setValueIncrement(10);
		fuel_tempLimBox->setCallback([this](float a)
									 {
				properties->tempLimit = a;
				reactor->setFuelTemperatureLimit(a); });

		// Create the water temperature limit
		water_tempLimBox = limits_tab->add<FloatBox<float>>(properties->waterTempLimit);
		rel->setAnchor(water_tempLimBox, RelativeGridLayout::makeAnchor(2, 4, 1, 1, Alignment::Fill, Alignment::Middle));
		water_tempLimBox->setFixedSize(Vector2i(100, 20));
		water_tempLimBox->setUnits(degCelsiusUnit);
		water_tempLimBox->setDefaultValue(to_string(water_tempLimBox->value()));
		water_tempLimBox->setFontSize(16);
		water_tempLimBox->setFormat(SCI_NUMBER_FORMAT);
		water_tempLimBox->setMinMaxValues(0, 100);
		water_tempLimBox->setValueIncrement(10);
		water_tempLimBox->setCallback([this](float a)
									  {
				properties->waterTempLimit = a;
				reactor->setWaterTemperatureLimit(a); });

		// Create the water level limit
		/*water_levelLimBox = limits_tab->add<FloatBox<float>>(properties->waterLevelLimit);
		rel->setAnchor(water_levelLimBox, RelativeGridLayout::makeAnchor(2, 5, 1, 1, Alignment::Fill, Alignment::Middle));
		water_levelLimBox->setFixedSize(Vector2i(100, 20));
		water_levelLimBox->setUnits("m");
		water_levelLimBox->setDefaultValue(to_string(water_levelLimBox->value()));
		water_levelLimBox->setFontSize(16);
		water_levelLimBox->setFormat("[0-9]*\\.?[0-9]+");
		water_levelLimBox->setMinValue(0.f);
		water_levelLimBox->setValueIncrement(0.1f);
		water_levelLimBox->setCallback([this](float a) {
			properties->waterLevelLimit = a;
			reactor->setWaterLevelLimit(a);
		});
		water_levelLimBox->setEnabled(!DEMO_VERSION);
		water_levelLimBox->setEditable(!DEMO_VERSION);*/

		// Create a panel for rod reactivity plot visibility
		rel->appendRow(RelativeGridLayout::Size(27.5f, RelativeGridLayout::SizeType::Fixed));
		CustomWidget *checkBoxPanelRods = limits_tab->add<CustomWidget>();
		rel->setAnchor(checkBoxPanelRods, RelativeGridLayout::makeAnchor(1, 5));
		checkBoxPanelRods->setLayout(panelsLayout);
		checkBoxPanelRods->add<Label>("All control rods at once: ", "sans-bold");
		allRodsBox = checkBoxPanelRods->add<SliderCheckBox>();
		allRodsBox->setFontSize(16);
		allRodsBox->setChecked(properties->allRodsAtOnce);
		allRodsBox->setCallback([this](bool value)
								{ properties->allRodsAtOnce = value; });

		// Create a panel for rod reactivity plot visibility
		rel->appendRow(RelativeGridLayout::Size(27.5f, RelativeGridLayout::SizeType::Fixed));
		CustomWidget *checkBoxPanelAutoScram = limits_tab->add<CustomWidget>();
		rel->setAnchor(checkBoxPanelAutoScram, RelativeGridLayout::makeAnchor(1, 6));
		checkBoxPanelAutoScram->setLayout(panelsLayout);
		checkBoxPanelAutoScram->add<Label>("SCRAM after pulse: ", "sans-bold");
		autoScramBox = checkBoxPanelAutoScram->add<SliderCheckBox>();
		autoScramBox->setFontSize(16);
		autoScramBox->setChecked(properties->automaticPulseScram);
		autoScramBox->setCallback([this](bool value)
								  {
				properties->automaticPulseScram = value;
				reactor->setAutoScram(value); });
	}

	void createPulseTab()
	{
		CustomWidget *pulse_tab = tabControl->createTab("Pulse");
		pulse_tab->setId("pulse tab");
		RelativeGridLayout *rel = new RelativeGridLayout();
		rel->appendCol(RelativeGridLayout::Size(15.f, RelativeGridLayout::SizeType::Fixed)); // 0 left border
		rel->appendCol(2.f);																 // 1 pulse graph
		rel->appendCol(1.f);																 // 2 info panel
		rel->appendCol(RelativeGridLayout::Size(15.f, RelativeGridLayout::SizeType::Fixed)); // 3 right border
		rel->appendRow(RelativeGridLayout::Size(15.f, RelativeGridLayout::SizeType::Fixed)); // 0 top border
		rel->appendRow(1.f);																 // 1 content
		pulse_tab->setLayout(rel);

		pulseGraph = pulse_tab->add<CustomGraph>(4, "Last pulse");
		rel->setAnchor(pulseGraph, RelativeGridLayout::makeAnchor(0, 0, 2, 2));
		initializePulseGraph();

		standInCover = pulse_tab->add<CustomLabel>("Perform a pulse experiment to view data", "sans-bold", 35);
		rel->setAnchor(standInCover, RelativeGridLayout::makeAnchor(0, 0, 2, 2));
		standInCover->setTextAlignment(CustomLabel::TextAlign::HORIZONTAL_CENTER | CustomLabel::TextAlign::VERTICAL_CENTER);
		standInCover->setDrawBackground(true);
		standInCover->setBackgroundColor(Color(60, 255));
		standInCover->setColor(Color(255, 255));
		standInCover->setVisible(true);

		CustomWidget *dataSheet = pulse_tab->add<CustomWidget>();
		rel->setAnchor(dataSheet, RelativeGridLayout::makeAnchor(2, 1));

		RelativeGridLayout *rel2 = new RelativeGridLayout();
		rel2->appendCol(RelativeGridLayout::Size(15.f, RelativeGridLayout::SizeType::Fixed));  // 0 left border
		rel2->appendCol(RelativeGridLayout::Size(5.f, RelativeGridLayout::SizeType::Fixed));   // 1 color border
		rel2->appendCol(1.f);																   // 2 content
		rel2->appendCol(RelativeGridLayout::Size(15.f, RelativeGridLayout::SizeType::Fixed));  // 3 seperation border
		rel2->appendCol(RelativeGridLayout::Size(5.f, RelativeGridLayout::SizeType::Fixed));   // 4 color border
		rel2->appendCol(1.f);																   // 5 content
		rel2->appendRow(RelativeGridLayout::Size(100.f, RelativeGridLayout::SizeType::Fixed)); // 0 content
		rel2->appendRow(RelativeGridLayout::Size(15.f, RelativeGridLayout::SizeType::Fixed));  // 1 seperation border
		rel2->appendRow(RelativeGridLayout::Size(100.f, RelativeGridLayout::SizeType::Fixed)); // 2 content
		rel2->appendRow(RelativeGridLayout::Size(15.f, RelativeGridLayout::SizeType::Fixed));  // 3 bottom border
		rel2->appendRow(RelativeGridLayout::Size(20.f, RelativeGridLayout::SizeType::Fixed));  // 4 time interval
		rel2->appendRow(RelativeGridLayout::Size(20.f, RelativeGridLayout::SizeType::Fixed));  // 5 time labels
		rel2->appendRow(1.f);																   // 6 display label
		dataSheet->setLayout(rel2);

		Color colors[4] = {Color(255, 0, 0, 255), Color(0, 255), Color(0, 255, 0, 255), Color(255, 0, 255, 255)};
		std::string text[4] = {"Peak power", "FWHM", "Peak fuel temp.", "Pulse energy"};
		for (int i = 0; i < 4; i++)
		{
			CustomWidget *marker = dataSheet->add<CustomWidget>();
			rel2->setAnchor(marker, RelativeGridLayout::makeAnchor((i % 2) * 3 + 1, (i / 2) * 2));
			marker->setDrawBackground(true);
			marker->setBackgroundColor(colors[i]);

			CustomLabel *baseLabel = dataSheet->add<CustomLabel>(text[i], "sans-bold");
			rel2->setAnchor(baseLabel, RelativeGridLayout::makeAnchor((i % 2) * 3 + 2, (i / 2) * 2));
			baseLabel->setPadding(0, 7.f);
			baseLabel->setPadding(1, 5.f);
			baseLabel->setBackgroundColor(Color(32, 255));
			baseLabel->setDrawBackground(true);
			baseLabel->setTextAlignment(CustomLabel::TextAlign::TOP | CustomLabel::TextAlign::LEFT);
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
			pulseLabels[i]->setTextAlignment(CustomLabel::TextAlign::BOTTOM | CustomLabel::TextAlign::LEFT);
			rel3->setAnchor(pulseLabels[i], RelativeGridLayout::makeAnchor(0, 0));
		}

		pulseTimer = dataSheet->add<IntervalSlider>();
		rel2->setAnchor(pulseTimer, RelativeGridLayout::makeAnchor(1, 4, 5, 1));
		pulseTimer->setEnabled(false);
		pulseTimer->setHighlightColor(coolBlue);
		pulseTimer->setSteps(50U);
		for (int i = 0; i < 2; i++)
			pulseTimer->setCallback(i, [this](float /*change*/)
									{ updatePulseTrack(); });
		CustomLabel *infoLbl;
		for (int i = 0; i < 2; i++)
		{
			infoLbl = dataSheet->add<CustomLabel>(i ? "5s later" : "pulse start", "sans-bold");
			infoLbl->setColor(Color(255, 255));
			infoLbl->setFontSize(17.f);
			infoLbl->setPadding(1, 4);
			rel2->setAnchor(infoLbl, RelativeGridLayout::makeAnchor(1, 5, 5, 1, i ? Alignment::Maximum : Alignment::Minimum, Alignment::Minimum));
		}

		CustomWidget *displayPanel = dataSheet->add<CustomWidget>();
		rel2->setAnchor(displayPanel, RelativeGridLayout::makeAnchor(1, 6, 5, 1, Alignment::Middle, Alignment::Middle));
		displayPanel->setLayout(new BoxLayout(Orientation::Horizontal, Alignment::Fill));

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

		for (int i = 0; i < 4; i++)
		{
			pulseDisplayLabels[i]->setColor(Color(255, 255));
			pulseDisplayLabels[i]->setTextAlignment((i % 2) ? (CustomLabel::TextAlign::TOP | CustomLabel::TextAlign::LEFT) : (CustomLabel::TextAlign::VERTICAL_CENTER | CustomLabel::TextAlign::HORIZONTAL_CENTER));
		}
	}

	void createOtherTab()
	{
		CustomWidget *other_tab = tabControl->createTab("Other");
		other_tab->setId("other tab");
		RelativeGridLayout *rel = new RelativeGridLayout();
		rel->appendCol(RelativeGridLayout::Size(15.f, RelativeGridLayout::SizeType::Fixed));  // 0 left border
		rel->appendCol(RelativeGridLayout::Size(120.f, RelativeGridLayout::SizeType::Fixed)); // 1 save button
		rel->appendCol(RelativeGridLayout::Size(10.f, RelativeGridLayout::SizeType::Fixed));  // 2 border
		rel->appendCol(RelativeGridLayout::Size(120.f, RelativeGridLayout::SizeType::Fixed)); // 3 load button
		rel->appendCol(RelativeGridLayout::Size(10.f, RelativeGridLayout::SizeType::Fixed));  // 2 border
		rel->appendCol(RelativeGridLayout::Size(120.f, RelativeGridLayout::SizeType::Fixed)); // 5 division thing
		rel->appendRow(RelativeGridLayout::Size(15.f, RelativeGridLayout::SizeType::Fixed));  // 0 top border
		rel->appendRow(RelativeGridLayout::Size(30.f, RelativeGridLayout::SizeType::Fixed));  // 1 Load and save settings
		rel->appendRow(RelativeGridLayout::Size(15.f, RelativeGridLayout::SizeType::Fixed));  // 2 Seperating space
		rel->appendRow(RelativeGridLayout::Size(30.f, RelativeGridLayout::SizeType::Fixed));  // 3 Load and save log
		rel->appendRow(RelativeGridLayout::Size(15.f, RelativeGridLayout::SizeType::Fixed));  // 4 Seperating space
		rel->appendRow(RelativeGridLayout::Size(30.f, RelativeGridLayout::SizeType::Fixed));  // 5 Rod Curves
		rel->appendRow(RelativeGridLayout::Size(15.f, RelativeGridLayout::SizeType::Fixed));  // 6 Seperating space
		rel->appendRow(RelativeGridLayout::Size(30.f, RelativeGridLayout::SizeType::Fixed));  // 7 Reset simulator
		rel->appendRow(RelativeGridLayout::Size(15.f, RelativeGridLayout::SizeType::Fixed));  // 8 Seperating space
		rel->appendRow(RelativeGridLayout::Size(30.f, RelativeGridLayout::SizeType::Fixed));  // 9 Load script

		other_tab->setLayout(rel);

		Button *saveBtn = other_tab->add<Button>("Save settings");
		rel->setAnchor(saveBtn, RelativeGridLayout::makeAnchor(1, 1));
		saveBtn->setCallback([this]()
							 {
				toggleBaseWindow(false);
				std::string saveFileName = file_dialog({ { "rrs", "Simulator settings file" } }, true);
				if (saveFileName.substr((size_t)std::max(0, (int)saveFileName.length() - 4), std::min((size_t)4, saveFileName.length())) != ".rrs")
				{
					saveFileName = saveFileName.append(".rrs");
				}
				//saveSettings(saveFileName);
				saveArchive(saveFileName); });

		Button *loadBtn = other_tab->add<Button>("Load settings");
		rel->setAnchor(loadBtn, RelativeGridLayout::makeAnchor(3, 1));
		loadBtn->setCallback([this]()
							 {
				toggleBaseWindow(false);
				std::string loadFileName = file_dialog({ { "rrs", "Simulator settings file" } }, false);
				// loadSettingsFromFile(loadFileName);
				loadArchive(loadFileName);
				std::cout << "Graph size after GUI: " << properties->graphSize << std::endl;
				updateSettings(); });

		Button *saveLogBtn = other_tab->add<Button>("Save data");
		rel->setAnchor(saveLogBtn, RelativeGridLayout::makeAnchor(1, 3));
		saveLogBtn->setCallback([this]()
								{
				std::string logFileName = file_dialog(
					{ { "dat", "Data file" },{ "txt", "Text file" } }, true);
				reactor->dataToFile(logFileName); });

		CustomLabel *divisionLabel = other_tab->add<CustomLabel>("Save each steps:");
		rel->setAnchor(divisionLabel, RelativeGridLayout::makeAnchor(3, 3));

		IntBox<int> *divisionBox = other_tab->add<IntBox<int>>(reactor->data_division);
		rel->setAnchor(divisionBox, RelativeGridLayout::makeAnchor(5, 3));
		// divisionBox->setUnits("%");
		divisionBox->setDefaultValue(to_string(reactor->data_division));
		divisionBox->setFontSize(16);
		divisionBox->setFormat("[0-9]+");
		divisionBox->setSpinnable(true);
		divisionBox->setMinValue(1);
		divisionBox->setMaxValue(100);
		divisionBox->setValueIncrement(1);
		divisionBox->setCallback([this](int a)
								 { reactor->data_division = a; });

		Button *saveRodCurves = other_tab->add<Button>("Rod curves");
		rel->setAnchor(saveRodCurves, RelativeGridLayout::makeAnchor(1, 5));
		saveRodCurves->setCallback([this]()
								   {
				std::string logFileName = file_dialog(
					{ { "dat", "Data file" },{ "txt", "Text file" } }, true);
				reactor->rodsToFile(logFileName); });

		Button *loadScriptBtn = other_tab->add<Button>("Load script");
		rel->setAnchor(loadScriptBtn, RelativeGridLayout::makeAnchor(1, 9));
		loadScriptBtn->setCallback([this]()
								   {
				toggleBaseWindow(false);
				std::string loadFileName = file_dialog({ { "rrs", "Simulator Script file" } }, false);
				loadScriptFromFile(loadFileName); });

		Button *resetBtn = other_tab->add<Button>("Reset simulator");
		rel->setAnchor(resetBtn, RelativeGridLayout::makeAnchor(1, 7));
		resetBtn->setCallback([this]()
							  { this->resetSimToStart(); });
	}

	double trackerY[2] = {0., 1.};





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



public:
	double lastTime = get_seconds_since_epoch();


	double lastData = 0.;
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


	// Method for calculating autoscale factors

	bool prevToggle;
};
