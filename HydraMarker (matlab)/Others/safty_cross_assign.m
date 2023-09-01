function [safty0,safty1] = safty_cross_assign(state,shape,idx)

    safty0 = ones(size(state.val));
    safty1 = ones(size(state.val));
    
    % find uncompleted tags
    knowns = double(state.val>=0);
    knowns(state.val==-2) = NaN;
    for shape_it = 1 : size(shape.rot_prop,1)
        for shape_rot = shape.rot_prop{shape_it}(:,1)'-1
            shape_cur = logical(rot90(shape.val{shape_it},shape_rot));
            [shape_m,shape_n] = size(shape_cur);
            
            [rtag_m,rtag_n] = find_rtag(state,shape_cur,idx);
            rstack = build_stack(state.val,shape_cur,rtag_m,rtag_n);
            for rot = 0 : 3
                field = rot90(state.val,rot);
                [ktags_m,ktags_n] = find(conv2(rot90(knowns,rot),rot90(shape_cur,2),'valid')>0);
                kstack = build_stack(field,shape_cur,ktags_m,ktags_n);
                
                for it = 1 : size(rstack,3)
                    ustack_it = repmat(rstack(:,:,it),[1,1,size(kstack,3)]);
                    eq_pos = (ustack_it==kstack);
                    eq_pos(ustack_it==-1) = 1;
                    match_idx = find(all(eq_pos,[1,2]));
                    for ik = 1 : length(match_idx)
                        match_tag_A = kstack(:,:,match_idx(ik));
                        match_tag_B = rstack(:,:,it);
                        risk = 0.5.^(sum(match_tag_A==-1|match_tag_B==-1,'all')-1);
                        risk0 = risk.*(match_tag_A==0);
                        risk1 = risk.*(match_tag_A==1);
                        risk0(match_tag_A==-1) = risk0(match_tag_A==-1)*0.5;
                        risk1(match_tag_A==-1) = risk1(match_tag_A==-1)*0.5;
                        risk0(match_tag_B~=-1) = 0;
                        risk1(match_tag_B~=-1) = 0;
                        safty0(rtag_m(it):rtag_m(it)+shape_m-1,rtag_n(it):rtag_n(it)+shape_n-1) = ...
                            safty0(rtag_m(it):rtag_m(it)+shape_m-1,rtag_n(it):rtag_n(it)+shape_n-1).* ...
                                (1-risk0);
                        safty1(rtag_m(it):rtag_m(it)+shape_m-1,rtag_n(it):rtag_n(it)+shape_n-1) = ...
                            safty1(rtag_m(it):rtag_m(it)+shape_m-1,rtag_n(it):rtag_n(it)+shape_n-1).* ...
                                (1-risk1);
                    end % for match tag
                end % for utags
            end % for field rot
        end % for shape rot
    end % for shape
    safty0 = safty0(idx);
    safty1 = safty1(idx);
end

function stack = build_stack(field,shape,m,n)
    [shape_m,shape_n] = size(shape);
    stack = zeros(shape_m,shape_n,length(m));
    for it = 1 : length(m)
        tag_temp = field(m(it):m(it)+shape_m-1,n(it):n(it)+shape_n-1);
        tag_temp(~shape)=3;
        stack(:,:,it) = tag_temp; 
    end
end
