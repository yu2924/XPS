$fn = 90;
dx = 62;
dy = 15;

module spacer()
{
    twsh = 1.5;
    t = 3.3 - twsh;
    translate([-dx/2, -dy/2, 0])
        linear_extrude(height=t, center=false)
            import(file="OutputConnectorSpacerPlane.dxf", layer="0");
}

spacer();
