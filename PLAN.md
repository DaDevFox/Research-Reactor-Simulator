# Pane/Control System

## system frame

### v1

Panes compile-time configured (via inheritance/composition) to simply show/hide

### v2

Panes compile-time configured to include/exclude controls, which show/hide, in trees of layouts/nested controls

### v3

Panes can take runtime/loaded configuration for positions, control memberships

## Pane contents

### Lower-Left

Radiation Area Monitor Annunciators:
- 3x3 grid of pairs of high-low indicator lights (18 total) with grey bg when unlit, green when lit
- boxed console software unit of 6 readings for radiation levels 

### Lower-Center
Upright Panel:
- Shim 1, Shim 2, Regulator rod % withdrawns as 3 radial gauges with fill progress, black bg, green fill; 12.5% major markings, 2% minor markings, ruler markings filled as progress; scale labels at 25%, actual value in large bottom right, percent value in medium top left
- Graph to right (part of same group) as above
^ 4, all horizontally collinear (share vertical mean position)

Upright Panel right-side:
- Pump Controls
- Automatic Power settings (xScale)

Lower Panel:
- middle sections at 50% width
    - Manual Scram Button full-width
    - 3 rows of 5 boxes, each with paired left-right backlit buttons
    - rightmost 3 of the first row are control rod up/downs (blue/orange respectively)
- right section at 30% width
  - CIC scale power control

### Lower-Right

3 controls in row
- Needle gauge: reactor percent power
- Simple number gauge: pool temperature
- Simple number gauge: fuel temperature

### Upper-Left
- 5 horizontally split vertical progress indicators
  - group of 2 (source range)
    - neutron count
    - source range period
  - group of 2 (wide range)
    - reactor log power 
    - reactor period
  - group of 1 (power range)
    - reactor power percentage

### Upper-Center

Fullscreen view of main graph (as in Lower-Center)

### Upper-Right

in left 50%:
- Radial Gauges as in lower-center:
    - Pool Conductivity 
    - Flow Rate

in right 50% (centered grid layout):
    fuel temperature        pool level
    pool temp 1             pool temp 2 (slight variance to pool temp 1)



