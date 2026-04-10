#pragma once

#include <nanogui/window.h>

/**
 * \brief Popup window for combo boxes, popup buttons, nested dialogs etc.
 *
 * Usually the Popup instance is constructed by another widget (e.g. \ref
 * PopupButton) and does not need to be created by hand.
 */
class CustomPopup : public nanogui::Window {
 public:
  /// Create a new popup parented to a screen (first argument) and a parent
  /// window
  CustomPopup(nanogui::Widget* parent, nanogui::Window* parentWindow);

  /// Return the anchor position in the parent window; the placement of the
  /// popup is relative to it
  void setAnchorPos(const nanogui::Vector2i& anchorPos) {
    mAnchorPos = anchorPos;
  }
  /// Set the anchor position in the parent window; the placement of the popup
  /// is relative to it
  const nanogui::Vector2i& anchorPos() const { return mAnchorPos; }

  /// Set the anchor height; this determines the vertical shift relative to the
  /// anchor position
  void setAnchorHeight(int anchorHeight) { mAnchorHeight = anchorHeight; }
  /// Return the anchor height; this determines the vertical shift relative to
  /// the anchor position
  int anchorHeight() const { return mAnchorHeight; }

  /// Return the parent window of the popup
  nanogui::Window* parentWindow() { return mParentWindow; }
  /// Return the parent window of the popup
  const nanogui::Window* parentWindow() const { return mParentWindow; }

  /// Invoke the associated layout generator to properly place child widgets, if
  /// any
  virtual void performLayout(NVGcontext* ctx) override;

  /// Draw the popup window
  virtual void draw(NVGcontext* ctx) override;

  virtual void save(nanogui::Serializer& s) const override;
  virtual bool load(nanogui::Serializer& s) override;

 protected:
  /// Internal helper function to maintain nested window position values
  virtual void refreshRelativePlacement() override;

 protected:
  nanogui::Window* mParentWindow;
  nanogui::Vector2i mAnchorPos;
  int mAnchorHeight;
};
