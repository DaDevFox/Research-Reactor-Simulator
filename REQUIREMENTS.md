Line items are referred to by their number across project documentation. Numbers of the form (X) indicate estimated difficulty associated with a task (just a rough heuristic – definitely changeable during the course of develeopment). [LP] indicates a requirement item was noted as lower priority. The numbering is roughly ranked by priority (aside from overall changes in item #1). 



Overall changes
- 1.1) (1) Fix inconsistent font sizes, obscured labels, and layout glitches
- 1.2) (4) Undertake full-scale style redesign (UI refresh)
    - 1.2.1) (4) Match coors/panel style to MUTR
- 1.3) (8) Create panelling system + 3 distinct panels (Left, Center, and Right)

## Reactor Power & Temperature Readout (Center Panel)
- 2.1) (4) Apply 90 degree rotation + scale of layout for top-to-bottom data propagation.
- 2.2) (4) Show scaling (xScale for CIC) as a background line graph with a red-colored series
- 2.3) (1) Add Source Indicator Light
- 2.4) (2) Log Power: Apply calculation to represent data in volts (apply hardware component calculation which converts power to voltage; apply log scaling afterward)
    - 2.4.1) (1) Log Power Calculation: % Power=(10^(Volts-8))
- 2.5) (1) CIC Power: Follows CIC scale knob behavior
- 2.6) (1) FC Power: (0-100%) on a linear scale

## Reactor Mode Control (Center Panel)
- 3.1) (4) Dial switch for toggling between: 
- 3.1.1) Steady State Mode (Manual – maintain constant power)
- 3.1.2) Automatic Mode for system-regulated output
- 3.2) (1) Simplify display by removing unnecessary extras
- 3.3) [LP] (4) Clean up simulation core to remove redundant elements

## Control Rod Buttons (Center Panel)
- 4.1) (2) Add control rod manipulation control panel:
    - 4.1.1) (6) 5×3 (15 button spaces/19 buttons) layout; on/contact split for latter 4 in first row (see 3.1.2)
    - 4.1.2) (2) First Row Buttons:
      - 4.1.2.1) Orange: “On” status (magnetic activation check – should turn off after SCRAM and back on after RESET)
      - 4.1.2.2) Blue: “Contact” status (rod position matching drive mechanism check)
    - 4.1.3) (1) Second/Third Row Buttons:
      - 4.1.3.1) Second row: up button
      - 4.1.3.2) Third row: down button
- 4.2) (1) Manual SCRAM button control at top of control panel

## Neutron Flux Monitor (Left Panel)
- 5.1) (7) Add to left panel (Panel 0)
    - 5.1.1) [LP] (5) Power range % vertical indicator
    - 5.1.2) (5) Reactor period vertical range indicator
    - 5.1.3) [LP] (5) Reactor log power % vertical range indicator
    - 5.1.4) [LP] (5) Source range period vertical range indicator
    - 5.1.5) [LP] (5) Neutron count rate vertical range indicator
- 5.2) (5) Render as popout in Center Panel (toggleable)
    - 5.2.1) (4) Make collapsible/expandable with arrow icon buttons

## Annunciators Control Panel (Center Panel)
- 6.1) (2) Add annunciators control panel 
    - 6.1.1) (2) 3x3 (9 indicators) layout
- 6.2) (1) SCRAM indicators to move:
    - 6.2.1) Power
    - 6.2.2) Water temperature
    - 6.2.3) Fuel temperature
    - 6.2.4) Manual
    - 6.2.5) Period
- 6.3) (1) Dummy SCRAM indicators to add:
    - 6.3.1) External
    - 6.3.2) High Voltage
    - 6.3.3) Duplicate power into Safety Control 1, 2
- 6.4) Source Indicator Light: illuminates when count rate is less than 1 CPS. Since the simulator doesn't have a count rate, we can have it come on when the power is less than 10^-7% of full power. When the light is on, the control rods can't be withdrawn

## Source & Cooling Controls (Center Panel)
- 7.1) (2) Add interactive control panel
    - 7.1.1) (2) 3x3 (9 buttons/indicators) layout
- 7.2) (2) Interactive controls:
    - 7.2.1) Source control: This is nominally the same as turning the source on and off in the existing simulator. I am not sure how they implemented it though
    - 7.2.1) Pump control: We can treat the pump controls as the same as the cooling system in the existing simulator. The Diffuser Pump Button can just be a dummy
- 7.3) [LP] (1) Dummy controls:
    - 7.3.1) 2 other controls [hold for more info]
    - 7.3.2) PC Pump On / Off Buttons - Same as existing cooling in existing simulator 
    - 7.3.3) Diffuser Pump On / Off - Dummy
    - 7.3.4) Fans On / Off - Dummy
    - 7.3.5) Source In / Out - Same as source in existing simulator 
    - 7.3.6) Flow - Dummy

## Temperature Gauges (Right Panel)
- 8.1) (5) Display three separate temperature readouts:
    - 8.1.1) 3 for water temperature
    - 8.1.2) 3 for fuel temperature
    - 8.1.3) [LP] (1) Add simulated noise to readings (via urandom or Perlin noise)
- 8.2) (4) Classic needle gauge to show percentage of licensed power – 250KW; marked Ion Chamber on physical reactor console
- 8.3) (3) Pool Level and Conductivity are also displayed here. For right now, these can be dummys
- 8.4) (3) Flow Rate is also displayed here. This can be 0 when cooling is off and 120 when it is on

## Radiation Monitoring (Left Panel)
- 9.1 [LP] (3) Calculate and display power-proportional radiation metrics
- 9.2 [LP] (1) Add simulated noise to output values
