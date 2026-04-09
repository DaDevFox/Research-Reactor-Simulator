#pragma once

/*
        SimulatorGUI.cpp draws the main
        GUI screen
*/
#define NOMINMAX
#define SCROLL_BAR_THICKNESS 12
#define SCROLL_BAR_ROUND .8f

#include <Simulator.h>
#include <nanogui/button.h>
#include <nanogui/checkbox.h>
#include <nanogui/colorwheel.h>
#include <nanogui/combobox.h>
#include <nanogui/common.h>
#include <nanogui/entypo.h>
#include <nanogui/graph.h>
#include <nanogui/imagepanel.h>
#include <nanogui/imageview.h>
#include <nanogui/label.h>
#include <nanogui/layout.h>
#include <nanogui/messagedialog.h>
#include <nanogui/popupbutton.h>
#include <nanogui/progressbar.h>
#include <nanogui/screen.h>
#include <nanogui/slider.h>
#include <nanogui/tabwidget.h>
#include <nanogui/textbox.h>
#include <nanogui/toolbutton.h>
#include <nanogui/vscrollpanel.h>
#include <nanogui/window.h>

#include "../include/CustomGraph.h"
#include "../include/CustomWindow.h"
#include "../include/DataDisplay.h"
#include "../include/IntervalSlider.h"
#include "../include/ReactivityDisplay.h"
#include "../include/SliderCheckBox.h"
#include "../include/controlRodDisplay.h"
#include "../include/pieChart.h"
#include "CustomLabel.h"
#include "CustomWidget.h"
#if defined(_WIN32)
#include <windows.h>
#endif
#include <CustomTabWidget.h>
#include <Icon.h>
#include <math.h>
#include <nanogui/glutil.h>
#include <nanogui/nanogui.h>

#include <algorithm>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "../include/Plot.h"
#include "../include/RelativeGridLayout.h"
#include "../include/SerialClass.h"
#include "../include/Settings.h"
#include "../include/SimulatorGuiApp.h"
#include "../include/SimulatorGuiUiHelpers.h"
#include "../include/UiConfigProvider.h"
#include "../include/fileDialog.h"
#include "../include/util.h"

/* Resolution formats supported:
 *	HD 720	(1280 x 720)
 *	WXGA	(1280 x 768)
 *	laptop	(1366 x 768)
 *	SXGA	(1280 x 1024)
 *	WXGA	(1280 x 800)
 *	better	(1200+ x 700+)
 */
#define WINDOW_DEFAULT_WIDTH 1280  // HD 720p width
#define WINDOW_DEFAULT_HEIGHT 720  // HD 720p height

#define WINDOW_ICON_NUM 7  // Number of icon formats

// SIMULATOR VERSION
// major.minor.revision.build
#define VERSION_MAJOR 1
#define VERSION_MINOR 4
#define VERSION_REVISION 1
#define VERSION_BUILD 1  // Setting this to 0 doesn't display the build number

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

class Pane;

