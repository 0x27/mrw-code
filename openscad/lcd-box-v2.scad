use <common-objects.scad>

// Set basic attributes
wallThickness = 2;
lidThickness = 3;
boxLengthInner = 132; 
boxWidthInner = 84;
boxHeightInner = 48;
screwFixingWidth = 8;
lipDepth = 3;
lcdHoleLength =92;
lcdHoleWidth = 39;
buttonWidth = 12;

// Some compound variables that are complicated enough
// to warrant their own variable
distanceOfButtonsFromEdge = (boxWidthInner-lcdHoleWidth-14-(buttonWidth/2))/2-wallThickness;
screwFixingHeight = boxHeightInner / 4;


// Create one of the ends
cube([wallThickness,boxWidthInner+(wallThickness*2),boxHeightInner]);



// Create the lips the 2 ends need for the wooden base to butt up to
translate([0,0,-lipDepth]) {
    cube([wallThickness,boxWidthInner+(wallThickness*2),lipDepth]);
    
    translate([boxLengthInner+wallThickness,0,0])
        cube([wallThickness,boxWidthInner+(wallThickness*2),lipDepth]);
}


// Create the 2 sides
translate([wallThickness, 0, 0]) cube([boxLengthInner,wallThickness,boxHeightInner]);
translate([wallThickness,boxWidthInner+wallThickness,0]) cube([boxLengthInner,wallThickness,boxHeightInner]);



// Create the screw fixings for the base. Initial translate(...) used to move the cursor inside the box
translate([wallThickness,wallThickness,0]) {
    translate([boxLengthInner/6 - (screwFixingWidth/2),0,0])
        screwFixing(screwFixingWidth, screwFixingHeight, screwHoleDiameter);
        
    translate([(boxLengthInner/6*5) - (screwFixingWidth/2),0,0])
        screwFixing(screwFixingWidth, screwFixingHeight, screwHoleDiameter);
        
    translate([boxLengthInner/6 - (screwFixingWidth/2),boxWidthInner-screwFixingWidth,0])
        screwFixing(screwFixingWidth, screwFixingHeight, screwHoleDiameter, 2);
    
    translate([((boxLengthInner/6*5) - (screwFixingWidth/2)),boxWidthInner-screwFixingWidth,0])
        screwFixing(screwFixingWidth, screwFixingHeight, screwHoleDiameter, 2);
}



// Create the other end, with a for the sockets to fit through
difference() {
translate([boxLengthInner+wallThickness,0,0])
    cube([wallThickness,boxWidthInner+(wallThickness*2),boxHeightInner]);
    
translate([boxLengthInner+wallThickness - 10,20,1]) // Doesn't matter about x dimensions very much
    cube([20,18,32]);
}



// Create the lid with holes cut out for the buttons and LCD.
difference() {
    difference() {
        difference() {
            difference() {
                difference() {
                    translate([wallThickness/2,wallThickness/2,boxHeightInner])
                        cube([boxLengthInner+(2*(wallThickness/2)),boxWidthInner+(2 * (wallThickness/2)),lidThickness]);
                    
                    translate([wallThickness,wallThickness,0]) 
                        translate([boxLengthInner/5-(buttonWidth/2),distanceOfButtonsFromEdge,boxHeightInner - 10])
                            cube([buttonWidth,buttonWidth,40]);
                };
                
                translate([wallThickness,wallThickness,0]) 
                    translate([(boxLengthInner/5)*2-(buttonWidth/2),distanceOfButtonsFromEdge,boxHeightInner - 10])
                        cube([buttonWidth,buttonWidth,40]);
            };
            
            translate([wallThickness,wallThickness,0]) 
                translate([(boxLengthInner/5)*3-(buttonWidth/2),distanceOfButtonsFromEdge,boxHeightInner - 10])
                    cube([buttonWidth,buttonWidth,40]);
        };
        
        translate([wallThickness,wallThickness,0])
            translate([boxLengthInner-(boxLengthInner/5)-(buttonWidth/2),distanceOfButtonsFromEdge,boxHeightInner - 10])
                cube([buttonWidth,buttonWidth,40]);
        
    };
    
    translate([wallThickness,wallThickness,0])
        translate([(boxLengthInner/2)-(lcdHoleLength/2),boxWidthInner-lcdHoleWidth-12,boxHeightInner - 10])
            cube([lcdHoleLength,lcdHoleWidth,40]);
}