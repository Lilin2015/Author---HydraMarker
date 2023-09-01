function [safty0,safty1,idx] = measure_safety_assign(state,shape)
    idx = safty_freedom_assign(state,shape);
    [saftyB0,saftyB1] = safty_self_assign(state,shape,idx);
    [saftyC0,saftyC1] = safty_cross_assign(state,shape,idx);

    safty0 = saftyB0*saftyC0;
    safty1 = saftyB1*saftyC1;
end

