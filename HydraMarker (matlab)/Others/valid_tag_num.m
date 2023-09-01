function num = valid_tag_num(state,shape)
    num = 0;
    valid_map = zeros(size(state.val));
    for shape_it = 1 : size(shape.val,1)
        full = sum(shape.val{shape_it},'all');
        for shape_rot = shape.rot_prop{shape_it}(:,1)'-1
            shape_cur = logical(rot90(shape.val{shape_it},shape_rot));
            hit = conv2(state.val>=0,rot90(shape_cur,2),'valid')==full;
            valid_map = or(valid_map,imdilate(hit,strel(shape_cur),'full'));
        end
    end
    num = sum(valid_map,'all');
end

