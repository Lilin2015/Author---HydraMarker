function s_state = satfy_to_superstate(shape,state)             
    
    [safty0,safty1,idx] = measure_safety_assign(state,shape);

    support = inf(size(state.val));
    likelihood = ones(size(state.val));

    support(idx) = 0;
    likelihood(idx) = double(safty1>safty0);

    support(state.val>=0) = -1;
    likelihood(state.val>=0) = -1;
    support(state.val==-2) = -2;
    likelihood(state.val==-2) = -2;

    stuck_pos = (isnan(likelihood) & (support)==0);
    likelihood(stuck_pos) = -3;
    support(stuck_pos) = -3;

    s_state = Dsuperstate(likelihood,support);
end