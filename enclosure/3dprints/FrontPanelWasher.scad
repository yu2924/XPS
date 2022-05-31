$fn = 90;
dxy = 6;
sp = 4;
eps = 0.001;
trim = 0.4;

module frontpanelwasher()
{
    do = 5.5;
    di = 3.2 + trim;
    t = 0.4;
    difference()
    {
        cylinder(h=t, d=do, center=false);
        translate([0, 0, -eps]) cylinder(h=t+eps*2, d=di, center=false);
    }
}

for(x = [-1.5, -0.5, 0.5, 1.5])
{
    translate([x * (dxy + sp), 0, 0]) frontpanelwasher();
}
