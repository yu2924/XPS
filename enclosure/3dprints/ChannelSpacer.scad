$fn = 90;
dx = 40;
dy = 18;
sp = 4;

module spacer()
{
    trim = 0.0;
    twsh = 1.5;
    t1 = 3.3 - twsh - trim;
    t2 = 5.85 - trim;
    translate([0, dy, 0])
        rotate(a=180, v=[1, 0, 0])
        {
            union()
            {
                translate([0, 0, -t1])
                    linear_extrude(height=t1, center=false)
                        import(file="ChannelSpacerPlane.dxf", layer="0");
                translate([0, 0, -t2])
                    linear_extrude(height=t2, center=false)
                        import(file="ChannelSpacerPost.dxf", layer="0");
            }
        }
}

printmulti = false;

if(printmulti)
{
    translate([-dx/2,  sp/2, 0]) spacer();
    translate([-dx/2, -dy - sp/2, 0]) spacer();
}
else
{
    translate([-dx/2,  -dy/2, 0]) spacer();
}
