$fn = 90;
dxy = 15;
sp = 4;
eps = 0.001;
trim = 0.3;

module jackwasher()
{
    do = 13.5;
    di = 11.4 + trim;
    t = 0.4;
    difference()
    {
        cylinder(h=t, d=do, center=false);
        translate([0, 0, -eps]) cylinder(h=t+eps*2, d=di, center=false);
    }
}

for(x = [-0.5, 0.5])
{
    translate([x * (dxy + sp), 0, 0]) jackwasher();
}
