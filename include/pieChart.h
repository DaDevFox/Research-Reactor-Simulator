#pragma once

#include <nanogui/widget.h>

class PieChart : public nanogui::Widget {
 protected:
  bool isDrawingRelative = true;
  int drawGroups = 6;
  float borderWidth = 0.f;
  nanogui::Color borderColor = nanogui::Color(255, 255);
  nanogui::Color dataColors[7] = {
      nanogui::Color(255, 255),         nanogui::Color(34, 116, 165, 255),
      nanogui::Color(247, 92, 3, 255),  nanogui::Color(241, 196, 15, 255),
      nanogui::Color(0, 204, 102, 255), nanogui::Color(240, 58, 71, 255),
      nanogui::Color(153, 0, 153, 255)};
  const size_t averageValues = 500;

 public:
  PieChart(Widget* parent);
  ~PieChart();

  float getBorderWidth() { return borderWidth; }
  nanogui::Color* getColors() { return dataColors; }
  void setBorderWidth(float width) { borderWidth = width; }
  void setBorderColor(nanogui::Color value) { borderColor = value; }
  void setDrawRelative(bool value) { isDrawingRelative = value; }
  /*void setData(Simulator* reactor) {
          mReactor = reactor;
  }*/

  virtual void draw(NVGcontext* ctx) override;
};
