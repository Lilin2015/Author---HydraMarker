function [rtag_m,rtag_n] = find_rtag(state,shape_cur,idx)
    focus_ele = zeros(size(state.val));
    focus_ele(idx) = 1;
    touch_risk = conv2(double(focus_ele),rot90(shape_cur,2),'valid');
    touch_known = conv2(double(state.val>=0),rot90(shape_cur,2),'valid');
    [rtag_m,rtag_n] = find(touch_risk>0&touch_known);
end

