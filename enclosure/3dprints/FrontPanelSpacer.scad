$fn = 90;
dx = 215;
dy = 44;
dz = 1.6;
trim = 0.0;

module spacer()
{
    t = dz - trim;
    translate([-dx/2, -dy/2, 0])
        linear_extrude(height=t, center=false)
            import(file="FrontPanelSpacerPlane.dxf", layer="1");
}

spacer();
