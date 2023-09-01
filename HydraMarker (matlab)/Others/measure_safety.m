function [safty0,safty1] = measure_safety(state,shape)
    
    saftyA = safty_freedom(state,shape);

    [saftyB0,saftyB1] = safty_self(state,shape);
    [saftyC0,saftyC1] = safty_cross(state,shape);
    safty0 = saftyA.*saftyB0.*saftyC0;
    safty1 = saftyA.*saftyB1.*saftyC1;
end