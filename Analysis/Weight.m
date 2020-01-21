%% Weight Analysis for the Command Module Rev 6
% Last modified: B. Weinberg 20/JAN/2020

% Known weights
battery = 50; % g
liteAPRS = 8; % g
liteAPRSw = 9; % g
xbee_pro = 3; % g
teensy = 3; % g
pcb_density = 1.850; % g/cc
box_density = 0.9; % g/cc
iridium_modem = 36; % g

% Known Dimensions [length; width]
cell_tracker = distdim([1.65 1.6 0.01]./12, 'ft', 'm').*10; % cm
LVC = distdim([0.8 0.8 0.01]./12, 'ft', 'm').*10; % cm
bits = [4.5; 4.5] % cm
box = distdim([3.75 3.75 2.75]./12, 'ft', 'm').*10; % cm
box_thickness = distdim(0.25/12, 'ft', 'm').*10; % cm

% Calculated Weights
cell_weight = prod(cell_tracker) * pcb_density; 
LVC_weight = prod(LVC) * pcb_density; 
bits_weight = prod(bits) * pcb_density; 
box_weight = (prod(box) - prod(box - box_thickness)) * box_density;

total_battery = 4 * battery; 
heaviest_aprs = 2 * liteAPRSw; 
total_teensy = 2 * teensy; 

% Total Weight 
total_electronics = sum([cell_weight, LVC_weight, bits_weight, total_battery, heaviest_aprs, total_teensy, xbee_pro, iridium_modem]);

total_weight = sum([total_electronics, box_weight])