$fn = 90;
dx = 33;
dy = 34;
sp = 4;

module spacer()
{
    t1 = 2;
    t2 = 9.4 - t1;
    translate([0, dy, 0])
        rotate(a=180, v=[1, 0, 0])
        {
            translate([0, 0, -t1])
                linear_extrude(height=t1, center=false)
                    import(file="ControlPanelSpacerPlane.dxf", layer="0");
            translate([0, 0, -t1-t2])
                linear_extrude(height=t2, center=false)
                    import(file="ControlPanelSpacerPost.dxf", layer="0");
        }
}

module jig()
{
    t1 = 2;
    t2 = 1;
    t3 = 9.4 + 1.2 + 1.5;
    translate([0, dy, 0])
        rotate(a=180, v=[1, 0, 0])
        {
            translate([0, 0, -t1])
                linear_extrude(height=t1, center=false)
                    import(file="ControlPanelJigPlane.dxf", layer="0");
            translate([0, 0, -t1-t2])
                linear_extrude(height=t2, center=false)
                    import(file="ControlPanelJigPlane.dxf", layer="1");
            translate([0, 0, -t1-t2-t3])
                linear_extrude(height=t3, center=false)
                    import(file="ControlPanelSpacerPost.dxf", layer="0");
        }
}

printjig = true;
printmulti = false;

if(printjig)
{
    translate([-dx/2,  -dy/2, 0]) jig();
}
else if(printmulti)
{
    translate([ sp/2, -dy/2, 0]) spacer();
    translate([-dx - sp/2, -dy/2, 0]) spacer();
}
else
{
    translate([-dx/2,  -dy/2, 0]) spacer();
}
