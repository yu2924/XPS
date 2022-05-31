$fs = 0.1;
$fa = 1;

// ----------------------------------------
// cap

dx = 10;
dy = 7;
dz = 6;
wstem = 3.3;
hstem = 4;
sp = 4;
eps = 0.001;
trim = 0.0;

module cap()
{
    r = 1.5;
    w1 = 3.3+trim;
    d1 = 5;
    w2 = 4+trim;
    d2 = 1;
    difference()
    {
        translate([-dx/2+r, -dy/2+r, 0])
        {
            minkowski()
            {
                cube([dx-r*2, dy-r*2, dz-eps]);
                cylinder(r=r, h=eps);
            }
        }
        union()
        {
            // stem1
            translate([-w1/2, -w1/2, 0])
                cube([w1, w1, d1]);
            // stem2
            translate([-w2/2, -w2/2, 0])
                cube([w2, w2, d2]);
            // hole cutter
            translate([-w2/2, -w2/2, -eps])
                cube([w2, w2, eps*2]);
        }
    }
}

// ----------------------------------------
// output

printmulti = false;
printsingle = true;
xsect = false;

if(printmulti)
{
    for(x = [-0.5, 0.5])
    {
        for(y = [-0.5, 0.5])
        {
            translate([x * (dx + sp), y * (dy + sp)])
                rotate(a=180, v=[1, 0, 0])
                    translate([0, 0, -dz])
                        cap();
        }
    }
}
else if(printsingle)
{
    rotate(a=180, v=[1, 0, 0])
        translate([0, 0, -dz])
            cap();
}
else if(xsect)
{
    difference()
    {
        cap();
        translate([0, -10, 0])
            cube([20, 20, 20], center=true);
    }
}
else
{
    cap();
}