class SimulatorGUI : public nanogui::Screen {
 private:
  const string box_auth = BOX_ID;
  pair<bool, bool> isZero = pair<bool, bool>(false, false);
  double simulationTimes[SIM_TIME_FACTOR_NUMBER] = {
      0.01, 0.05, 0.1, 0.2, 0.3, 0.4, 0.5, 0.75, 1.,   1.5, 2.,
      4.,   5.,   7.5, 10., 15., 20., 50., 100., 250., 500.};
  const std::vector<string> modes = {
      "Manual",
      //"Square wave","Sine wave","Saw tooth",
      "Automatic"
      //, "Pulse"
  };
  const std::vector<string> ns_modes = {"Constant", "Square wave", "Sine wave",
                                        "Saw tooth"};
  const std::vector<string> sineModes = {"Normal", "Quadratic"};
  string sqw_settingNames[4] = {"Wave up start: ", "Wave up end: ",
                                "Wave down start: ", "Wave down end: "};
  string saw_settingNames[6] = {
      "Tooth up start: ",   "Tooth up peak: ",   "Tooth up end: ",
      "Tooth down start: ", "Tooth down peak: ", "Tooth down end: "};
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
  const std::string version();

#if defined(_WIN32)
  Serial *theBox = nullptr;
#endif
  Simulator *reactor = nullptr;
  CustomGraph *canvas = nullptr;
  CustomGraph *delayedGroupsGraph = nullptr;
  CustomGraph *pulseGraph = nullptr;
  CustomGraph *sourceGraph = nullptr;
  BoxLayout *layout;
  CustomWindow *baseWindow = nullptr;
  RelativeGridLayout *relativeLayout = nullptr;  // layout for the main window
  CustomLabel *fpsLabel = nullptr;
  Plot *reactivityPlot = nullptr;
  Plot *rodReactivityPlot = nullptr;
  Plot *powerPlot = nullptr;
  Plot *temperaturePlot = nullptr;
  Plot *delayedGroups[6] = {};
  Plot *pulsePlots[4];
  ToolButton *slowDown;
  ToolButton *playPause;
  ToolButton *speedUp;
  size_t selectedTime = 8;
  BezierCurve *rodCurves[NUMBER_OF_CONTROL_RODS] = {};
  Plot *rodDerivatives[NUMBER_OF_CONTROL_RODS] = {};
  CustomTabWidget *tabControl = nullptr;

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
  PeriodDisplay *periodDisplay = nullptr;
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
  IntervalSlider *displayTimeSlider = nullptr;
  SliderCheckBox *timeLockedBox = nullptr;

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

  CustomLabel *timeLabel = nullptr;
  CustomLabel *simStatusLabel;
  CustomLabel *simFactorLabel;

  CustomLabel *pulseLabels[4];
  CustomLabel *pulseDisplayLabels[4];
  CustomLabel *standInCover;

  DataDisplay<double> *powerShow = nullptr;
  DataDisplay<float> *reactivityShow = nullptr;
  DataDisplay<float> *rodReactivityShow = nullptr;
  DataDisplay<float> *temperatureShow = nullptr;
  DataDisplay<double> *periodShow = nullptr;
  DataDisplay<double> *waterTemperatureShow = nullptr;
  DataDisplay<double> *waterLevelShow;

  Plot *operationModes[3];
  Plot *operationModesTrackers[3];

  std::shared_ptr<IUiConfigProvider> uiConfigProvider;
  UiStyleConfig uiStyleConfig;
  UiPanelLayoutConfig uiPanelLayoutConfig;
  Color coolBlue;

  BoxLayout *panelsLayout = nullptr;

  std::vector<std::unique_ptr<Pane>> panes;
  std::map<std::pair<int, int>, Pane *> paneLookup;
  Pane *activePane = nullptr;

  uint16_t LEDstatus = 0;
  bool boxConnected = false;
  void setSimulationTime(size_t time);

  void playPauseSimulation(bool play);

  void updateSimulationIcon();

  void initializeSimulator();

  void initializePanes();

  bool navigatePane(int dx, int dy);

  void updatePulseTrack(bool updateData = false);

  void viewingIntervalChanged(bool firstChanged);

  void createDataDisplays(Widget *parent, RelativeGridLayout *rLayout);

  void initializeGraph();

#if defined(_WIN32)
  vector<string> comPorts;
  vector<string> lastCOMports;
  void initializeSerial();

  void updateCOMports();

  void tryConnectingTo(string port);

  void initializePulseGraph();

  SimulatorGUI(std::shared_ptr<IUiConfigProvider> configProvider = nullptr);

  void createBottomPanel();

  void createMainTab();

  void createGraphSettingsTab();

  void createPhysicsSettingsTab();

  void createRodSettingsTab();

  void createDelayedGroupsTab();

  double *operationModesPlots[2][3];
  const int simModeFields[3] = {7, 3, 8};
  void createOperationModesTab();

