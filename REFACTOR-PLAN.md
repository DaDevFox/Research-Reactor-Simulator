# Refactor Plan: Modular, Maintainable, and Future-Proof Frontend

## Principles

1. **Separation of Concerns**
   - UI, business logic, and data models must be separated.
   - Widgets should not directly manipulate simulation state.

2. **Component-Based Architecture**
   - Encapsulate UI elements as reusable, self-contained components.
   - Each widget (e.g., `CustomGraph`, `ControlRodDisplay`) should have a clear interface and minimal dependencies.

3. **Abstraction of GUI Framework**
   - Minimize direct dependencies on NanoGUI in application logic.
   - Use wrapper classes or interfaces for widgets and data providers.

4. **Event-Driven Design**
   - Use observer, signal/slot, or event bus patterns to decouple UI events from application logic.

5. **Configuration and Dependency Injection**
   - Pass dependencies (data models, controllers) into components, not hard-coded.

6. **Reusability and Testability**
   - Components should be reusable and easy to test in isolation.

7. **Framework Independence**
   - Minimize direct use of NanoGUI/NanoVG APIs in business logic to ease future migration.

---

## Concrete Refactoring Steps

### 1. **Custom Widgets and Panels**
- Refactor `CustomGraph`, `ControlRodDisplay`, `CustomImagePanel`, `Plot`, etc.:
  - Each should be a standalone class with a clear public API.
  - Abstract data sources: pass in data providers or observer interfaces.
  - Decouple rendering from data/model logic.

### 2. **Application Logic and State Management**
- Centralize state management in a controller/model class.
- Expose state changes via events/callbacks.
- Avoid direct widget-to-widget communication.

### 3. **GUI Construction and Layout**
- Use builder/factory patterns for UI construction.
- Group related widgets into logical panels/components.
- Use configuration objects for layout, not hard-coded values.

### 4. **Event Handling**
- Standardize event handling using observer or signal/slot patterns.
- Avoid inline lambdas for complex logic; use named callbacks or event dispatcher.

### 5. **Abstraction Layer for GUI**
- Introduce interfaces for data providers, event listeners, and widget factories.
- Example: `IDataProvider`, `IControlRodListener`.

---

## Example: Abstracting a Custom Widget

```cpp
// Interface for data provider
class IDataProvider {
public:
    virtual float getValue() const = 0;
};

// CustomGraph now depends on interface, not concrete simulation
class CustomGraph : public nanogui::Widget {
    const IDataProvider* dataProvider;
public:
    CustomGraph(const IDataProvider* provider) : dataProvider(provider) {}
    void draw(NVGcontext *ctx) override {
        float value = dataProvider->getValue();
        // draw graph
    }
};
```

---

## Example: Event Handling via Observer Pattern

```cpp
class IControlRodListener {
public:
    virtual void onControlRodPositionChanged(float value) = 0;
};

slider->setCallback([listener](float value) {
    listener->onControlRodPositionChanged(value);
});
```

---

## Directory Structure Suggestion

```
include/
  components/         # All custom widgets/components
  interfaces/         # Data providers, event listeners, etc.
  controllers/        # State management, business logic
src/
  components/
  controllers/
  main.cpp            # Application entry point
```

---

## Next Steps
- Audit all custom widgets for direct dependencies on NanoGUI and simulation state.
- Introduce interfaces/abstractions for data providers and event listeners.
- Refactor event handling to use observer or signal/slot patterns.
- Modularize GUI construction using factories or builders.
- Document component interfaces and usage patterns for maintainers.
