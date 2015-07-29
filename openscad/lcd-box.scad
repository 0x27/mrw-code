// Set basic attributes
wallThickness = 2;
lidThickness = 3;
boxLength = 139;
boxWidth = 87;
boxHeight = 51;
screwFixingWidth = 8;
lipDepth = 3;
lcdHoleLength =92;
lcdHoleWidth = 39;
buttonWidth = 12;
lidInStep = 2;


// Create one of the ends
cube([wallThickness,boxWidth+wallThickness,boxHeight]);



// Create the lips the 2 ends need for the wooden base to butt up to
translate([0,0,-lipDepth]) cube([wallThickness,boxWidth+wallThickness,lipDepth]);
translate([boxLength,0,0]) translate([0,0,-lipDepth]) cube([wallThickness,boxWidth+wallThickness,lipDepth]);



// Create the 2 sides
cube([boxLength,wallThickness,boxHeight]);
translate([0,boxWidth,0]) cube([boxLength+wallThickness,wallThickness,boxHeight]);



// Create the screw fixings for the base
translate([boxLength/8,wallThickness,0]) cube([screwFixingWidth,screwFixingWidth,boxHeight / 3]);
translate([boxLength - (boxLength/8)- wallThickness, wallThickness,0]) cube([screwFixingWidth,screwFixingWidth,boxHeight / 3]);
translate([boxLength/8,boxWidth-screwFixingWidth,0]) cube([screwFixingWidth,screwFixingWidth,boxHeight / 3]);
translate([boxLength - (boxLength/8)- wallThickness,boxWidth-screwFixingWidth,0]) cube([screwFixingWidth,screwFixingWidth,boxHeight / 3]);


// Create the other end, with a for the sockets to fit through
difference() {
translate([boxLength,0,0]) cube([wallThickness,boxWidth+wallThickness,boxHeight]);
translate([boxLength - 10,16,0]) cube([20,16,35]);
}



// Create the lid with holes cut out for the buttons and LCD
difference() {
    difference() {
        difference() {
            difference() {
                difference() {
                    translate([lidInStep,lidInStep,boxHeight]) cube([boxLength+wallThickness-(2*lidInStep),boxWidth+          wallThickness-(2 * lidInStep),lidThickness]);
                    translate([boxLength/5,(boxWidth-lcdHoleWidth-12-(buttonWidth/2))/2-wallThickness,boxHeight - 10]) cube([buttonWidth,buttonWidth,40]);
                };
                translate([(boxLength/5)*2,(boxWidth-lcdHoleWidth-12-(buttonWidth/2))/2-wallThickness,boxHeight - 10]) cube([buttonWidth,buttonWidth,40]);
            };
            translate([(boxLength/5)*3,(boxWidth-lcdHoleWidth-12-(buttonWidth/2))/2-wallThickness,boxHeight - 10]) cube([buttonWidth,buttonWidth,40]);
        };
        translate([boxLength-(boxLength/5),(boxWidth-lcdHoleWidth-12-(buttonWidth/2))/2-wallThickness,boxHeight - 10]) cube([buttonWidth,buttonWidth,40]);
    };
    translate([(boxLength/2)-(lcdHoleLength/2),boxWidth-lcdHoleWidth-12,boxHeight - 10]) cube([lcdHoleLength,lcdHoleWidth,40]);
}