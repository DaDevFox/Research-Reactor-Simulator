#include <nanogui/layout.h>

#include <vector>

class RelativeGridLayout : public nanogui::Layout {
 public:
  enum FillMode { Always, IfLess, IfMore };
  enum SizeType { Fixed, Relative };

  struct Anchor {
    int pos[2] = {0, 0};
    int span[2] = {1, 1};
    nanogui::Vector4i padding;
    FillMode expansion[2] = {FillMode::Always, FillMode::Always};
    nanogui::Alignment alignment[2] = {nanogui::Alignment::Fill,
                                       nanogui::Alignment::Fill};

    Anchor() { padding = nanogui::Vector4i(0, 0, 0, 0); }

    Anchor(int x, int y) : Anchor() {
      pos[0] = x;
      pos[1] = y;
    }

    Anchor(int x, int y, int w, int h) : Anchor(x, y) {
      span[0] = w;
      span[1] = h;
    }

    Anchor(int x, int y, int w, int h, nanogui::Vector4i padding_)
        : Anchor(x, y, w, h) {
      padding = padding_;
    }

    void setAlignment(nanogui::Alignment alignX, nanogui::Alignment alignY) {
      alignment[0] = alignX;
      alignment[1] = alignY;
    }

    void setFillMode(FillMode modeX, FillMode modeY) {
      expansion[0] = modeX;
      expansion[1] = modeY;
    }
  };
  struct Size {
   protected:
    float value_;
    SizeType type_;

   public:
    Size(float value = 1.f, SizeType type = SizeType::Relative) {
      value_ = value;
      type_ = type;
    }
    const float& value() const { return value_; }
    const SizeType& type() const { return type_; }
  };

  RelativeGridLayout() {
    mTable[0] = std::vector<Size>();
    mTable[1] = std::vector<Size>();
  };

  int margin() const { return mMargin; }
  void setMargin(int margin) { mMargin = margin; }

  /// Return the number of cols
  int colCount() const { return (int)mTable[0].size(); }

  /// Return the number of rows
  int rowCount() const { return (int)mTable[1].size(); }

  size_t appendRow(float size) {
    return appendRow(Size(size, SizeType::Relative));
  }
  size_t appendCol(float size) {
    return appendCol(Size(size, SizeType::Relative));
  }

  /// Append a row of the given size (and stretch factor)
  size_t appendRow(Size rowSize) {
    mTable[1].push_back(rowSize);
    if (rowSize.type() == SizeType::Relative) {
      relativeSum[1] += rowSize.value();
    } else {
      fixedSum[1] += rowSize.value();
    };
    return mTable[1].size() - 1;
  }

  /// Append a column of the given size (and stretch factor)
  size_t appendCol(Size columnSize) {
    mTable[0].push_back(columnSize);
    if (columnSize.type() == SizeType::Relative) {
      relativeSum[0] += columnSize.value();
    } else {
      fixedSum[0] += columnSize.value();
    };
    return mTable[0].size() - 1;
  }

  /// Set the stretch factor of a given row
  void setRowSize(int index, Size rowSize) {
    if (mTable[1].at(index).type() == SizeType::Relative) {
      relativeSum[1] -= mTable[1].at(index).value();
    } else {
      fixedSum[1] -= mTable[0].at(index).value();
    }
    if (rowSize.type() == SizeType::Relative) {
      relativeSum[1] += rowSize.value();
    } else {
      fixedSum[1] += rowSize.value();
    };
    mTable[1].at(index) = rowSize;
  }

  /// Set the stretch factor of a given column
  void setColSize(int index, Size columnSize) {
    if (mTable[0].at(index).type() == SizeType::Relative) {
      relativeSum[0] -= mTable[0].at(index).value();
    } else {
      fixedSum[0] -= mTable[0].at(index).value();
    }
    if (columnSize.type() == SizeType::Relative) {
      relativeSum[0] += columnSize.value();
    } else {
      fixedSum[0] += columnSize.value();
    };
    mTable[0].at(index) = columnSize;
  }

  /// Specify the anchor data structure for a given widget
  void setAnchor(const nanogui::Widget* widget, const Anchor& anchor) {
    mAnchor[widget] = anchor;
  }

  /// Retrieve the anchor data structure for a given widget
  Anchor anchor(const nanogui::Widget* widget) const {
    auto it = mAnchor.find(widget);
    if (it == mAnchor.end()) {
      Anchor a = Anchor(0, 0);
      a.setAlignment(nanogui::Alignment::Minimum, nanogui::Alignment::Minimum);
      return a;
    } else {
      return it->second;
    }
  }

  /* Implementation of the layout interface */
  virtual nanogui::Vector2i preferredSize(
      NVGcontext* ctx, const nanogui::Widget* widget) const override;
  virtual void performLayout(NVGcontext* ctx,
                             nanogui::Widget* widget) const override;

  static Anchor makeAnchor(int x, int y, int w = 1, int h = 1,
                           nanogui::Alignment alignX = nanogui::Alignment::Fill,
                           nanogui::Alignment alignY = nanogui::Alignment::Fill,
                           FillMode modeX = FillMode::Always,
                           FillMode modeY = FillMode::Always);

 protected:
  nanogui::Vector2f positionOf(const nanogui::Widget* widget, size_t x,
                               size_t y) const;
  nanogui::Vector4i boundsOf(const nanogui::Widget* widget, size_t x, size_t y,
                             size_t w, size_t h) const;

  std::vector<Size> mTable[2];
  float relativeSum[2] = {0.f, 0.f};
  float fixedSum[2] = {0.f, 0.f};
  std::unordered_map<const nanogui::Widget*, Anchor> mAnchor;
  int mMargin = 0;
};
