#pragma once
#include <nanogui/nanogui.h>
#include "../../include/Settings.h"
#include "../../include/Simulator.h"

// INFO: V1 (see PLAN.md)
class Pane {
public:
// sets place in "console space" which the user can navigate through directionally
 virtual nanogui::Vector2i consoleCoordinates() = 0;
 virtual nanogui::Vector2i consoleDimensions() = 0;

  virtual void show(CustomWindow&) = 0;
  virtual void hide(CustomWindow&) = 0;

  Pane(Settings *properties, Simulator *reactor) : properties(properties), reactor(reactor) {}
  virtual ~Pane();

  Settings *properties;
  Simulator *reactor;
};