  const std::string labels[5] = {"Period", "Power", "Fuel temperature",
                                 "Water temperature", "Water level"};
  const Simulator::ScramSignals reasons[5] = {
      Simulator::ScramSignals::Period, Simulator::ScramSignals::Power,
      Simulator::ScramSignals::FuelTemperature,
      Simulator::ScramSignals::WaterTemperature,
      Simulator::ScramSignals::WaterLevel};
  void createOperationalLimitsTab();

  void createPulseTab();

  void createOtherTab();

  double trackerY[2] = {0., 1.};
  void resetSimToStart();

  void handleDerivativeChange();

  void hardcoreMode(bool value);

  template <typename WidgetClass, typename... Args>
  WidgetClass *makeSettingLabel(Widget *parent, std::string text,
                                int fixedWidth = 0, const Args &...args) {
    Widget *panel = parent->add<Widget>();
    panel->setLayout(panelsLayout);
    CustomLabel *temp = panel->add<CustomLabel>(text, "sans-bold");
    if (fixedWidth) temp->setFixedWidth(fixedWidth);
    return panel->add<WidgetClass>(args...);
  }
  IntBox<int> *makeSimulationSetting(CustomWidget *parent, int initialValue,
                                     std::string text);

  void handleDebugChanged();

  ~SimulatorGUI();

  virtual bool keyboardEvent(int key, int scancode, int action, int modifiers);

  virtual bool resizeEvent(const Eigen::Vector2i &size);

  // DEBUG: Override to trace mouse events
  virtual bool mouseButtonEvent(const nanogui::Vector2i &p, int button,
                                bool down, int modifiers) override {
    static std::ofstream debugLog("debug_clicks.txt", std::ios::app);
    debugLog << "[DEBUG] mouseButtonEvent at (" << p.x() << ", " << p.y()
             << ") button=" << button << " down=" << down << std::endl;
    Widget *target = findWidget(p);
    if (target) {
      debugLog << "[DEBUG] findWidget found: " << typeid(*target).name()
               << std::endl;
    } else {
      debugLog << "[DEBUG] findWidget found: nullptr" << std::endl;
    }
    bool result = Screen::mouseButtonEvent(p, button, down, modifiers);
    debugLog << "[DEBUG] Screen::mouseButtonEvent returned: " << result
             << std::endl;
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
  int cheat1[7] = {GLFW_KEY_G, GLFW_KEY_O, GLFW_KEY_D, GLFW_KEY_M,
                   GLFW_KEY_O, GLFW_KEY_D, GLFW_KEY_E};
  int cheat2[5] = {GLFW_KEY_D, GLFW_KEY_E, GLFW_KEY_B, GLFW_KEY_U, GLFW_KEY_G};
  int cheat3[5] = {GLFW_KEY_R, GLFW_KEY_E, GLFW_KEY_S, GLFW_KEY_E, GLFW_KEY_T};
  bool debugMode = false;
  deque<int> last10keys = deque<int>();
  size_t displayInterval[2] = {0, 0};
  bool btns[11];
  int lastModeState = 0;
  void updateAlphaGraph();

  std::string getTimeSinceStart();

 public:
  double lastTime = get_seconds_since_epoch();
  virtual void draw(NVGcontext *ctx);

  double lastData = 0.;
  void handleBox();

#endif
  bool shouldUpdateNeutronSource = false;
  void updateNeutronSourceTab();

  void handleBoxData(uint16_t box_data, double now);

  // static string formatDecimals(const double x, const int decDigits, bool
  // removeTrailingZeros = true)
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
  void reculculateDisplayInterval(double fromTime, double toTime);

  pair<int, int> recalculatePowerExtremes(double fromTime = 0.,
                                          double toTime = 0.);

  std::vector<string> getCOMports();

  void saveArchive(std::string path);

  void loadArchive(std::string path);

  void loadScriptFromFile(std::string path);

  void updateSettings(bool updateReactor = true);

  bool prevToggle;
  void toggleBaseWindow(bool value);
};

int runSimulatorGuiApp(int argc, char **argv);
