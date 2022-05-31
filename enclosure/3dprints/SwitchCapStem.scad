$fs = 0.1;
$fa = 1;
$fn = 30;
dx = 5;
dy = 4.5;
ds = 3.3;
sp = 4;

module stem()
{
    translate([0, -dy/2, 0])
        linear_extrude(height=ds, center=false)
            import(file="SwitchCapStemXsect.dxf", layer="0");
}

printmulti = false;

if(printmulti)
{
    for(x = [-0.5, 0.5])
    {
        for(y = [-0.5, 0.5])
        {
            translate([x * (dx + sp), y * (dy + sp)]) stem();
        }
    }
}
else
{
    stem();
}
