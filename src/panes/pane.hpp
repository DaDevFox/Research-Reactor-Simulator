#pragma once
#include <nanogui/nanogui.h>
#include <spdlog/spdlog.h>
#include <type_traits>
#include "../../include/Settings.h"
#include "../../include/Simulator.h"
#include "../../include/CustomWindow.h"

// INFO: V1 (see PLAN.md)
class Pane {
public:
// sets place in "console space" which the user can navigate through directionally
 virtual nanogui::Vector2i consoleCoordinates() = 0;
 virtual nanogui::Vector2i consoleDimensions() = 0;

  virtual void show(CustomWindow&) = 0;
  virtual void hide(CustomWindow&) = 0;

  Pane(Settings *properties, Simulator *reactor) : properties(properties), reactor(reactor) {}
  virtual ~Pane() = default;

  Settings *properties;
  Simulator *reactor;

protected:
  template <typename T>
  static void safeRemoveFromBaseWindow(CustomWindow &baseWindow, T *&widget,
                                       const char *tag) {
    static_assert(std::is_base_of<nanogui::Widget, T>::value,
                  "safeRemoveFromBaseWindow requires Widget-derived types");
    if (!widget) return;

    nanogui::Widget *raw = widget;
    const bool isChild = (raw->parent() == &baseWindow) &&
                         (baseWindow.childIndex(raw) >= 0);
    if (!isChild) {
      spdlog::warn("{}: skip removeChild; widget is not a baseWindow child",
                   tag);
      widget = nullptr;
      return;
    }

    baseWindow.removeChild(raw);
    widget = nullptr;
  }
};
