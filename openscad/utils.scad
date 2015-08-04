// Draw a 'ramp' polygon, i.e. a pyramid with a flat sqaure base
// Orientation can be 0, 1 (rotate about the centre by 90 deg), 2 (180), or 3 (270)
module drawRamp(baseWidth, baseLength, height, orientation) {
    translate([orientation > 0 && orientation < 3 ? baseWidth : 0, orientation > 1 && orientation < 4 ? baseLength : 0, 0])
    rotate([0,0,orientation * 90])
    polyhedron(
        points=[[0,0,0],
                [baseWidth,0,0],
                [baseWidth,baseLength,0],
                [0,baseLength,0],
                [0,0,height],
                [baseWidth,0,height]],
        faces=[[0,4,3],[0,1,5,4],[1,2,5],[2,3,4,5],[0,3,2,1]]);
}