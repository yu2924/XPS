$fn = 90;
dxy = 6;
sp = 4;
eps = 0.001;
trim = 0.3;

module spacer()
{
    do = 6;
    di = 3.2 + trim;
    t = 1;
    difference()
    {
        cylinder(h=t, d=do, center=false);
        translate([0, 0, -eps]) cylinder(h=t+eps*2, d=di, center=false);
    }
}

for(x = [-0.5, 0.5])
{
    translate([x * (dxy + sp), 0, 0]) spacer();
}
