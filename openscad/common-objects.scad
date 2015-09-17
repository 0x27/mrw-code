use <utils.scad>

// Create a screw fixing, with a hole in the bottom for the screw, 
// and a triangular 'buttress' at the top to help with printing 
module screwFixing(screwFixingWidth, screwFixingHeight, screwHoleDiameter, orientation) {
    difference() {
        cube(size=[screwFixingWidth,screwFixingWidth,screwFixingHeight]);
        translate([screwFixingWidth/2,screwFixingWidth/2,0])
            cylinder(r=screwHoleDiameter,h=screwFixingHeight/6*5);
    }
    translate([0,0,screwFixingHeight])
        drawRamp(screwFixingWidth,screwFixingWidth,screwFixingHeight/2,orientation);
}

//createScrewFixing(10, 10, 2);