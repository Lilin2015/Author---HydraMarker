function idx = safty_freedom_assign(state,shape)

    unknowns = double(state.val==-1);
    unknowns(state.val==-2) = Inf;
    safty = Inf(size(state.val));

    for shape_it = 1 : size(shape.val,1)
        for shape_rot = shape.rot_prop{shape_it}(:,1)'-1
            shape_cur = logical(rot90(shape.val{shape_it},shape_rot));
            
            chg_tag_map = conv2(unknowns,rot90(shape_cur,2),'valid');
            
            freedom_cur = imerode(chg_tag_map,rot90(shape_cur,2),'full');

            safty = min(safty,freedom_cur);
        end % for shape rot
    end % for shape
    safty(state.val>=0) = inf;
    [~,idx] = min(safty,[],'all','linear');
end


