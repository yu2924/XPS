$fs = 0.1;
$fa = 1;

// ----------------------------------------
// cap

module capsolid(w, h, d, rh, rv)
{
    difference()
    {
        minkowski()
        {
            minkowski()
            {
                cube([w-rh*2-rv*2, h-rh*2-rv*2, d*2-rh*2-rv*2], center=true);
                cylinder(r=rh, h=rh);
            }
            sphere(rv);
        }
        translate([0, 0, -d])
            cube([w*2, h*2, d*2], center=true);
    }
}

module capshell(w, h, d)
{
    rho = 1.5;
    rhi = 0.5;
    rv = 0.2;
    difference()
    {
        capsolid(w, h, d, rho, rv);
        translate([0, 0, -1]) capsolid(w-2, h-2, d, rhi, rv);
    }
}

dx = 10;
dy = 7;
dz = 6;
wstem = 3.3;
hstem = 4;
sp = 4;

module cap()
{
    difference()
    {
        capshell(dx, dy, dz);
        translate([0, 0, dz-1])
            cube([5, wstem, 1], center=true);
    }
    translate([0,  wstem/2+0.5, hstem/2+1])
        cube([wstem-0.5, 1, hstem], center=true);
    translate([0, -wstem/2-0.5, hstem/2+1])
        cube([wstem-0.5, 1, hstem], center=true);
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
