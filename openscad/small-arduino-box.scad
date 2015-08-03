// Set basic attributes
wallThickness = 2;
lidThickness = 3;
boxLengthInner = 81; 
boxWidthInner = 78;
boxHeightInner = 38;
screwFixingWidth = 8;
lipDepth = 3;
lidInStep = 1;

// NOTE - NOT USED IN THIS PROJECT BUT SETTTING THEM TO ZERO
// HAS THE SAME EFFECT AS REMOVING THE LOGIC FOR THEM. MIGHT
// WANT TO ADD BUTTONS BACK IN ONE DAY SO LEAVING LOGIC IN PLACE
lcdHoleLength =0;
lcdHoleWidth = 0;
buttonWidth = 0;


// Create one of the ends
cube([wallThickness,boxWidthInner+(wallThickness*2),boxHeightInner]);



// Create the lips the 2 ends need for the wooden base to butt up to
translate([0,0,-lipDepth])
    cube([wallThickness,boxWidthInner+(wallThickness*2),lipDepth]);
    
translate([0,0,-lipDepth])
    translate([boxLengthInner+wallThickness,0,0])
        cube([wallThickness,boxWidthInner+(wallThickness*2),lipDepth]);



// Create the 2 sides
translate([wallThickness, 0, 0]) cube([boxLengthInner,wallThickness,boxHeightInner]);
translate([wallThickness,boxWidthInner+wallThickness,0]) cube([boxLengthInner,wallThickness,boxHeightInner]);



// Create the screw fixings for the base. Initial translate(...) used to move the cursor inside the box
translate([wallThickness,wallThickness,0])
    translate([boxLengthInner/6 - (screwFixingWidth/2),0,0])
        cube([screwFixingWidth,screwFixingWidth,boxHeightInner / 3]);
        
translate([wallThickness,wallThickness,0]) 
    translate([((boxLengthInner/6*5) - (screwFixingWidth/2)),0,0])
        cube([screwFixingWidth,screwFixingWidth,boxHeightInner / 3]);
        
        
translate([wallThickness,wallThickness,0])
    translate([boxLengthInner/6-(screwFixingWidth/2),boxWidthInner-screwFixingWidth,0])
        cube([screwFixingWidth,screwFixingWidth,boxHeightInner / 3]);

translate([wallThickness,wallThickness,0])
    translate([((boxLengthInner/6*5) - (screwFixingWidth/2)),boxWidthInner-screwFixingWidth,0])
        cube([screwFixingWidth,screwFixingWidth,boxHeightInner / 3]);


// Create the other end, with a for the sockets to fit through
difference() {
translate([boxLengthInner+wallThickness,0,0])
    cube([wallThickness,boxWidthInner+(wallThickness*2),boxHeightInner]);
    
translate([boxLengthInner+wallThickness - 10,20,1]) // Doesn't matter about x dimensions very much
    cube([20,18,32]);
}



// Create the lid with holes cut out for the buttons and LCD.
// NOTE: IN THIS MODEL THERE ARE NO HOLES BUT HAVING CODED THE LOGIC
// FOR LAYING THEM OUT IT IS EASIER TO SET THEIR DIMENSIONS TO ZERO
// AND LEAVE IN THE LOGIC
difference() {
    difference() {
        difference() {
            difference() {
                difference() {
                    translate([lidInStep,lidInStep,boxHeightInner])
                        cube([boxLengthInner+(2*lidInStep),boxWidthInner+(2 * lidInStep),lidThickness]);
                    
                    translate([boxLengthInner/5,20,boxHeightInner - 10])
                        cube([buttonWidth,buttonWidth,40]);
                };
                
                translate([(boxLengthInner/5)*2,20,boxHeightInner - 10])
                    cube([buttonWidth,buttonWidth,40]);
            };
            
            translate([(boxLengthInner/5)*3,20,boxHeightInner - 10])
                cube([buttonWidth,buttonWidth,40]);
        };
        
        translate([boxLengthInner-(boxLengthInner/5),20,boxHeightInner - 10])
            cube([buttonWidth,buttonWidth,40]);
        
    };
    
    translate([(boxLengthInner/2)-(lcdHoleLength/2),70,boxHeightInner - 10])
        cube([lcdHoleLength,lcdHoleWidth,40]);
}